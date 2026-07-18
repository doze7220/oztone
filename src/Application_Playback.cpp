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
      m_playlistManager.Advance();
    } else {
      m_renderer.TriggerFlyText(L"PREV TRACK");
      m_playlistManager.Previous();
    }

    size_t skipCount = 0;
    bool played = false;
    size_t totalCount = m_playlistManager.GetCount();

    m_audioPlayer.Stop();

    while (skipCount < totalCount) {
      int distance = (cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) ? 1 : -1;
      if (PlayCurrentTrack(distance)) {
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
      m_renderer.SetDrumTarget(0);
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

bool Application::PlayCurrentTrack(int relativeDistance) {
  std::wstring track = m_playlistManager.GetCurrentTrack();
  if (m_audioPlayer.Play(track)) {
    // 1. 先にフリップを確定させ、インデックスを切り替える
    m_renderer.SetDrumTarget(relativeDistance);

    // 2. 切り替わった新スロットに対して画像をセットする
    if (relativeDistance == 1 && m_isPrefetchReady.load()) {
      m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());
    } else {
      if (m_tagManager.Load(track)) {
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
        m_renderer.SetAlbumArt(nullptr);
      }
    }

    // 3. 新スロットに対してフレーミング情報をセットする
    float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
    m_framingDb.GetFraming(track, artX, artY, artScale);
    m_renderer.SetBackgroundFraming(artX, artY, artScale);

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
