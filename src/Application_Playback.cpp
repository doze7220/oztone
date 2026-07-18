#include "Application.h"
#include <filesystem>

void Application::HandleMediaCommand(int cmd) {
  if (cmd == APPCOMMAND_MEDIA_PLAY_PAUSE) {
    m_audioPlayer.TogglePlayPause();
    m_renderer.TriggerFlyText(L"PLAY/PUASE");
  } else if (cmd == APPCOMMAND_MEDIA_STOP) {
    m_audioPlayer.Stop();
    m_renderer.TriggerFlyText(L"STOP");
  } else if (cmd == APPCOMMAND_MEDIA_NEXTTRACK ||
             cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) {
    if (!m_playlistManager.IsEmpty()) {
      m_playlistManager.SaveToFile(m_config.GetDefaultPlaylistPath());
      if (!m_framingDbPath.empty()) {
        m_framingDb.SaveToFile(m_framingDbPath);
      }
    }
    if (cmd == APPCOMMAND_MEDIA_NEXTTRACK) {
      m_renderer.TriggerFlyText(L"NEXT TRACK");
      size_t prevIndex = m_playlistManager.GetCurrentIndex();
      m_playlistManager.Advance();
      if (m_playlistManager.GetCurrentIndex() <= prevIndex) {
          m_isContinuousStream = false;
          m_streamBreakDirection = StreamBreakDirection::Next;
      }
    } else {
      m_renderer.TriggerFlyText(L"PREV TRACK");
      size_t prevIndex = m_playlistManager.GetCurrentIndex();
      m_playlistManager.Previous();
      if (m_playlistManager.GetCurrentIndex() >= prevIndex) {
          m_isContinuousStream = false;
          m_streamBreakDirection = StreamBreakDirection::Prev;
      }
    }

    size_t skipCount = 0;
    bool played = false;
    size_t totalCount = m_playlistManager.GetCount();

    m_audioPlayer.Stop();

    while (skipCount < totalCount) {
      std::wstring track = m_playlistManager.GetCurrentTrack();

      if (!m_isContinuousStream) {
          m_renderer.ResetDrumPosition(m_playlistManager.GetCurrentIndex(), m_streamBreakDirection == StreamBreakDirection::Next);
          m_isContinuousStream = true;
      }

      if (cmd == APPCOMMAND_MEDIA_NEXTTRACK && skipCount == 0 &&
          m_isPrefetchReady.load()) {
        m_renderer.SetTrackInfo(m_prefetchedTitle, m_prefetchedArtist, m_playlistManager.GetCurrentIndex());
        m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());
      } else {
        if (m_tagManager.Load(track)) {
          std::wstring title = m_tagManager.GetTitle();
          std::wstring artist = m_tagManager.GetArtist();
          if (title.empty())
            title = std::filesystem::path(track).filename().wstring();
          if (artist.empty())
            artist = L"---";
          m_renderer.SetTrackInfo(title, artist, m_playlistManager.GetCurrentIndex());

          const auto &artBytes = m_tagManager.GetAlbumArtBytes();
          if (!artBytes.empty()) {
            Microsoft::WRL::ComPtr<ID2D1Bitmap> artBitmap;
            if (m_renderer.LoadBitmapFromMemory(artBytes, &artBitmap)) {
              m_renderer.SetAlbumArt(artBitmap.Get());
            } else {
              m_renderer.SetAlbumArt(nullptr);
            }
          } else {
            m_renderer.SetAlbumArt(nullptr);
          }
        } else {
          std::wstring title;
          try {
            title = std::filesystem::path(track).filename().wstring();
          } catch (...) {
            title = L"Unknown";
          }
          m_renderer.SetTrackInfo(title, L"---", m_playlistManager.GetCurrentIndex());
          m_renderer.SetAlbumArt(nullptr);
        }
      }

      float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
      m_framingDb.GetFraming(track, artX, artY, artScale);
      m_renderer.SetBackgroundFraming(artX, artY, artScale);
      if (PlayCurrentTrack()) {
        played = true;
        break;
      }

      if (cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) {
        m_playlistManager.Previous();
      } else {
        m_playlistManager.Advance();
      }
      skipCount++;
    }

    if (!played) {
      m_renderer.SetTrackInfo(L"NO TRACK", L"---", 0, true);
      m_renderer.SetAlbumArt(nullptr);
    }
  }
}

