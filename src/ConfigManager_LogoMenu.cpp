#include "ConfigManager.h"

void ConfigManager::LoadLogoMenuSettings() {
    m_showAppLogo = LoadOrWriteInt(L"Visibility", L"ShowAppLogo") != 0;

    m_logoX = LoadOrWriteInt(L"Layout_AppLogo", L"X");
    m_logoY = LoadOrWriteInt(L"Layout_AppLogo", L"Y");
    m_logoWidth = LoadOrWriteInt(L"Layout_AppLogo", L"Width");
    m_logoHeight = LoadOrWriteInt(L"Layout_AppLogo", L"Height");

    m_logoMenuTypingFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuTypingFontSize");
    m_logoMenuTypingFontFamily = LoadOrWriteString(L"Layout_LogoMenu", L"MenuTypingFontFamily");
    m_logoMenuTextColor = LoadOrWriteString(L"Layout_LogoMenu", L"MenuTextColor");
    m_logoMenuTextOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuTextOffsetX");
    m_logoMenuTextOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuTextOffsetY");
    m_logoMenuTypingLetterSpacing = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuTypingLetterSpacing");

    m_logoMenuIconSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuIconSize");
    m_logoMenuIconSpacing = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconSpacing");
    m_logoMenuIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconOffsetX");
    m_logoMenuIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"MenuIconOffsetY");

    m_logoMenuScrollDuration = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuScrollDuration");
    m_logoMenuFontFamily = LoadOrWriteString(L"Layout_LogoMenu", L"MenuFontFamily");

    m_logoMenuVisualizerFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"VisualizerIconFontSize");
    m_logoMenuVisualizerIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"VisualizerIconOffsetX");
    m_logoMenuVisualizerIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"VisualizerIconOffsetY");

    m_logoMenuLockIconFontSize = LoadOrWriteFloat(L"Layout_LogoMenu", L"LockIconFontSize");
    m_logoMenuLockIconOffsetX = LoadOrWriteInt(L"Layout_LogoMenu", L"LockIconOffsetX");
    m_logoMenuLockIconOffsetY = LoadOrWriteInt(L"Layout_LogoMenu", L"LockIconOffsetY");

    m_logoMenuDescShadowOffsetX = LoadOrWriteFloat(L"Layout_LogoMenu", L"DescShadowOffsetX");
    m_logoMenuDescShadowOffsetY = LoadOrWriteFloat(L"Layout_LogoMenu", L"DescShadowOffsetY");
    m_logoMenuDescShadowOpacity = LoadOrWriteFloat(L"Layout_LogoMenu", L"DescShadowOpacity");

    m_menuLeaveDelay = LoadOrWriteFloat(L"Layout_LogoMenu", L"MenuLeaveDelay");
}
