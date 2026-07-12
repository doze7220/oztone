#include "Application.h"
#include "LayoutCalculator.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

Application::Application() {}

Application::~Application() {
  m_trackAnalyzer.Uninitialize();

  if (m_prefetchThread.joinable()) {
    m_prefetchThread.join();
  }
  m_audioPlayer.Uninitialize();

  if (!m_playlistManager.IsEmpty()) {
    m_playlistManager.SaveToFile(m_config.GetDefaultPlaylistPath());
  }

  wchar_t exePath[MAX_PATH];
  GetModuleFileNameW(NULL, exePath, MAX_PATH);
  std::wstring dbPath = std::filesystem::path(exePath).parent_path().wstring() + L"\\oztone_track.odb";
  m_trackDatabase.SaveToFile(dbPath);
  if (!m_framingDbPath.empty()) {
      m_framingDb.SaveToFile(m_framingDbPath);
  }
}

void Application::ResetAllSettings() {
  m_config.SaveDefaultSettings();
  m_config.ResetToDefaults();
  m_config.LoadSettings();
  m_renderer.ReloadResources();

  HWND hwnd = m_window.GetHWND();
  if (hwnd) {
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    if (GetMonitorInfoW(hMonitor, &mi)) {
      int screenWidth = mi.rcMonitor.right - mi.rcMonitor.left;
      int screenHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
      UINT dpi = GetDpiForWindow(hwnd);
      int pxWidth = MulDiv(1024, dpi, 96);
      int pxHeight = MulDiv(512, dpi, 96);
      int x = mi.rcMonitor.left + (screenWidth - pxWidth) / 2;
      int y = mi.rcMonitor.top + (screenHeight - pxHeight) / 2;
      SetWindowPos(hwnd, nullptr, x, y, pxWidth, pxHeight, SWP_NOZORDER | SWP_NOACTIVATE);
    }
  }
}







void Application::OnFilesDropped(const std::vector<std::wstring> &paths) {
  bool wasEmpty = m_playlistManager.IsEmpty();
  bool addedAny = false;

  auto IsSupportedAudioFile = [](const std::filesystem::path &p) {
    std::wstring ext = p.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    if (ext == L".mp3" || ext == L".flac" || ext == L".wav" || ext == L".ogg") {
      return true;
    }
    // [EXPERIMENTAL] MP4/M4A Support
    // if (ext == L".mp4" || ext == L".m4a") {
    //   return true;
    // }
    return false;
  };

  auto IsValidAudioFile = [](const std::filesystem::path &p) {
    std::wstring ext = p.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    // if (ext == L".mp4" || ext == L".m4a") {
    //   return true;
    // }

    std::ifstream file(p, std::ios::binary);
    if (!file.is_open())
      return false;

    unsigned char header[4] = {0};
    file.read(reinterpret_cast<char *>(header), 4);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead >= 4) {
      if (header[0] == 'f' && header[1] == 'L' && header[2] == 'a' &&
          header[3] == 'C')
        return true;
      if (header[0] == 'O' && header[1] == 'g' && header[2] == 'g' &&
          header[3] == 'S')
        return true;
      if (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' &&
          header[3] == 'F')
        return true;
    }

    if (bytesRead >= 2) {
      if (header[0] == 'I' && header[1] == 'D' && header[2] == '3')
        return true;
      if (header[0] == 0xFF && (header[1] & 0xE0) == 0xE0)
        return true;
    }

    return false;
  };

  std::wstring firstAddedTrack;

  for (const auto &pathWStr : paths) {
    try {
      std::filesystem::path p(pathWStr);
      if (std::filesystem::is_directory(p)) {
        for (auto it = std::filesystem::recursive_directory_iterator(p);
             it != std::filesystem::recursive_directory_iterator(); ++it) {
          if (it->is_regular_file()) {
            bool isSupported = IsSupportedAudioFile(it->path());
            bool isValid = IsValidAudioFile(it->path());

            if (isSupported && isValid) {
              if (m_playlistManager.Add(it->path().wstring())) {
                addedAny = true;
                if (firstAddedTrack.empty())
                  firstAddedTrack = it->path().wstring();
              }
            }
          }
        }
      } else if (std::filesystem::is_regular_file(p)) {
        bool isSupported = IsSupportedAudioFile(p);
        bool isValid = IsValidAudioFile(p);

        if (isSupported && isValid) {
          if (m_playlistManager.Add(p.wstring())) {
            addedAny = true;
            if (firstAddedTrack.empty())
              firstAddedTrack = p.wstring();
          }
        }
      }
    } catch (...) {
      // アクセス拒否などのエラーは無視
    }
  }

  if (addedAny) {
    std::filesystem::path defaultPath = m_config.GetDefaultPlaylistPath();
    std::filesystem::path playlistDir = defaultPath.parent_path();
    if (!playlistDir.empty() && !std::filesystem::exists(playlistDir)) {
      std::filesystem::create_directories(playlistDir);
    }
    m_playlistManager.SaveToFile(defaultPath.wstring());
    UpdatePlaylistSummaries();

    bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (!isShiftPressed) {
      m_playlistManager.RebuildQueue(m_config.GetShuffleMode());
      if (!firstAddedTrack.empty()) {
        m_playlistManager.WarpToTrack(firstAddedTrack);
      }
    } else {
      m_playlistManager.ShuffleNextLoop();
    }

    std::vector<std::wstring> unparsed = m_playlistManager.GetShuffleList();
    if (!unparsed.empty()) {
      for (const auto &path : unparsed) {
        m_trackAnalyzer.AddTrackToQueue(path);
      }
    }

    if (!isShiftPressed || (wasEmpty && !m_audioPlayer.IsPlaying())) {
      m_audioPlayer.Stop();

      size_t skipCount = 0;
      bool played = false;
      size_t totalCount = m_playlistManager.GetCount();

      while (skipCount < totalCount) {
        std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
        if (m_tagManager.Load(currentTrack)) {
          std::wstring title = m_tagManager.GetTitle();
          std::wstring artist = m_tagManager.GetArtist();
          if (title.empty())
            title = std::filesystem::path(currentTrack).filename().wstring();
          if (artist.empty())
            artist = L"---";
          m_renderer.SetTrackInfo(title, artist);

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
            title = std::filesystem::path(currentTrack).filename().wstring();
          } catch (...) {
            title = L"UNKNOWN";
          }
          m_renderer.SetTrackInfo(title, L"---");
          m_renderer.SetAlbumArt(nullptr);
        }

        float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
      m_framingDb.GetFraming(currentTrack, artX, artY, artScale);
      m_renderer.SetBackgroundFraming(artX, artY, artScale);
      if (PlayCurrentTrack()) {
          played = true;
          break;
        }

        m_playlistManager.Advance();
        skipCount++;
      }

      if (!played) {
        m_renderer.SetTrackInfo(L"NO TRACK", L"---");
        m_renderer.SetAlbumArt(nullptr);
      }
    }
  }
}

