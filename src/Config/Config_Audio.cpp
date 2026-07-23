#include "Config_Audio.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Audio(Config_Audio& outConfig) {
    outConfig.DefaultVolume = LoadOrWriteFloat(L"Audio", L"DefaultVolume");
    outConfig.ShuffleMode = LoadOrWriteInt(L"Audio", L"ShuffleMode") != 0;
    outConfig.SkipSeconds = LoadOrWriteFloat(L"Audio", L"SkipSeconds");
}
