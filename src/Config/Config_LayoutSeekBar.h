#pragma once
#include <string>

struct Config_LayoutSeekBar {
    float SeekBarMargin = 20.0f;
    int Height = 8;
    int BottomOffset = 25;
    std::wstring SeekBarFgColor = L"#FFFFFF";
    float SeekBarFgOpacity = 0.4f;
    std::wstring SeekBarBgColor = L"#000000";
    float SeekBarBgOpacity = 0.3f;
    float TimeFontSize = 20.0f;
    float TimeLetterSpacing = -3.0f;
    float TimeMarginRight = 15.0f;
    std::wstring TextColor = L"#FFFFFF";
    float TextOpacity = 1.0f;
};