void Application::Run() {
  while (m_window.ProcessMessages()) {
    ULONGLONG currentTime = GetTickCount64();
    if (currentTime - m_lastConfigCheckTime >= 1000) {
      m_lastConfigCheckTime = currentTime;
      if (m_config.CheckForUpdates()) {
        m_config.LoadSettings();
        m_renderer.ReloadResources();
        m_window.RegisterHotkeys();
      }
    }

    if (m_audioPlayer.IsAtEnd()) {
      // ロードが完了するまで待機（このフレームはスキップして待つ）
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
        m_playlistManager.Advance();

        size_t skipCount = 0;
        bool played = false;
        size_t totalCount = m_playlistManager.GetCount();

        while (skipCount < totalCount) {
          std::wstring track = m_playlistManager.GetCurrentTrack();

          if (skipCount == 0) {
            // 最初の曲は先読みデータを利用
            m_renderer.SetTrackInfo(m_prefetchedTitle, m_prefetchedArtist);
            m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());
          } else {
            // スキップされた場合は同期的にタグを読み直す
            if (m_tagManager.Load(track)) {
              std::wstring title = m_tagManager.GetTitle();
              std::wstring artist = m_tagManager.GetArtist();
              if (title.empty())
                title = std::filesystem::path(track).filename().wstring();
              if (artist.empty())
                artist = L"---";
              m_renderer.SetTrackInfo(title, artist);

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
              m_renderer.SetTrackInfo(title, L"---");
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
          m_renderer.SetTrackInfo(L"NO TRACK", L"---");
          m_renderer.SetAlbumArt(nullptr);
        }
      }
    }

    ForceRender();
    Sleep(1); // CPU使用率を抑えるための仮のスリープ
  }
}

void Application::ForceRender() {
  m_renderer.SetFocusedPlaylistIndex(m_focusedPlaylistIndex);

  // 1. 時間と進行度の計算
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

  // 2. スペクトルデータの取得
  m_renderer.SetNextTrackInfo(m_isPrefetchReady.load(),
                              m_prefetchedAlbumArt.Get(), m_prefetchedTitle,
                              m_prefetchedArtist);

  std::vector<float> spectrum;
  m_audioPlayer.GetSpectrumData(spectrum);

  // 3. アニメーションと状態の更新
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

  m_renderer.UpdateAnimation(
      0.016f, m_window.IsControlHovered(), m_window.IsVolumeHovered(),
      m_window.IsPlaylistHovered(), m_window.IsLogoMenuHovered(),
      m_window.GetLogoMenuHoveredIndex(), m_playlistManager.GetCurrentIndex(),
      m_playlistManager.GetCount(), m_isPlaylistListViewMode,
      m_window.GetPlaybackHoveredIndex(), playlistHoveredItemIndex,
      &m_window.GetLogoMenuItems(), logoClicked, logoMenuClicked,
      playbackClicked, &isPlaylistExpanded, &isLogoMenuExpanded);

  m_window.SetPlaylistExpanded(isPlaylistExpanded);
  m_window.SetLogoMenuExpanded(isLogoMenuExpanded);

  // 4. レイアウトキャッシュの更新
  m_renderer.UpdateTextLayouts(timeString, m_audioPlayer.GetVolume(),
                               m_playlistManager.GetCurrentIndex(),
                               m_playlistManager.GetCount());

  // 5. 描画の実行
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


void Application::ProcessCommandLineArgs(int argc, LPWSTR *argv) {
  if (argc <= 1 || !argv)
    return;
  std::vector<std::wstring> files;
  for (int i = 1; i < argc; ++i) {
    files.push_back(argv[i]);
  }
  if (!files.empty()) {
    OnFilesDropped(files);
  }
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

