#include "Config_LayoutGlobalHotkeys.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutGlobalHotkeys(Config_LayoutGlobalHotkeys& outConfig) {
    outConfig.FontSize = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"FontSize");
    outConfig.LineSpacing = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"LineSpacing");
    outConfig.CoreColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"CoreColor");
    outConfig.GlowColor = LoadOrWriteString(L"Layout_GlobalHotkeys", L"GlowColor");
    outConfig.GlowOpacity = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"GlowOpacity");
    outConfig.KeyColumnOffset = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"KeyColumnOffset");
    outConfig.ActionColumnOffset = LoadOrWriteFloat(L"Layout_GlobalHotkeys", L"ActionColumnOffset");
}
