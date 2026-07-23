#include "Config_LayoutLogoMenu.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutLogoMenu(Config_LayoutLogoMenu& outConfig) {
    outConfig.MenuTypingFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuTypingFontSize");
    outConfig.MenuTextColor = LoadOrWriteString(L"Layout_LogoMenu", L"MenuTextColor");
    outConfig.MenuTextOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuTextOffsetX");
    outConfig.MenuTextOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuTextOffsetY");
    outConfig.MenuTypingLetterSpacing = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuTypingLetterSpacing");
    outConfig.MenuIconSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuIconSize");
    outConfig.MenuIconSpacing = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconSpacing");
    outConfig.MenuIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconOffsetX");
    outConfig.MenuIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconOffsetY");
    outConfig.MenuScrollDuration = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuScrollDuration");
    outConfig.VisualizerIconFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"VisualizerIconFontSize");
    outConfig.VisualizerIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"VisualizerIconOffsetX");
    outConfig.VisualizerIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"VisualizerIconOffsetY");
    outConfig.LockIconFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"LockIconFontSize");
    outConfig.LockIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"LockIconOffsetX");
    outConfig.LockIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"LockIconOffsetY");
}
