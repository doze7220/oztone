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

void ConfigManager::LoadWindowSettings() {
  m_showTitleBar = LoadOrWriteInt(L"Window", L"ShowTitleBar", 0) != 0;
  m_showWindowFrame = LoadOrWriteInt(L"Window", L"ShowWindowFrame", 0) != 0;
  m_showTaskbar = LoadOrWriteInt(L"Window", L"ShowTaskbar", 0) != 0;
  m_zOrder = LoadOrWriteInt(L"Window", L"ZOrder", 0);
  m_savePositionOnExit = LoadOrWriteInt(L"Window", L"SavePositionOnExit", 1) != 0;
  m_enableResize = LoadOrWriteInt(L"Window", L"EnableResize", 0) != 0;
  m_lockWindowPosition = LoadOrWriteInt(L"Window", L"LockWindowPosition", 0) != 0;
  
  m_windowX = LoadOrWriteInt(L"Window", L"WindowX", -2147483648);
  m_windowY = LoadOrWriteInt(L"Window", L"WindowY", -2147483648);
  m_windowWidth = LoadOrWriteInt(L"Window", L"WindowWidth", 1024);
  m_windowHeight = LoadOrWriteInt(L"Window", L"WindowHeight", 512);

  m_enableShadow = LoadOrWriteInt(L"Layout_Window", L"EnableShadow", 1) != 0;
  m_shadowOffsetX = LoadOrWriteFloat(L"Layout_Window", L"ShadowOffsetX", 2.0f);
  m_shadowOffsetY = LoadOrWriteFloat(L"Layout_Window", L"ShadowOffsetY", 2.0f);
  m_shadowOpacity = LoadOrWriteFloat(L"Layout_Window", L"ShadowOpacity", 0.7f);
  m_bgOpacity = LoadOrWriteFloat(L"Layout_Window", L"BgOpacity", 0.8f);
  m_bgDarkenOpacity = LoadOrWriteFloat(L"Layout_Window", L"BgDarkenOpacity", 0.4f);
  m_backgroundArtMode = LoadOrWriteInt(L"Layout_Window", L"BackgroundArtMode", 0);
  m_controlHoverHeight = LoadOrWriteFloat(L"Layout_Window", L"ControlHoverHeight", 50.0f);
  m_controlLeaveDelay = LoadOrWriteFloat(L"Layout_Window", L"ControlLeaveDelay", 3.0f);
  m_hoverIconColor = LoadOrWriteString(L"Layout_Window", L"HoverIconColor", L"#ffa500");
  m_hoverFadeOutSpeed = LoadOrWriteFloat(L"Layout_Window", L"HoverFadeOutSpeed", 2.0f);
}
