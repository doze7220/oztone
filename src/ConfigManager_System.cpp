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

void ConfigManager::LoadSystemSettings() {
    // Layout_GlobalHotkeys
    m_ghFontFamily = LoadOrWriteString(L"Layout_GlobalHotkeys", L"FontFamily", L"MS Gothic");
    m_ghFontSize = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"FontSize", 15.0f);
    m_ghLineSpacing = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"LineSpacing", 20.0f);
    m_ghCoreColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"CoreColor", L"#FFFFFF");
    m_ghGlowColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"GlowColor", L"#00FFFF");
    m_ghGlowOpacity = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"GlowOpacity", 0.10f);
    m_ghKeyColumnOffset = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"KeyColumnOffset", 360.0f);
    m_ghActionColumnOffset = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"ActionColumnOffset", 140.0f);
    m_ghShadowColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"ShadowColor", L"#000000");
    m_ghShadowOpacity = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"ShadowOpacity", 0.2f);

    // Layout_OSD
    m_osdFontFamily = LoadOrWriteString(L"Layout_OSD", L"OsdFontFamily", L"MS Gothic");
    m_osdFontSize = LoadOrWriteFloat(L"Layout_OSD", L"OsdFontSize", 48.0f);
    m_osdTextColor = LoadOrWriteString(L"Layout_OSD", L"OsdTextColor", L"#FFFFFF");
    m_osdShadowColor = LoadOrWriteString(L"Layout_OSD", L"OsdShadowColor", L"#000000");
    m_osdShadowOffsetX = LoadOrWriteFloat(L"Layout_OSD", L"OsdShadowOffsetX", 2.0f);
    m_osdShadowOffsetY = LoadOrWriteFloat(L"Layout_OSD", L"OsdShadowOffsetY", 2.0f);
    m_osdShadowOpacity = LoadOrWriteFloat(L"Layout_OSD", L"OsdShadowOpacity", 0.8f);
    m_osdFadeWait = LoadOrWriteFloat(L"Layout_OSD", L"OsdFadeWait", 1.5f);
    m_osdFadeSpeed = LoadOrWriteFloat(L"Layout_OSD", L"OsdFadeSpeed", 1.5f);
    m_enableOSD = LoadOrWriteInt(L"Layout_OSD", L"EnableOSD", 1) != 0;

    // GlobalHotkeys
    m_showHotkeys = LoadOrWriteInt(L"GlobalHotkeys", L"ShowHotkeys", 0) != 0;
    m_modNextTrack = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_NextTrack", 11);
    m_vkNextTrack = LoadOrWriteInt(L"GlobalHotkeys", L"VK_NextTrack", 39);
    m_modPrevTrack = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PrevTrack", 11);
    m_vkPrevTrack = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PrevTrack", 37);
    m_modPlayPause = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PlayPause", 11);
    m_vkPlayPause = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PlayPause", 38);
    m_modStop = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_Stop", 11);
    m_vkStop = LoadOrWriteInt(L"GlobalHotkeys", L"VK_Stop", 40);
    m_modVolUp5 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolUp5", 11);
    m_vkVolUp5 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolUp5", 36);
    m_modVolDown5 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolDown5", 11);
    m_vkVolDown5 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolDown5", 35);
    m_modVolUp25 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolUp25", 11);
    m_vkVolUp25 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolUp25", 33);
    m_modVolDown25 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolDown25", 11);
    m_vkVolDown25 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolDown25", 34);
    m_modPrevPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PrevPlaylist", 10);
    m_vkPrevPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PrevPlaylist", 33);
    m_modNextPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_NextPlaylist", 10);
    m_vkNextPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"VK_NextPlaylist", 34);
    m_modActiveTopMost = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ActiveTopMost", 10);
    m_vkActiveTopMost = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ActiveTopMost", 36);
    m_modActiveBottom = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ActiveBottom", 10);
    m_vkActiveBottom = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ActiveBottom", 35);
    m_modExitApp = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ExitApp", 10);
    m_vkExitApp = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ExitApp", 46);
}