void Application::PrefetchNextTrack() {
  m_isPrefetchReady.store(false);

  if (m_prefetchThread.joinable()) {
    m_prefetchThread.join();
  }

  m_prefetchThread = std::thread([this]() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    m_prefetchedTitle.clear();
    m_prefetchedArtist.clear();
    m_prefetchedAlbumArt.Reset();

    std::wstring nextFile = m_playlistManager.GetNextTrack();
    if (!nextFile.empty()) {
      bool loadSuccess = false;
      try {
        if (std::filesystem::exists(std::filesystem::path(nextFile))) {
          loadSuccess = m_tagManager.Load(nextFile);
        }
      } catch (...) {
        loadSuccess = false;
      }

      if (loadSuccess) {
        m_prefetchedTitle = m_tagManager.GetTitle();
        m_prefetchedArtist = m_tagManager.GetArtist();

        if (m_prefetchedTitle.empty()) {
          try {
            m_prefetchedTitle =
                std::filesystem::path(nextFile).filename().wstring();
          } catch (...) {
            m_prefetchedTitle = L"UNKNOWN";
          }
        }
        if (m_prefetchedArtist.empty()) {
          m_prefetchedArtist = L"---";
        }

        const auto &artBytes = m_tagManager.GetAlbumArtBytes();
        if (!artBytes.empty()) {
          m_renderer.LoadBitmapFromMemory(artBytes, &m_prefetchedAlbumArt);
        }

        UpdateTrackMetadataIfNeeded(nextFile);
      } else {
        try {
          m_prefetchedTitle =
              std::filesystem::path(nextFile).filename().wstring();
        } catch (...) {
          m_prefetchedTitle = L"UNKNOWN";
        }
        m_prefetchedArtist = L"---";
      }
    }

    m_isPrefetchReady.store(true);

    if (SUCCEEDED(hr)) {
      CoUninitialize();
    }
  });
}

void Application::LoadCurrentTrackArtAsync() {
  m_isCurrentArtLoadReady.store(false);

  if (m_currentArtThread.joinable()) {
    m_currentArtThread.join();
  }

  m_currentArtThread = std::thread([this]() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    m_loadedCurrentArt.Reset();

    std::wstring currentFile = m_playlistManager.GetCurrentTrack();
    if (!currentFile.empty()) {
      bool loadSuccess = false;
      try {
        if (std::filesystem::exists(std::filesystem::path(currentFile))) {
          TagManager localTag;
          if (localTag.Load(currentFile)) {
            const auto &artBytes = localTag.GetAlbumArtBytes();
            if (!artBytes.empty()) {
              m_renderer.LoadBitmapFromMemory(artBytes, &m_loadedCurrentArt);
            }
          }
        }
      } catch (...) {}
    }

    m_isCurrentArtLoadReady.store(true);
    if (SUCCEEDED(hr)) {
      CoUninitialize();
    }
  });
}

bool Application::PlayCurrentTrack() {
  std::wstring track = m_playlistManager.GetCurrentTrack();
  if (m_audioPlayer.Play(track)) {
    UpdateTrackMetadataIfNeeded(track);
    PrefetchNextTrack();
    return true;
  }
  return false;
}

void Application::UpdateTrackMetadataIfNeeded(const std::wstring &filepath) {
  TagManager localTagManager;
  if (localTagManager.Load(filepath)) {
    std::wstring title = localTagManager.GetTitle();
    std::wstring artist = localTagManager.GetArtist();
    if (title.empty()) {
      try {
        title = std::filesystem::path(filepath).filename().wstring();
      } catch (...) {
        title = L"UNKNOWN";
      }
    }
    if (artist.empty()) {
      artist = L"---";
    }

    TrackMetadata currentMeta;
    if (m_trackDatabase.GetMetadata(filepath, currentMeta)) {
      bool needsUpdate = false;
      if (!currentMeta.isMetaLoaded) {
        needsUpdate = true;
      } else if (currentMeta.title != title || currentMeta.artist != artist) {
        needsUpdate = true;
      }

      if (needsUpdate) {
        currentMeta.title = title;
        currentMeta.artist = artist;
        currentMeta.timeString = localTagManager.GetTimeString();
        currentMeta.isMetaLoaded = true;
        m_trackDatabase.UpdateMetadata(filepath, currentMeta);
        std::wstring defaultPath = m_config.GetDefaultPlaylistPath();
        m_playlistManager.SaveToFile(defaultPath);
        UpdatePlaylistSummaries();
      }
    } else {
        currentMeta.filepath = filepath;
        currentMeta.title = title;
        currentMeta.artist = artist;
        currentMeta.timeString = localTagManager.GetTimeString();
        currentMeta.isMetaLoaded = true;
        m_trackDatabase.UpdateMetadata(filepath, currentMeta);
        std::wstring defaultPath = m_config.GetDefaultPlaylistPath();
        m_playlistManager.SaveToFile(defaultPath);
        UpdatePlaylistSummaries();
    }
  }
}
