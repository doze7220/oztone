#pragma once
#include <string>

struct Config_LayoutOSD {
    float OsdFontSize = 48.0f;
    std::wstring OsdTextColor = L"#FFFFFF";
    float OsdFadeWait = 1.5f;
    float OsdFadeSpeed = 1.5f;
    bool EnableOSD = true;
};
