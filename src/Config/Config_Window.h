#pragma once

struct Config_Window {
    bool ShowTitleBar = false;
    bool ShowWindowFrame = false;
    bool ShowTaskbar = false;
    int ZOrder = 0;
    bool SavePositionOnExit = true;
    bool EnableResize = false;
    bool LockWindowPosition = false;
    int WindowX = -2147483648;
    int WindowY = -2147483648;
    int WindowWidth = 1024;
    int WindowHeight = 512;
};
