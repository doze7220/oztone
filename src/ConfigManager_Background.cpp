#include "ConfigManager.h"
#include <string>

void ConfigManager::SetBackgroundArtMode(int mode) {
  m_backgroundArtMode = mode;
  WritePrivateProfileStringW(L"Background", L"BackgroundArtMode",
                             std::to_wstring(mode).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetCrossfadeDuration(float duration) {
  m_crossfadeDuration = duration;
  WritePrivateProfileStringW(L"Background", L"CrossfadeDuration",
                             std::to_wstring(duration).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::LoadBackgroundSettings() {
  m_bgOpacity = LoadOrWriteFloat(L"Background", L"BgOpacity");
  m_bgDarkenOpacity = LoadOrWriteFloat(L"Background", L"BgDarkenOpacity");
  m_backgroundArtMode = LoadOrWriteInt(L"Background", L"BackgroundArtMode");
  m_crossfadeDuration = LoadOrWriteFloat(L"Background", L"CrossfadeDuration");
}
