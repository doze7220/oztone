#include "Config_Visibility.h"
#include "../ConfigManager.h"
#include "../ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Visibility(Config_Visibility& outConfig) {
    outConfig.ShowAppLogo = LoadOrWriteInt(L"Visibility", L"ShowAppLogo") != 0;
    outConfig.ShowNowPlaying = LoadOrWriteInt(L"Visibility", L"ShowNowPlaying") != 0;
    outConfig.ShowSeekBar = LoadOrWriteInt(L"Visibility", L"ShowSeekBar") != 0;
    outConfig.ShowPlaybackControls = LoadOrWriteInt(L"Visibility", L"ShowPlaybackControls") != 0;
    outConfig.ShowVolumeControl = LoadOrWriteInt(L"Visibility", L"ShowVolumeControl") != 0;
}
