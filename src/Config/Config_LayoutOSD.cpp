#include "Config_LayoutOSD.h"
#include "../ConfigManager.h"
#include "../ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_LayoutOSD(Config_LayoutOSD& outConfig) {
    outConfig.OsdFontSize = LoadOrWriteFloat(L"Layout_OSD", L"OsdFontSize");
    outConfig.OsdTextColor = LoadOrWriteString(L"Layout_OSD", L"OsdTextColor");
    outConfig.OsdFadeWait = LoadOrWriteFloat(L"Layout_OSD", L"OsdFadeWait");
    outConfig.OsdFadeSpeed = LoadOrWriteFloat(L"Layout_OSD", L"OsdFadeSpeed");
    outConfig.EnableOSD = LoadOrWriteInt(L"Layout_OSD", L"EnableOSD") != 0;
}
