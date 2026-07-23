#include "Config_Background.h"
#include "../ConfigManager.h"
#include "../ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Background(Config_Background& outConfig) {
    outConfig.BgOpacity = LoadOrWriteFloat(L"Background", L"BgOpacity");
    outConfig.BgDarkenOpacity = LoadOrWriteFloat(L"Background", L"BgDarkenOpacity");
    outConfig.BackgroundArtMode = LoadOrWriteInt(L"Background", L"BackgroundArtMode");
    outConfig.CrossfadeDuration = LoadOrWriteFloat(L"Background", L"CrossfadeDuration");
}
