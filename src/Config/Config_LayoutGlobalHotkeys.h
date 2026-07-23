#pragma once
#include <string>

struct Config_LayoutGlobalHotkeys {
    float FontSize = 15.0f;
    float LineSpacing = 20.0f;
    std::wstring CoreColor = L"#FFFFFF";
    std::wstring GlowColor = L"#00FFFF";
    float GlowOpacity = 0.10f;
    float KeyColumnOffset = 360.0f;
    float ActionColumnOffset = 140.0f;
};
