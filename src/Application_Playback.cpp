#include "Application.h"
#include <filesystem>
#include <map>
#include <cmath>

void Application::HandleMediaCommand(int cmd) {
  if (cmd == APPCOMMAND_MEDIA_PLAY_PAUSE) {
    m_audioManager.TogglePlayPause();
    m_renderer.TriggerFlyText(L"PLAY/PUASE");
  } else if (cmd == APPCOMMAND_MEDIA_STOP) {
    m_audioManager.Stop();
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

    m_audioManager.Stop();

    int totalDistance = 0;
    while (skipCount < totalCount) {
      int distance = (cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) ? 1 : -1;
      totalDistance += distance;
      std::wstring track = m_playlistManager.GetCurrentTrack();
      if (m_audioManager.Play(track)) {
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
      m_renderer.GetTrackDrum().StartDrumAnimation(0, 0.0f, 0.0f, nullptr, nullptr);
      m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
    } else {
      PlayCurrentTrack(totalDistance);
    }
  }
}


bool Application::PlayCurrentTrack(int relativeDistance) {
  std::wstring track = m_playlistManager.GetCurrentTrack();
  
  // TrackMetadataの自己修復はメインスレッド同期処理のため、ファイルロックを取得される前に実行する
  UpdateTrackMetadataIfNeeded(track);

  if (m_audioManager.Play(track)) {

    // 司令塔としてのデータ分配と非同期処理の発注
    bool isNewThumb = false;
    uint32_t currentThumbId = m_thumbnailManager.GetOrGenerateThumbId(track, isNewThumb);

    // 背景フレーミングの設定伝達処理を復活
    float framingX = 0.0f;
    float framingY = 0.0f;
    float framingScale = 1.0f;
    m_framingDb.GetFraming(track, framingX, framingY, framingScale);
    m_backgroundManager.SetBackgroundFraming(framingScale, framingX, framingY);

    std::vector<BYTE> artData = FileManager::ExtractAlbumArtBinary(track);
    if (artData.empty()) {
        m_isCurrentBackgroundPlaceholder = true;
        m_backgroundManager.RequestLoad(L"");
    } else {
        m_isCurrentBackgroundPlaceholder = false;
        m_backgroundManager.RequestLoad(track);
    }

    auto dataProvider = [this](int relativeIndex, DrumSlot* slot) -> TrackMetadata {
      size_t currentIdx = m_playlistManager.GetCurrentIndex();
      size_t total = m_playlistManager.GetCount();
      size_t targetIdx = (currentIdx + relativeIndex + total) % total;
      
      std::wstring path = m_playlistManager.GetShuffleList()[targetIdx];
      
      if (slot) {
          bool isNew = false;
          slot->thumbId = m_thumbnailManager.GetOrGenerateThumbId(path, isNew);
      }

      TrackMetadata meta;
      if (m_trackDatabase.GetMetadata(path, meta) && meta.isMetaLoaded) {
        return meta;
      } else {
          AudioMetadata fmMeta = FileManager::ExtractTextMetadata(path);
          if (!fmMeta.title.empty() || !fmMeta.artist.empty()) {
            meta.title = fmMeta.title;
            meta.artist = fmMeta.artist;
            if (meta.title.empty()) {
              try { meta.title = std::filesystem::path(path).filename().wstring(); } catch (...) { meta.title = L"UNKNOWN"; }
            }
            if (meta.artist.empty()) meta.artist = L"　";
          } else {
            try { meta.title = std::filesystem::path(path).filename().wstring(); } catch (...) { meta.title = L"UNKNOWN"; }
            meta.artist = L"　";
          }
        return meta;
      }
    };

    auto onComplete = [this, track, currentThumbId]() {
      ID2D1Bitmap* thumbBmp = m_thumbnailManager.GetCachedThumbnailBitmap(currentThumbId);
      
      m_renderer.GetTrackDrum().SetAlbumArt(thumbBmp);

      if (!thumbBmp) {
        m_thumbnailManager.RequestThumbnailLoad(currentThumbId, m_renderer.GetD2DContext(), m_renderer.GetWicFactory());
      }
    };

    m_renderer.GetTrackDrum().StartDrumAnimation(relativeDistance, m_config.GetTrackDrumMaxDuration(), m_config.GetTrackDrumMaxSpeed(), dataProvider, onComplete);
    return true;
  }
  return false;
}

void Application::UpdateTrackMetadataIfNeeded(const std::wstring &filepath) {
  AudioMetadata fmMeta = FileManager::ExtractTextMetadata(filepath);
  if (!fmMeta.title.empty() || !fmMeta.artist.empty()) {
    std::wstring title = fmMeta.title;
    std::wstring artist = fmMeta.artist;
    if (title.empty()) {
      try {
        title = std::filesystem::path(filepath).filename().wstring();
      } catch (...) {
        title = L"UNKNOWN";
      }
    }
    if (artist.empty()) {
      artist = L"　";
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
        wchar_t buf[32];
        swprintf_s(buf, 32, L"%02d:%02d", fmMeta.durationSeconds / 60, fmMeta.durationSeconds % 60);
        currentMeta.timeString = buf;
        currentMeta.isMetaLoaded = true;
        m_trackDatabase.UpdateMetadata(filepath, currentMeta);
        std::wstring defaultPath = m_config.GetDefaultPlaylistPath();
        m_playlistManager.SaveToFile(defaultPath);
        UpdatePlaylistSummaries();
        
        m_renderer.GetTrackDrum().UpdateCurrentDrumSlot(currentMeta);
      }
    } else {
        currentMeta.filepath = filepath;
        currentMeta.title = title;
        currentMeta.artist = artist;
        wchar_t buf[32];
        swprintf_s(buf, 32, L"%02d:%02d", fmMeta.durationSeconds / 60, fmMeta.durationSeconds % 60);
        currentMeta.timeString = buf;
        currentMeta.isMetaLoaded = true;
        m_trackDatabase.UpdateMetadata(filepath, currentMeta);
        std::wstring defaultPath = m_config.GetDefaultPlaylistPath();
        m_playlistManager.SaveToFile(defaultPath);
        UpdatePlaylistSummaries();
        
        m_renderer.GetTrackDrum().UpdateCurrentDrumSlot(currentMeta);
    }
  }
}
