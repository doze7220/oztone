#pragma once
#include <string>

struct Config_LayoutSeekBar {
    float SeekBarMargin = 20.0f;
    int Height = 8;
    int BottomOffset = 25;
    std::wstring FgColor = L"#FFFFFF";
    float FgOpacity = 0.4f;
    std::wstring BgColor = L"#000000";
    float BgOpacity = 0.3f;
    float TimeFontSize = 20.0f;
    float TimeLetterSpacing = -3.0f;
    float TimeMarginRight = 15.0f;
    std::wstring TextColor = L"#FFFFFF";
    float TextOpacity = 1.0f;
};
