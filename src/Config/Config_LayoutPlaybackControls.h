#pragma once
#include <string>

struct Config_LayoutPlaybackControls {
    float ControlHoverHeight = 50.0f;
    int BaseBottomOffset = 22;
    int CenterOffsetX = 0;
    int ButtonSpacing = 55;
    int ButtonSize = 30;
    std::wstring SkipIconPoints = L"0.1,-0.5,0.65,-0.5,-0.15,0.0,0.65,0.5,0.1,0.5,-0.65,0.0";
    float SkipTextFontSize = 15.0f;
    float SkipTextOffsetX = 0.04f;
    float SkipTextOffsetY = -0.05f;
    float SkipTextShadowShift = 1.00f;
};
