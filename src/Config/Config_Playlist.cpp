#include "Config_Playlist.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Playlist(Config_Playlist& outConfig) {
    outConfig.DefaultPlaylistPath = LoadOrWriteString(L"Playlist", L"DefaultPlaylistPath");
}
