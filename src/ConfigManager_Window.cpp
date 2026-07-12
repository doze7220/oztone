#include "ConfigManager.h"

void ConfigManager::SaveWindowPosition(int x, int y, int width, int height) {
  m_windowX = x;
  m_windowY = y;
  m_windowWidth = width;
  m_windowHeight = height;

  WritePrivateProfileStringW(L"Window", L"WindowX", std::to_wstring(x).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowY", std::to_wstring(y).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowWidth",
                             std::to_wstring(width).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowHeight",
                             std::to_wstring(height).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetZOrder(int zOrder) {
  m_zOrder = zOrder;
  WritePrivateProfileStringW(L"Window", L"ZOrder",
                             std::to_wstring(zOrder).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetSavePositionOnExit(bool save) {
  m_savePositionOnExit = save;
  WritePrivateProfileStringW(L"Window", L"SavePositionOnExit",
                             save ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetEnableResize(bool enable) {
  m_enableResize = enable;
  WritePrivateProfileStringW(L"Window", L"EnableResize", enable ? L"1" : L"0",
                             m_iniFilePath.c_str());
}

void ConfigManager::SetLockWindowPosition(bool lock) {
  m_lockWindowPosition = lock;
  WritePrivateProfileStringW(L"Window", L"LockWindowPosition",
                             lock ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetBackgroundArtMode(int mode) {
  m_backgroundArtMode = mode;
  WritePrivateProfileStringW(L"Layout_Window", L"BackgroundArtMode",
                             std::to_wstring(mode).c_str(),
                             m_iniFilePath.c_str());
}
