#include "Application.h"
#include "LayoutCalculator.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

void Application::Run() {
  while (m_window.ProcessMessages()) {
    if (m_isWaitingForDevice) {
      if (m_audioPlayer.HasValidOutputDevice()) {
        m_isWaitingForDevice = false;
        if (m_audioPlayer.Initialize()) {
          m_audioPlayer.SetVolume(m_config.GetDefaultVolume());
          std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
          if (!currentTrack.empty()) {
            if (m_audioPlayer.Play(currentTrack)) {
              m_audioPlayer.Seek(m_suspendPosition);
              if (!m_suspendIsPlaying) {
                m_audioPlayer.TogglePlayPause();
              }
            }
          }
        }
      } else {
        Sleep(10);
        continue;
      }
    }

    ULONGLONG currentTime = GetTickCount64();

    if (!m_isWaitingForDevice && m_audioPlayer.IsPlaying()) {
      float currentPos = m_audioPlayer.GetPositionSeconds();
      if (m_watchdogState == WatchdogState::Normal) {
        float intervalMs = m_config.GetWatchdogInterval() * 1000.0f;
        if (currentTime - m_lastWatchdogPollTime >= static_cast<ULONGLONG>(intervalMs)) {
          m_lastWatchdogPollTime = currentTime;
          if (m_lastWatchdogPosition >= 0.0f && currentPos == m_lastWatchdogPosition) {
            m_watchdogState = WatchdogState::Warning;
            m_watchdogWarningStartTime = currentTime;
          }
          m_lastWatchdogPosition = currentPos;
        }
      } else if (m_watchdogState == WatchdogState::Warning) {
        if (currentPos != m_lastWatchdogPosition) {
          m_watchdogState = WatchdogState::Normal;
          m_lastWatchdogPosition = currentPos;
          m_lastWatchdogPollTime = currentTime;
        } else {
          float timeoutMs = m_config.GetWatchdogTimeout() * 1000.0f;
          if (currentTime - m_watchdogWarningStartTime >= static_cast<ULONGLONG>(timeoutMs)) {
            m_suspendPosition = currentPos;
            m_suspendIsPlaying = true;
            m_audioPlayer.Uninitialize();
            m_isWaitingForDevice = true;
            m_watchdogState = WatchdogState::Normal;
            m_lastWatchdogPosition = -1.0f;
            continue;
          }
        }
      }
    } else {
      m_watchdogState = WatchdogState::Normal;
      m_lastWatchdogPosition = -1.0f;
    }

    if (currentTime - m_lastConfigCheckTime >= 1000) {
      m_lastConfigCheckTime = currentTime;
      if (m_config.CheckForUpdates()) {
        m_config.LoadSettings();
        m_renderer.ReloadResources();
        m_window.RegisterHotkeys();
      }
    }

    if (m_audioPlayer.IsAtEnd()) {
      // 繝ｭ繝ｼ繝峨′螳御ｺ・☆繧九∪縺ｧ蠕・ｩ滂ｼ医％縺ｮ繝輔Ξ繝ｼ繝縺ｯ繧ｹ繧ｭ繝・・縺励※蠕・▽・・
      if (m_isPrefetchReady.load()) {
        std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
        if (m_audioPlayer.IsLearningValid()) {
          float learnedPeak = m_audioPlayer.GetLearningPeakAmplitude();
          float learnedFreq = m_audioPlayer.GetLearningMaxFrequency();
          TrackMetadata meta;
          if (m_trackDatabase.GetMetadata(currentTrack, meta)) {
            if (learnedPeak > meta.peakAmplitude || meta.peakAmplitude == 0.0f) {
              meta.peakAmplitude = learnedPeak;
              meta.maxFrequency = learnedFreq;
              meta.isFFTLoaded = true;
              m_trackDatabase.UpdateMetadata(currentTrack, meta);
            }
          }
        }

        if (!m_playlistManager.IsEmpty()) {
          m_playlistManager.SaveToFile(m_config.GetDefaultPlaylistPath());
          if (!m_framingDbPath.empty()) {
            m_framingDb.SaveToFile(m_framingDbPath);
          }
        }
        
        size_t prevIndex = m_playlistManager.GetCurrentIndex();
        m_playlistManager.Advance();
        if (m_playlistManager.GetCurrentIndex() <= prevIndex) {
            m_isContinuousStream = false;
            m_streamBreakDirection = StreamBreakDirection::Next;
        }

        size_t skipCount = 0;
        bool played = false;
        size_t totalCount = m_playlistManager.GetCount();

        while (skipCount < totalCount) {
          std::wstring track = m_playlistManager.GetCurrentTrack();

          if (!m_isContinuousStream) {
              m_renderer.ResetDrumPosition(m_playlistManager.GetCurrentIndex(), m_streamBreakDirection == StreamBreakDirection::Next);
              m_isContinuousStream = true;
          }

          if (skipCount == 0) {
            // 譛蛻昴・譖ｲ縺ｯ蜈郁ｪｭ縺ｿ繝・・繧ｿ繧貞茜逕ｨ
            m_renderer.SetTrackInfo(m_prefetchedTitle, m_prefetchedArtist, m_playlistManager.GetCurrentIndex());
            m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());
          } else {
            // 繧ｹ繧ｭ繝・・縺輔ｌ縺溷ｴ蜷医・蜷梧悄逧・↓繧ｿ繧ｰ繧定ｪｭ縺ｿ逶ｴ縺・
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
                title = L"UNKNOWN";
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

          m_playlistManager.Advance();
          skipCount++;
        }

        if (!played) {
          m_renderer.SetTrackInfo(L"NO TRACK", L"---", 0, true);
          m_renderer.SetAlbumArt(nullptr);
        }
      }
    }

    ForceRender();
    Sleep(1); // CPU菴ｿ逕ｨ邇・ｒ謚代∴繧九◆繧√・莉ｮ縺ｮ繧ｹ繝ｪ繝ｼ繝・
  }
}

