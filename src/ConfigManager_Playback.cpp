#include "ConfigManager.h"
#include <string>

void ConfigManager::SetDefaultVolume(float volume) {
  m_defaultVolume = volume;
  WritePrivateProfileStringW(L"Audio", L"DefaultVolume",
                             std::to_wstring(volume).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetShuffleMode(bool mode) {
  m_shuffleMode = mode;
  WritePrivateProfileStringW(L"Audio", L"ShuffleMode", mode ? L"1" : L"0",
                             m_iniFilePath.c_str());
}

void ConfigManager::SetSkipSeconds(float seconds) {
  m_skipSeconds = seconds;
  wchar_t buf[32];
  swprintf_s(buf, L"%.1f", seconds);
  WritePrivateProfileStringW(L"Audio", L"SkipSeconds", buf,
                             m_iniFilePath.c_str());
}
