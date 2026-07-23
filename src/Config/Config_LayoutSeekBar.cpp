#include "Config_LayoutSeekBar.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutSeekBar(Config_LayoutSeekBar& outConfig) {
    outConfig.SeekBarMargin = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarMargin");
    outConfig.Height = LoadOrWriteInt(L"Layout_SeekBar", L"Height");
    outConfig.BottomOffset = LoadOrWriteInt(L"Layout_SeekBar", L"BottomOffset");
    outConfig.FgColor = LoadOrWriteString(L"Layout_SeekBar", L"FgColor");
    outConfig.FgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"FgOpacity");
    outConfig.BgColor = LoadOrWriteString(L"Layout_SeekBar", L"BgColor");
    outConfig.BgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"BgOpacity");
    outConfig.TimeFontSize = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeFontSize");
    outConfig.TimeLetterSpacing = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeLetterSpacing");
    outConfig.TimeMarginRight = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeMarginRight");
    outConfig.TextColor = LoadOrWriteString(L"Layout_SeekBar", L"TextColor");
    outConfig.TextOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"TextOpacity");
}
