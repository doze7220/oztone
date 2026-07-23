#include "Config_LayoutAppLogo.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutAppLogo(Config_LayoutAppLogo& outConfig) {
    outConfig.X = LoadOrWriteInt(L"Layout_AppLogo", L"X");
    outConfig.Y = LoadOrWriteInt(L"Layout_AppLogo", L"Y");
    outConfig.Width = LoadOrWriteInt(L"Layout_AppLogo", L"Width");
    outConfig.Height = LoadOrWriteInt(L"Layout_AppLogo", L"Height");
    outConfig.LogoIdleOpacity = LoadOrWriteFloat(L"Layout_AppLogo", L"LogoIdleOpacity");
    outConfig.LogoFadeSpeed = LoadOrWriteFloat(L"Layout_AppLogo", L"LogoFadeSpeed");
}
