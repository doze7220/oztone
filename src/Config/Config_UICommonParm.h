#pragma once
#include <string>

struct Config_UICommonParm {
    std::wstring FocusColor;
    float HoverFadeOutSpeed = 2.0f;
    float BaseLeaveDelay = 3.0f;
    std::wstring BaseFontFamily;
    std::wstring MonoFontFamily;
    std::wstring IconFontFamily;
    std::wstring OsdFontFamily;
    bool EnableShadow = true;
    std::wstring ShadowColor;
    float ShadowOffsetX = 2.0f;
    float ShadowOffsetY = 2.0f;
    float ShadowOpacity = 0.8f;
};
