#include "ConfigManager.h"

#include <windows.h>
#include <string>

// [GlobalHotkeys] セクションの設定
void ConfigManager::SetShowHotkeys(bool show) {
    m_showHotkeys = show;
    std::wstring value = show ? L"1" : L"0";
    WritePrivateProfileStringW(L"GlobalHotkeys", L"ShowHotkeys", value.c_str(), m_iniFilePath.c_str());
}

// [Layout_OSD] セクションの設定
void ConfigManager::SetEnableOSD(bool enable) {
    m_enableOSD = enable;
    std::wstring value = enable ? L"1" : L"0";
    WritePrivateProfileStringW(L"Layout_OSD", L"EnableOSD", value.c_str(), m_iniFilePath.c_str());
}
