#include "ConfigManager.h"

void ConfigManager::LoadLogoMenuSettings() {
    m_showAppLogo = LoadOrWriteInt(L"Visibility", L"ShowAppLogo", 1) != 0;

    m_logoX = LoadOrWriteInt(L"Layout_AppLogo", L"X", 16);
    m_logoY = LoadOrWriteInt(L"Layout_AppLogo", L"Y", 16);
    m_logoWidth = LoadOrWriteInt(L"Layout_AppLogo", L"Width", 64);
    m_logoHeight = LoadOrWriteInt(L"Layout_AppLogo", L"Height", 64);

    m_logoMenuTypingFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuTypingFontSize", 14.0f);
    m_logoMenuTypingFontFamily = LoadOrWriteString(L"Layout_LogoMenu", L"MenuTypingFontFamily", L"Meiryo");
    m_logoMenuTextColor = LoadOrWriteString(L"Layout_LogoMenu", L"MenuTextColor", L"#FFFFFF");
    m_logoMenuTextOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuTextOffsetX", 8);
    m_logoMenuTextOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuTextOffsetY", -18);
    m_logoMenuTypingLetterSpacing = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuTypingLetterSpacing", -1.0f);

    m_logoMenuIconSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuIconSize", 44.0f);
    m_logoMenuIconSpacing = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconSpacing", 54);
    m_logoMenuIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconOffsetX", -24);
    m_logoMenuIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconOffsetY", 38);

    m_logoMenuScrollDuration = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuScrollDuration", 0.5f);
    m_logoMenuFontFamily = LoadOrWriteString(L"Layout_LogoMenu", L"MenuFontFamily", L"Segoe UI Emoji");

    m_logoMenuVisualizerFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"VisualizerIconFontSize", 24.0f);
    m_logoMenuVisualizerIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"VisualizerIconOffsetX", 6);
    m_logoMenuVisualizerIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"VisualizerIconOffsetY", 0);

    m_logoMenuLockIconFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"LockIconFontSize", 20.0f);
    m_logoMenuLockIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"LockIconOffsetX", -2);
    m_logoMenuLockIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"LockIconOffsetY", -2);

    m_logoMenuDescShadowOffsetX = LoadOrWriteFloat(L"Layout_LogoMenu", L"DescShadowOffsetX", 2.0f);
    m_logoMenuDescShadowOffsetY = LoadOrWriteFloat(L"Layout_LogoMenu", L"DescShadowOffsetY", 1.0f);
    m_logoMenuDescShadowOpacity = LoadOrWriteFloat(L"Layout_LogoMenu", L"DescShadowOpacity", 0.8f);

    m_menuLeaveDelay = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuLeaveDelay", 2.0f);
}