void Application::ForceRender() {
  m_renderer.SetFocusedPlaylistIndex(m_focusedPlaylistIndex);
  m_renderer.SetShuffleIndices(m_playlistManager.GetShuffleIndices());

  // 1. 譎る俣縺ｨ騾ｲ陦悟ｺｦ縺ｮ險育ｮ・
  float posSec = m_audioPlayer.GetPositionSeconds();
  float lenSec = m_audioPlayer.GetLengthSeconds();

  int posM = static_cast<int>(posSec) / 60;
  int posS = static_cast<int>(posSec) % 60;
  int lenM = static_cast<int>(lenSec) / 60;
  int lenS = static_cast<int>(lenSec) % 60;

  wchar_t timeBuf[32];
  swprintf_s(timeBuf, L"%d:%02d / %d:%02d", posM, posS, lenM, lenS);
  std::wstring timeString(timeBuf);

  float progress = 0.0f;
  if (lenSec > 0.0f) {
    progress = posSec / lenSec;
    if (progress > 1.0f)
      progress = 1.0f;
    if (progress < 0.0f)
      progress = 0.0f;
  }

  // 2. 繧ｹ繝壹け繝医Ν繝・・繧ｿ縺ｮ蜿門ｾ・


  std::vector<float> spectrum;
  m_audioPlayer.GetSpectrumData(spectrum);

  // 3. 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ縺ｨ迥ｶ諷九・譖ｴ譁ｰ
  int playlistHoveredItemIndex = -1;
  if (m_window.IsPlaylistHovered()) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(m_window.GetHandle(), &pt);
    UINT dpi = GetDpiForWindow(m_window.GetHandle());
    float logicalY =
        static_cast<float>(pt.y) / (static_cast<float>(dpi) / 96.0f);

    RECT rect;
    GetClientRect(m_window.GetHandle(), &rect);
    float logicalWidth = static_cast<float>(rect.right - rect.left) /
                         (static_cast<float>(dpi) / 96.0f);
    float logicalHeight = static_cast<float>(rect.bottom - rect.top) /
                          (static_cast<float>(dpi) / 96.0f);

    if (m_isPlaylistListViewMode) {
      std::vector<std::wstring> playlists = m_config.GetAvailablePlaylists();
      int totalPlaylists = static_cast<int>(playlists.size());
      std::wstring currentPlaylist = m_config.GetDefaultPlaylistPath();
      int currentIndex = 0;
      for (int i = 0; i < totalPlaylists; ++i) {
        if (playlists[i] == currentPlaylist) {
          currentIndex = i;
          break;
        }
      }
      PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
          logicalWidth, logicalHeight, &m_config, 0.0f,
          m_renderer.GetPlaylistManualScrollY(), currentIndex, totalPlaylists);
      playlistHoveredItemIndex = LayoutCalculator::GetPlaylistItemIndexAt(
          logicalY, layout, totalPlaylists);
    } else {
      int totalTracks = static_cast<int>(m_playlistManager.GetCount());
      int currentTrackIndex =
          static_cast<int>(m_playlistManager.GetCurrentIndex());
      PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
          logicalWidth, logicalHeight, &m_config, 0.0f,
          m_renderer.GetPlaylistManualScrollY(), currentTrackIndex,
          totalTracks);
      playlistHoveredItemIndex = LayoutCalculator::GetPlaylistItemIndexAt(
          logicalY, layout, totalTracks);
    }
  }
  m_window.SetPlaylistHoveredItemIndex(playlistHoveredItemIndex);

  bool logoClicked = m_window.ConsumeLogoClicked();
  int logoMenuClicked = m_window.ConsumeLogoMenuClickedIndex();
  int playbackClicked = m_window.ConsumePlaybackClickedIndex();

  if (logoMenuClicked >= 0) {
    const auto& items = m_window.GetLogoMenuItems();
    if (logoMenuClicked < items.size()) {
      int cmdId = items[logoMenuClicked].commandId;
      if (cmdId == Window::ID_LOGO_VISUALIZER) {
        int mode = m_config.GetVisualizerMode();
        if (mode == 1) m_renderer.TriggerFlyText(L"VISUALIZER: PRISM BEAT");
        else if (mode == 2) m_renderer.TriggerFlyText(L"VISUALIZER: HALO DUST");
        else m_renderer.TriggerFlyText(L"VISUALIZER: OFF");
      } else if (cmdId == Window::ID_LOGO_BG_MODE) {
        int mode = m_config.GetBackgroundArtMode();
        if (mode == 0) m_renderer.TriggerFlyText(L"BACKGROUND: NOW PLAYING");
        else if (mode == 1) m_renderer.TriggerFlyText(L"BACKGROUND: HIDDEN");
        else m_renderer.TriggerFlyText(L"BACKGROUND: DEFAULT");
      } else if (cmdId == Window::ID_LOGO_RESIZE_MODE) {
        bool on = m_config.GetEnableResize();
        m_renderer.TriggerFlyText(on ? L"RESIZE MODE: ON" : L"RESIZE MODE: OFF");
      } else if (cmdId == Window::ID_LOGO_LOCK_POS) {
        bool on = m_config.GetLockWindowPosition();
        m_renderer.TriggerFlyText(on ? L"WINDOW LOCK: ON" : L"WINDOW LOCK: OFF");
      } else if (cmdId == Window::ID_LOGO_PLAYLIST_POS) {
        int pos = m_config.GetPlaylistPosition();
        m_renderer.TriggerFlyText(pos == 0 ? L"PLAYLIST POS: LEFT" : L"PLAYLIST POS: RIGHT");
      }
    }
  }

  bool isPlaylistExpanded = false;
  bool isLogoMenuExpanded = false;

  bool wasDrumAnimating = m_renderer.IsDrumAnimating();

  m_renderer.UpdateAnimation(
      0.016f, m_window.IsControlHovered(), m_window.IsVolumeHovered(),
      m_window.IsPlaylistHovered(), m_window.IsLogoMenuHovered(),
      m_window.GetLogoMenuHoveredIndex(), m_playlistManager.GetCurrentIndex(),
      m_playlistManager.GetCount(), m_isPlaylistListViewMode,
      m_window.GetPlaybackHoveredIndex(), playlistHoveredItemIndex,
      &m_window.GetLogoMenuItems(), logoClicked, logoMenuClicked,
      playbackClicked, &isPlaylistExpanded, &isLogoMenuExpanded);

  if (wasDrumAnimating && !m_renderer.IsDrumAnimating()) {
      LoadCurrentTrackArtAsync();
  }

  if (m_isCurrentArtLoadReady.load()) {
      m_isCurrentArtLoadReady.store(false);
      if (m_currentArtThread.joinable()) {
          m_currentArtThread.join();
      }
      m_renderer.SetAlbumArt(m_loadedCurrentArt.Get());
  }

  m_window.SetPlaylistExpanded(isPlaylistExpanded);
  m_window.SetLogoMenuExpanded(isLogoMenuExpanded);

  // 4. 繝ｬ繧､繧｢繧ｦ繝医く繝｣繝・す繝･縺ｮ譖ｴ譁ｰ
  m_renderer.UpdateTextLayouts(timeString, m_audioPlayer.GetVolume(),
                               m_playlistManager.GetCurrentIndex(),
                               m_playlistManager.GetCount());

  // 5. 謠冗判縺ｮ螳溯｡・
  std::vector<std::wstring> shuffleList = m_playlistManager.GetShuffleList();
  std::vector<TrackMetadata> metadataList;
  metadataList.reserve(shuffleList.size());
  for (const auto& path : shuffleList) {
    TrackMetadata meta;
    if (!m_trackDatabase.GetMetadata(path, meta)) {
      meta.filepath = path;
      try {
        meta.title = std::filesystem::path(path).filename().wstring();
      } catch (...) {
        meta.title = L"UNKNOWN";
      }
      meta.artist = L"---";
    }
    metadataList.push_back(meta);
  }

  m_renderer.Render(
      m_window.IsHovered(), m_window.IsControlHovered(),
      m_window.IsVolumeHovered(), m_window.IsPlaylistHovered(),
      m_window.IsLogoMenuHovered(), m_window.GetLogoMenuHoveredIndex(),
      &m_window.GetLogoMenuItems(), m_isPlaylistListViewMode,
      m_audioPlayer.IsPlaying(), progress, spectrum, m_audioPlayer.GetVolume(),
      m_playlistManager.GetCurrentIndex(), m_playlistManager.GetCount(),
      metadataList,
      m_window.GetPlaylistToolbarHoveredIndex(), &m_playlistSummaries);
}

