#include "Config_System.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_System(Config_System& outConfig) {
    outConfig.WatchdogInterval = LoadOrWriteFloat(L"System", L"WatchdogInterval");
    outConfig.WatchdogTimeout = LoadOrWriteFloat(L"System", L"WatchdogTimeout");
    outConfig.MaxThumbnailCache = LoadOrWriteInt(L"System", L"MaxThumbnailCache");
    outConfig.ThumbnailSize = LoadOrWriteFloat(L"System", L"ThumbnailSize");
    outConfig.ThumbnailJpegQuality = LoadOrWriteFloat(L"System", L"ThumbnailJpegQuality");
}
