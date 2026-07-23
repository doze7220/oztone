#include "Config_GlobalHotkeys.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"
#include <windows.h>
#include <string>

void ConfigManager::LoadSection_GlobalHotkeys(Config_GlobalHotkeys& outConfig) {
    outConfig.ShowHotkeys = (LoadOrWriteInt(L"GlobalHotkeys", L"ShowHotkeys") != 0);

    outConfig.ModifierNextTrack = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_NextTrack");
    outConfig.VKNextTrack = LoadOrWriteInt(L"GlobalHotkeys", L"VK_NextTrack");
    outConfig.ModifierPrevTrack = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PrevTrack");
    outConfig.VKPrevTrack = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PrevTrack");
    outConfig.ModifierPlayPause = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PlayPause");
    outConfig.VKPlayPause = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PlayPause");
    outConfig.ModifierStop = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_Stop");
    outConfig.VKStop = LoadOrWriteInt(L"GlobalHotkeys", L"VK_Stop");
    
    outConfig.ModifierVolUp5 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolUp5");
    outConfig.VKVolUp5 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolUp5");
    outConfig.ModifierVolDown5 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolDown5");
    outConfig.VKVolDown5 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolDown5");
    outConfig.ModifierVolUp25 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolUp25");
    outConfig.VKVolUp25 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolUp25");
    outConfig.ModifierVolDown25 = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_VolDown25");
    outConfig.VKVolDown25 = LoadOrWriteInt(L"GlobalHotkeys", L"VK_VolDown25");
    
    outConfig.ModifierPrevPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_PrevPlaylist");
    outConfig.VKPrevPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"VK_PrevPlaylist");
    outConfig.ModifierNextPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_NextPlaylist");
    outConfig.VKNextPlaylist = LoadOrWriteInt(L"GlobalHotkeys", L"VK_NextPlaylist");
    
    outConfig.ModifierActiveTopMost = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ActiveTopMost");
    outConfig.VKActiveTopMost = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ActiveTopMost");
    outConfig.ModifierActiveBottom = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ActiveBottom");
    outConfig.VKActiveBottom = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ActiveBottom");
    
    outConfig.ModifierExitApp = LoadOrWriteInt(L"GlobalHotkeys", L"Modifier_ExitApp");
    outConfig.VKExitApp = LoadOrWriteInt(L"GlobalHotkeys", L"VK_ExitApp");
}
