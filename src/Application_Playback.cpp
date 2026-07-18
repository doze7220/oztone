#include "Application.h"
#include <filesystem>
#include <map>
#include <cmath>

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
    // 1. フリップ用バッファの構築
    std::map<int, DrumSlot> drumBuffer;
    
    int startIdx = -std::abs(relativeDistance) - 2;
    int endIdx = std::abs(relativeDistance) + 2;
    
    size_t totalTracks = m_playlistManager.GetCount();
    size_t currentIndex = m_playlistManager.GetCurrentIndex();
    const auto& shuffleIndices = m_playlistManager.GetShuffleIndices();
    const auto& shuffleList = m_playlistManager.GetShuffleList();
    
    for (int i = startIdx; i <= endIdx; ++i) {
      int absIndex = static_cast<int>(currentIndex) + i;
      int normalizedIndex = 0;
      if (totalTracks > 0) {
        normalizedIndex = (absIndex % static_cast<int>(totalTracks) + static_cast<int>(totalTracks)) % static_cast<int>(totalTracks);
      }
      
      DrumSlot slot;
      if (totalTracks > 0 && normalizedIndex >= 0 && normalizedIndex < static_cast<int>(shuffleList.size())) {
        std::wstring path = shuffleList[normalizedIndex];
        
        TrackMetadata meta;
        if (m_trackDatabase.GetMetadata(path, meta) && meta.isMetaLoaded) {
          slot.trackTitle = meta.title;
          slot.trackArtist = meta.artist;
        } else {
          TagManager tempTag;
          if (tempTag.Load(path)) {
            slot.trackTitle = tempTag.GetTitle();
            slot.trackArtist = tempTag.GetArtist();
            if (slot.trackTitle.empty()) {
              try { slot.trackTitle = std::filesystem::path(path).filename().wstring(); } catch (...) { slot.trackTitle = L"UNKNOWN"; }
            }
            if (slot.trackArtist.empty()) slot.trackArtist = L"---";
          }
        }
        if (normalizedIndex < static_cast<int>(shuffleIndices.size())) {
          wchar_t buffer[64];
          swprintf_s(buffer, L"%03zu/%03zu", shuffleIndices[normalizedIndex] + 1, totalTracks);
          slot.trackNumber = buffer;
        }
      }
      drumBuffer[i] = slot;
    }

    // 先にフリップを確定させ、インデックスを切り替える
    m_renderer.SetDrumTarget(relativeDistance, drumBuffer);

    // 2. 切り替わった新スロットに対して画像をセットする
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

    // 3. 新スロットに対してフレーミング情報をセットする
    float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
    m_framingDb.GetFraming(track, artX, artY, artScale);
    m_renderer.SetBackgroundFraming(artX, artY, artScale);

    UpdateTrackMetadataIfNeeded(track);
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
