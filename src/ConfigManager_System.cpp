#include "ConfigManager.h"

#include <windows.h>
#include <string>
#include <algorithm>

// [GlobalHotkeys]
void ConfigManager::SetShowHotkeys(bool show) {
    m_showHotkeys = show;
    std::wstring value = show ? L"1" : L"0";
    WritePrivateProfileStringW(L"GlobalHotkeys", L"ShowHotkeys", value.c_str(), m_iniFilePath.c_str());
}

// [Layout_OSD]
void ConfigManager::SetEnableOSD(bool enable) {
    m_enableOSD = enable;
    std::wstring value = enable ? L"1" : L"0";
    WritePrivateProfileStringW(L"Layout_OSD", L"EnableOSD", value.c_str(), m_iniFilePath.c_str());
}

// [System]
void ConfigManager::SetMaxThumbnailCache(int cache) {
    m_maxThumbnailCache = cache;
    std::wstring value = std::to_wstring(cache);
    WritePrivateProfileStringW(L"System", L"MaxThumbnailCache", value.c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetThumbnailSize(float size) {
    m_thumbnailSize = size;
    std::wstring value = std::to_wstring(size);
    WritePrivateProfileStringW(L"System", L"ThumbnailSize", value.c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetThumbnailJpegQuality(float quality) {
    m_thumbnailJpegQuality = std::clamp(quality, 0.0f, 1.0f);
    std::wstring value = std::to_wstring(m_thumbnailJpegQuality);
    WritePrivateProfileStringW(L"System", L"ThumbnailJpegQuality", value.c_str(), m_iniFilePath.c_str());
}

void ConfigManager::LoadSystemSettings() {
    // Layout_GlobalHotkeys
    m_ghFontSize = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"FontSize");
    m_ghLineSpacing = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"LineSpacing");
    m_ghCoreColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"CoreColor");
    m_ghGlowColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"GlowColor");
    m_ghGlowOpacity = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"GlowOpacity");
    m_ghKeyColumnOffset = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"KeyColumnOffset");
    m_ghActionColumnOffset = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"ActionColumnOffset");

    // Layout_OSD
    m_osdFontSize = LoadOrWriteFloat(L"Layout_OSD", L"OsdFontSize");
    m_osdTextColor = LoadOrWriteString(L"Layout_OSD", L"OsdTextColor");
    m_osdFadeWait = LoadOrWriteFloat(L"Layout_OSD", L"OsdFadeWait");
    m_osdFadeSpeed = LoadOrWriteFloat(L"Layout_OSD", L"OsdFadeSpeed");
    m_enableOSD = LoadOrWriteInt(L"Layout_OSD", L"EnableOSD") != 0;

    // System
    m_watchdogInterval = std::clamp(LoadOrWriteFloat(L"System", L"WatchdogInterval"), 0.5f, 1000.0f);
    m_watchdogTimeout = std::clamp(LoadOrWriteFloat(L"System", L"WatchdogTimeout"), 2.0f, 1000.0f);
    m_maxThumbnailCache = LoadOrWriteInt(L"System", L"MaxThumbnailCache");
    m_thumbnailSize = LoadOrWriteFloat(L"System", L"ThumbnailSize");
    m_thumbnailJpegQuality = std::clamp(LoadOrWriteFloat(L"System", L"ThumbnailJpegQuality"), 0.0f, 1.0f);

    // GlobalHotkeys
    m_showHotkeys = LoadOrWriteInt(L"GlobalHotkeys", L"ShowHotkeys") != 0;
    m_modNextTrack = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_NextTrack");
    m_vkNextTrack = LoadOrWriteInt(L"GlobalHotkeys", L"VK_NextTrack");
    m_modPrevTrack = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PrevTrack");
    m_vkPrevTrack = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PrevTrack");
    m_modPlayPause = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PlayPause");
    m_vkPlayPause = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PlayPause");
    m_modStop = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_Stop");
    m_vkStop = LoadOrWriteInt(L"GlobalHotkeys", L"VK_Stop");
    m_modVolUp5 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolUp5");
    m_vkVolUp5 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolUp5");
    m_modVolDown5 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolDown5");
    m_vkVolDown5 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolDown5");
    m_modVolUp25 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolUp25");
    m_vkVolUp25 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolUp25");
    m_modVolDown25 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolDown25");
    m_vkVolDown25 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolDown25");
    m_modPrevPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PrevPlaylist");
    m_vkPrevPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PrevPlaylist");
    m_modNextPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_NextPlaylist");
    m_vkNextPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"VK_NextPlaylist");
    m_modActiveTopMost = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ActiveTopMost");
    m_vkActiveTopMost = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ActiveTopMost");
    m_modActiveBottom = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ActiveBottom");
    m_vkActiveBottom = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ActiveBottom");
    m_modExitApp = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ExitApp");
    m_vkExitApp = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ExitApp");
}
