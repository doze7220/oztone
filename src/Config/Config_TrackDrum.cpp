#include "Config_TrackDrum.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_TrackDrum(Config_TrackDrum& outConfig) {
    outConfig.EnableTrackDrum = LoadOrWriteInt(L"TrackDrum", L"EnableTrackDrum") != 0;
    outConfig.MaxDuration = LoadOrWriteFloat(L"TrackDrum", L"MaxDuration");
    outConfig.MinSpeed = LoadOrWriteFloat(L"TrackDrum", L"MinSpeed");
    outConfig.MaxSpeed = LoadOrWriteFloat(L"TrackDrum", L"MaxSpeed");
    outConfig.Acceleration = LoadOrWriteFloat(L"TrackDrum", L"Acceleration");
    outConfig.Deceleration = LoadOrWriteFloat(L"TrackDrum", L"Deceleration");
    outConfig.JogDialConfirmDelay = LoadOrWriteFloat(L"TrackDrum", L"JogDialConfirmDelay");
}
