#include "Config_LayoutPlaybackControls.h"
#include "../ConfigManager.h"

void ConfigManager::LoadSection_LayoutPlaybackControls(Config_LayoutPlaybackControls& outConfig) {
    outConfig.ControlHoverHeight = LoadOrWriteFloat(L"Layout_PlaybackControls", L"ControlHoverHeight");
    outConfig.BaseBottomOffset = LoadOrWriteInt(L"Layout_PlaybackControls", L"BaseBottomOffset");
    outConfig.CenterOffsetX = LoadOrWriteInt(L"Layout_PlaybackControls", L"CenterOffsetX");
    outConfig.ButtonSpacing = LoadOrWriteInt(L"Layout_PlaybackControls", L"ButtonSpacing");
    outConfig.ButtonSize = LoadOrWriteInt(L"Layout_PlaybackControls", L"ButtonSize");
    outConfig.SkipIconPoints = LoadOrWriteString(L"Layout_PlaybackControls", L"SkipIconPoints");
    outConfig.SkipTextFontSize = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextFontSize");
    outConfig.SkipTextOffsetX = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetX");
    outConfig.SkipTextOffsetY = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetY");
    outConfig.SkipTextShadowShift = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextShadowShift");
}
