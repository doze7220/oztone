#include "Application.h"
#include <filesystem>

Application::Application() 
    : m_thumbnailDatabase(&m_config), 
      m_thumbCacher(&m_thumbnailDatabase) 
{}

Application::~Application() {
  m_thumbCacher.Uninitialize();
  m_trackAnalyzer.Uninitialize();

  m_audioPlayer.Uninitialize();

  m_config.SetDefaultPlaylistPath(m_config.GetDefaultPlaylistPath());

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

void Application::OnPowerSuspend() {
  m_suspendIsPlaying = m_audioPlayer.IsPlaying();
  m_suspendPosition = m_audioPlayer.GetPositionSeconds();
  m_audioPlayer.Uninitialize();
}

void Application::OnPowerResume() {
  m_isWaitingForDevice = true;
}

