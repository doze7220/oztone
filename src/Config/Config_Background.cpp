#include "Config_Background.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Background(Config_Background& outConfig) {
    outConfig.BackgroundOpacity = LoadOrWriteFloat(L"Background", L"BackgroundOpacity");
    outConfig.BackgroundDarkenOpacity = LoadOrWriteFloat(L"Background", L"BackgroundDarkenOpacity");
    outConfig.BackgroundArtMode = LoadOrWriteInt(L"Background", L"BackgroundArtMode");
    outConfig.CrossfadeDuration = LoadOrWriteFloat(L"Background", L"CrossfadeDuration");
}
