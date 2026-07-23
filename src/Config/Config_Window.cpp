#include "Config_Window.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Window(Config_Window& outConfig) {
    outConfig.ShowTitleBar = LoadOrWriteInt(L"Window", L"ShowTitleBar") != 0;
    outConfig.ShowWindowFrame = LoadOrWriteInt(L"Window", L"ShowWindowFrame") != 0;
    outConfig.ShowTaskbar = LoadOrWriteInt(L"Window", L"ShowTaskbar") != 0;
    outConfig.ZOrder = LoadOrWriteInt(L"Window", L"ZOrder");
    outConfig.SavePositionOnExit = LoadOrWriteInt(L"Window", L"SavePositionOnExit") != 0;
    outConfig.EnableResize = LoadOrWriteInt(L"Window", L"EnableResize") != 0;
    outConfig.LockWindowPosition = LoadOrWriteInt(L"Window", L"LockWindowPosition") != 0;
    outConfig.WindowX = LoadOrWriteInt(L"Window", L"WindowX");
    outConfig.WindowY = LoadOrWriteInt(L"Window", L"WindowY");
    outConfig.WindowWidth = LoadOrWriteInt(L"Window", L"WindowWidth");
    outConfig.WindowHeight = LoadOrWriteInt(L"Window", L"WindowHeight");
}
