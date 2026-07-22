#include "Application.h"
#include <filesystem>
#include <string>
#include <vector>

void Application::SetupCallbacks() {
  m_window.SetOnFilesDroppedCallback(
      [this](const std::vector<std::wstring> &files) {
        this->OnFilesDropped(files);
      });

  m_window.SetBackgroundClickCallback([this]() {
    m_renderer.ForceClearHoverDelays();
  });

  m_window.SetOnResizeCallback([this](int width, int height) {
    m_renderer.Resize(width, height);
    this->ForceRender();
  });

  m_window.SetCopyDataCallback([this](const std::wstring &path) {
    std::vector<std::wstring> files = {path};
    this->OnFilesDropped(files);
  });

  m_window.SetVolumeSetCallback([this](float vol) {
    m_audioManager.SetVolume(vol);
    m_config.SetDefaultVolume(vol);
    m_renderer.TriggerVolumeOsd();
  });
  m_window.SetPlaylistSwitchCallback(
      [this](const std::wstring &filepath) { this->SwitchPlaylist(filepath); });

  m_window.SetShuffleCallback([this]() {
    bool newMode = !m_config.GetShuffleMode();
    m_config.SetShuffleMode(newMode);

    std::wstring currentTrack;
    if (!m_playlistManager.IsEmpty()) {
      currentTrack = m_playlistManager.GetCurrentTrack();
    }

    m_playlistManager.RebuildQueue(newMode);

    m_renderer.TriggerFlyText(newMode ? L"SHUFFLE: ON" : L"SHUFFLE: OFF");

    if (!currentTrack.empty()) {
      m_playlistManager.WarpToTrack(currentTrack);
    }
  });

  m_window.SetResetAllCallback([this]() { this->ResetAllSettings(); });

  m_window.SetMediaCommandCallback(
      [this](int cmd) { this->HandleMediaCommand(cmd); });

  m_window.SetHotkeyCallback([this](int hotkeyId) {
    switch (hotkeyId) {
    case Window::HK_NEXT_TRACK:
      this->HandleMediaCommand(APPCOMMAND_MEDIA_NEXTTRACK);
      break;
    case Window::HK_PREV_TRACK:
      this->HandleMediaCommand(APPCOMMAND_MEDIA_PREVIOUSTRACK);
      break;
    case Window::HK_PLAY_PAUSE:
      this->HandleMediaCommand(APPCOMMAND_MEDIA_PLAY_PAUSE);
      break;
    case Window::HK_STOP:
      this->HandleMediaCommand(APPCOMMAND_MEDIA_STOP);
      break;
    case Window::HK_VOL_UP_5: {
      float vol = m_audioManager.GetVolume() + 0.05f;
      if (vol > 1.0f)
        vol = 1.0f;
      m_audioManager.SetVolume(vol);
      m_config.SetDefaultVolume(vol);
      m_renderer.TriggerVolumeOsd();
      m_renderer.TriggerFlyText(
          L"VOL " + std::to_wstring(static_cast<int>(vol * 100.0f + 0.5f)) +
          L"%");
      break;
    }
    case Window::HK_VOL_DOWN_5: {
      float vol = m_audioManager.GetVolume() - 0.05f;
      if (vol < 0.0f)
        vol = 0.0f;
      m_audioManager.SetVolume(vol);
      m_config.SetDefaultVolume(vol);
      m_renderer.TriggerVolumeOsd();
      m_renderer.TriggerFlyText(
          L"VOL " + std::to_wstring(static_cast<int>(vol * 100.0f + 0.5f)) +
          L"%");
      break;
    }
    case Window::HK_VOL_UP_25: {
      float vol = m_audioManager.GetVolume() + 0.25f;
      if (vol > 1.0f)
        vol = 1.0f;
      m_audioManager.SetVolume(vol);
      m_config.SetDefaultVolume(vol);
      m_renderer.TriggerVolumeOsd();
      m_renderer.TriggerFlyText(
          L"VOL " + std::to_wstring(static_cast<int>(vol * 100.0f + 0.5f)) +
          L"%");
      break;
    }
    case Window::HK_VOL_DOWN_25: {
      float vol = m_audioManager.GetVolume() - 0.25f;
      if (vol < 0.0f)
        vol = 0.0f;
      m_audioManager.SetVolume(vol);
      m_config.SetDefaultVolume(vol);
      m_renderer.TriggerVolumeOsd();
      m_renderer.TriggerFlyText(
          L"VOL " + std::to_wstring(static_cast<int>(vol * 100.0f + 0.5f)) +
          L"%");
      break;
    }
    case Window::HK_PREV_PLAYLIST:
    case Window::HK_NEXT_PLAYLIST: {
      std::vector<std::wstring> playlists = m_config.GetAvailablePlaylists();
      if (playlists.size() <= 1)
        break;
      std::wstring current = m_config.GetDefaultPlaylistPath();
      int idx = -1;
      for (int i = 0; i < (int)playlists.size(); ++i) {
        if (playlists[i] == current) {
          idx = i;
          break;
        }
      }
      if (idx != -1) {
        if (hotkeyId == Window::HK_NEXT_PLAYLIST) {
          idx = static_cast<int>((idx + 1) % playlists.size());
        } else {
          idx =
              static_cast<int>((idx - 1 + playlists.size()) % playlists.size());
        }
        this->SwitchPlaylist(playlists[idx]);
        if (hotkeyId == Window::HK_NEXT_PLAYLIST) {
          m_renderer.TriggerFlyText(L"NEXT PLAYLIST");
        } else {
          m_renderer.TriggerFlyText(L"PREV PLAYLIST");
        }
      }
      break;
    }
    case Window::HK_ACTIVE_TOPMOST: {
      m_config.SetZOrder(1);
      SetWindowPos(m_window.GetHandle(), HWND_TOPMOST, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE);
      SetForegroundWindow(m_window.GetHandle());
      m_renderer.TriggerFlyText(L"FIXED FRONT");
      break;
    }
    case Window::HK_ACTIVE_BOTTOM: {
      m_config.SetZOrder(2);
      SetWindowPos(m_window.GetHandle(), HWND_BOTTOM, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      m_renderer.TriggerFlyText(L"FIXED BACK");
      break;
    }
    case Window::HK_EXIT_APP:
      this->ClearPlaylist();
      m_renderer.TriggerFlyText(L"EXIT");
      PostMessage(m_window.GetHandle(), WM_CLOSE, 0, 0);
      break;
    }
  });

  m_window.SetSkipCommandCallback([this](float offset) {
    float pos = m_audioManager.GetPositionSeconds();
    m_audioManager.Seek(pos + offset);
  });

  m_window.SetPlaylistScrollCallback([this](int delta) {
    float itemHeight = static_cast<float>(m_config.GetPlaylistItemOffsetY());
    float scrollDelta =
        (static_cast<float>(delta) / WHEEL_DELTA) * itemHeight * 2.0f;
    m_renderer.AddPlaylistScroll(scrollDelta);
  });

  m_window.SetPlaylistToolbarClickCallback([this](int btnIndex) {
    this->OnPlaylistToolbarClicked(btnIndex);
  });

  m_window.SetPlaylistClickCallback([this](int x, int y) {
    this->OnPlaylistClicked(x, y);
  });

  m_window.SetPlaylistDoubleClickCallback([this](int x, int y) {
    this->OnPlaylistDoubleClicked(x, y);
  });

  m_window.SetArtFramingMoveCallback([this](float dx, float dy) {
    if (m_playlistManager.IsEmpty() || m_isCurrentBackgroundPlaceholder) return;
    std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
    float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
    m_framingDb.GetFraming(currentTrack, artX, artY, artScale);
    artX -= dx;
    artY -= dy;
    m_framingDb.SetFraming(currentTrack, artX, artY, artScale);
    m_backgroundManager.SetArtFramingScale(artScale);
    m_backgroundManager.SetArtFramingScroll(artX, artY);
    this->ForceRender();
  });

  m_window.SetArtFramingScrollCallback([this](float delta, int x, int y) {
    if (m_playlistManager.IsEmpty() || m_isCurrentBackgroundPlaceholder) return;
    std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
    float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
    m_framingDb.GetFraming(currentTrack, artX, artY, artScale);

    float logicalX = x / m_renderer.GetDpiScale();
    float logicalY = y / m_renderer.GetDpiScale();
    
    RECT clientRect;
    GetClientRect(m_window.GetHandle(), &clientRect);
    float logicalWidth = clientRect.right / m_renderer.GetDpiScale();
    float logicalHeight = clientRect.bottom / m_renderer.GetDpiScale();
    float centerX = logicalWidth / 2.0f;
    float centerY = logicalHeight / 2.0f;
    
    float oldScale = artScale;
    artScale += delta * 0.001f;
    if (artScale < 1.0f) artScale = 1.0f;
    
    if (oldScale > 0.0f) {
        float relX = logicalX - centerX;
        float relY = logicalY - centerY;
        artX = artX - relX * (artScale / oldScale - 1.0f);
        artY = artY - relY * (artScale / oldScale - 1.0f);
    }
    m_framingDb.SetFraming(currentTrack, artX, artY, artScale);
    m_backgroundManager.SetArtFramingScale(artScale);
    m_backgroundManager.SetArtFramingScroll(artX, artY);
    this->ForceRender();
  });

  m_window.SetArtFramingResetCallback([this]() {
    if (m_playlistManager.IsEmpty() || m_isCurrentBackgroundPlaceholder) return;
    std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
    m_framingDb.SetFraming(currentTrack, 0.0f, 0.0f, 1.0f);
    m_backgroundManager.SetArtFramingScale(1.0f);
    m_backgroundManager.SetArtFramingScroll(0.0f, 0.0f);
    m_renderer.TriggerFlyText(L"FRAMING RESET");
    this->ForceRender();
  });

  m_window.SetPlaylistPinnedToggleCallback([this](bool pinned) {
      m_renderer.TriggerFlyText(pinned ? L"PINNED: ON" : L"PINNED: OFF");
  });

  m_window.SetVolumeScrollCallback([this](int delta) {
    float vol = m_audioManager.GetVolume();
    if (delta > 0) {
      vol += 0.05f;
    } else if (delta < 0) {
      vol -= 0.05f;
    }
    if (vol > 1.0f)
      vol = 1.0f;
    if (vol < 0.0f)
      vol = 0.0f;
    m_audioManager.SetVolume(vol);
    m_config.SetDefaultVolume(vol);
    m_renderer.TriggerVolumeOsd();
    if (delta != 0) {
      m_renderer.TriggerFlyText(
          L"VOL. " + std::to_wstring(static_cast<int>(vol * 100.0f + 0.5f)) +
          L"%");
    }
  });

  m_window.SetVirtualScrollCallback([this](int delta) {
    if (m_playlistManager.IsEmpty()) return;

    if (delta > 0) {
      m_playlistManager.Previous();
      m_renderer.ResetPlaylistScroll();
    } else if (delta < 0) {
      m_playlistManager.Advance();
      m_renderer.ResetPlaylistScroll();
    }

    m_virtualScrollTimer = m_config.GetJogDialConfirmDelay();

    auto dataProvider = [this](int relDist, DrumSlot* slot) -> TrackMetadata {
      size_t total = m_playlistManager.GetCount();
      size_t targetIdx = (m_playlistManager.GetCurrentIndex() + relDist + total) % total;
      
      std::wstring path = m_playlistManager.GetShuffleList()[targetIdx];

      if (slot) {
          bool isNew = false;
          slot->thumbId = m_thumbnailManager.GetOrGenerateThumbId(path, isNew);
          slot->artBitmap = m_thumbnailManager.GetCachedThumbnailBitmap(slot->thumbId);
          if (!slot->artBitmap) {
              m_thumbnailManager.RequestThumbnailLoad(slot->thumbId, m_renderer.GetD2DContext(), m_renderer.GetWicFactory());
          }
      }

      TrackMetadata meta;
      if (m_trackDatabase.GetMetadata(path, meta)) {
        return meta;
      } else {
        try { meta.title = std::filesystem::path(path).filename().wstring(); } catch (...) { meta.title = L"UNKNOWN"; }
        meta.artist = L"";
        return meta;
      }
    };

    // ドラムエンジンは「前の曲=+1」「次の曲=-1」を期待する
    int distanceForDrum = 0;
    if (delta > 0) {
      distanceForDrum = 1;
    } else if (delta < 0) {
      distanceForDrum = -1;
    }
    m_renderer.GetTrackDrum().StartDrumAnimation(distanceForDrum, m_config.GetTrackDrumMaxDuration(), m_config.GetTrackDrumMaxSpeed(), dataProvider, nullptr);
  });

  m_window.SetPowerSuspendCallback([this]() { this->OnPowerSuspend(); });
  m_window.SetPowerResumeCallback([this]() { this->OnPowerResume(); });
}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow) {
  wchar_t exePath[MAX_PATH];
  GetModuleFileNameW(NULL, exePath, MAX_PATH);
  std::wstring dbPath = std::filesystem::path(exePath).parent_path().wstring() + L"\\oztone_track.odb";
  m_trackDatabase.LoadFromFile(dbPath);

  m_framingDbPath = std::filesystem::path(exePath).parent_path().wstring() + L"\\oztone_framing.odb";
  m_framingDb.LoadFromFile(m_framingDbPath);

  if (!m_config.Initialize()) {
    return false;
  }

  if (!m_window.Initialize(hInstance, nCmdShow, m_config)) {
    return false;
  }

  SetupCallbacks();

  for (auto &item : m_window.GetLogoMenuItemsMutable()) {
    if (item.commandId == Window::ID_LOGO_SHUFFLE) {
      item.toggleState = m_config.GetShuffleMode();
    }
  }

  m_renderer.SetThumbnailManager(&m_thumbnailManager);
  m_renderer.SetBackgroundManager(&m_backgroundManager);
  if (!m_renderer.Initialize(m_window.GetHandle(), m_config)) {
    return false;
  }

  m_thumbnailManager.Initialize();
  m_backgroundManager.Initialize(&m_config);

  if (m_audioManager.Initialize()) {
    m_audioManager.SetVolume(m_config.GetDefaultVolume());
    std::wstring defPlaylist = m_config.GetDefaultPlaylistPath();
    m_playlistManager.LoadFromFile(defPlaylist, &m_framingDb);
    m_playlistManager.RebuildQueue(m_config.GetShuffleMode());

    if (!m_playlistManager.IsEmpty()) {
      size_t skipCount = 0;
      bool played = false;
      size_t totalCount = m_playlistManager.GetCount();

      while (skipCount < totalCount) {
        if (PlayCurrentTrack(-1)) {
          played = true;
          break;
        }

        m_playlistManager.Advance();
        skipCount++;
      }

      if (!played) {
        // UIの初期表示（空状態）
        m_renderer.GetTrackDrum().StartDrumAnimation(0, 0.0f, 0.0f, nullptr, nullptr);
        m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
      }
    } else {
      // UIの初期表示（空状態）
      m_renderer.GetTrackDrum().StartDrumAnimation(0, 0.0f, 0.0f, nullptr, nullptr);
      m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
    }
  }

  m_trackAnalyzer.Initialize(&m_trackDatabase, &m_config);

  std::vector<std::wstring> unparsed = m_playlistManager.GetShuffleList();
  if (!unparsed.empty()) {
    for (const auto &path : unparsed) {
      m_trackAnalyzer.AddTrackToQueue(path);
      bool isNew = false;
      uint32_t thumbId = m_thumbnailManager.GetOrGenerateThumbId(path, isNew);
      if (isNew) {
        m_thumbnailManager.EnqueueTrack(thumbId, path);
      }
    }
  }

  UpdatePlaylistSummaries();

  return true;
}
