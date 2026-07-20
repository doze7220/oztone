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

    int totalDistance = 0;
    while (skipCount < totalCount) {
      int distance = (cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) ? 1 : -1;
      totalDistance += distance;
      std::wstring track = m_playlistManager.GetCurrentTrack();
      if (m_audioPlayer.Play(track)) {
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
  if (m_audioPlayer.Play(track)) {
    auto dataProvider = [this](int relativeIndex, DrumSlot* slot) -> TrackMetadata {
      size_t currentIdx = m_playlistManager.GetCurrentIndex();
      size_t total = m_playlistManager.GetCount();
      size_t targetIdx = (currentIdx + relativeIndex + total) % total;
      
      std::wstring path = m_playlistManager.GetShuffleList()[targetIdx];
      
      if (slot) {
          bool isNew = false;
          slot->thumbId = m_thumbnailDatabase.GetOrGenerateThumbId(path, isNew);
      }

      TrackMetadata meta;
      if (m_trackDatabase.GetMetadata(path, meta) && meta.isMetaLoaded) {
        return meta;
      } else {
        TagManager tempTag;
        if (tempTag.Load(path)) {
          meta.title = tempTag.GetTitle();
          meta.artist = tempTag.GetArtist();
          if (meta.title.empty()) {
            try { meta.title = std::filesystem::path(path).filename().wstring(); } catch (...) { meta.title = L"UNKNOWN"; }
          }
          if (meta.artist.empty()) meta.artist = L"---";
        } else {
          try { meta.title = std::filesystem::path(path).filename().wstring(); } catch (...) { meta.title = L"UNKNOWN"; }
          meta.artist = L"---";
        }
        return meta;
      }
    };

    m_tagManager.Load(track);
    // [Phase23-1] 背景画像読み込み・デコード処理および
    // 背景フレーミングの設定伝達処理をパージ

    auto onComplete = [this, track]() {
      bool isNew = false;
      uint32_t thumbId = m_thumbnailDatabase.GetOrGenerateThumbId(track, isNew);
      ID2D1Bitmap* thumbBmp = m_thumbnailDatabase.GetCachedThumbnailBitmap(thumbId);
      
      m_renderer.GetTrackDrum().SetAlbumArt(thumbBmp);

      if (!thumbBmp) {
        m_thumbnailDatabase.RequestThumbnailLoad(thumbId, m_renderer.GetD2DContext(), m_renderer.GetWicFactory());
      }

      UpdateTrackMetadataIfNeeded(track);
    };

    m_renderer.GetTrackDrum().StartDrumAnimation(relativeDistance, m_config.GetTrackDrumMaxDuration(), m_config.GetTrackDrumMaxSpeed(), dataProvider, onComplete);
    return true;
  }
  return false;
}

void Application::UpdateTrackMetadataIfNeeded(const std::wstring &filepath) {
  TagManager localTagManager;
  if (localTagManager.Load(filepath, true)) { // 画像抽出をスキップしてテキスト情報のみ取得
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
        
        m_renderer.GetTrackDrum().UpdateCurrentDrumSlot(currentMeta);
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
        
        m_renderer.GetTrackDrum().UpdateCurrentDrumSlot(currentMeta);
    }
  }
}