void Application::UpdatePlaylistSummaries() {
  std::vector<std::wstring> available = m_config.GetAvailablePlaylists();
  std::vector<PlaylistSummary> summaries;

  auto parseTime = [](const std::wstring &tStr) -> long long {
    if (tStr.empty() || tStr == L"---")
      return 0;
    long long total = 0;
    std::wstringstream ss(tStr);
    std::wstring part;
    std::vector<long long> parts;
    while (std::getline(ss, part, L':')) {
      try {
        parts.push_back(std::stoll(part));
      } catch (...) {
        return 0;
      }
    }
    if (parts.size() == 2) {
      total = parts[0] * 60 + parts[1];
    } else if (parts.size() == 3) {
      total = parts[0] * 3600 + parts[1] * 60 + parts[2];
    }
    return total;
  };

  for (const auto &path : available) {
    PlaylistSummary summary;
    summary.filepath = path;

    std::filesystem::path p(path);
    summary.displayName = p.stem().wstring();

    size_t count = 0;
    long long totalSeconds = 0;
    bool hasUnparsed = false;

    std::ifstream ifs(path, std::ios::binary);
    if (ifs) {
      std::string line;
      while (std::getline(ifs, line)) {
        if (line.empty())
          continue;
        if (line.back() == '\r')
          line.pop_back();
        count++;

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, line.c_str(),
                                              (int)line.length(), NULL, 0);
        if (size_needed > 0) {
          std::wstring wline(size_needed, 0);
          MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.length(),
                              &wline[0], size_needed);

          std::wstringstream wss(wline);
          std::wstring token;
          std::getline(wss, token, L'\t'); // token is filepath

          TrackMetadata meta;
          if (m_trackDatabase.GetMetadata(token, meta) && meta.isMetaLoaded) {
            totalSeconds += parseTime(meta.timeString);
          } else {
            hasUnparsed = true;
          }
        } else {
          hasUnparsed = true;
        }
      }
    }

    summary.trackCount = count;
    if (hasUnparsed) {
      summary.totalTimeString = L"---";
    } else {
      long long h = totalSeconds / 3600;
      long long m = (totalSeconds % 3600) / 60;
      long long s = totalSeconds % 60;
      wchar_t timeBuf[32];
      if (h > 0) {
        swprintf_s(timeBuf, L"%lld:%02lld:%02lld", h, m, s);
      } else {
        swprintf_s(timeBuf, L"%lld:%02lld", m, s);
      }
      summary.totalTimeString = timeBuf;
    }
    summaries.push_back(summary);
  }
  m_playlistSummaries = summaries;
}
