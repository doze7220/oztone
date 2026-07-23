#include "Config_LayoutSeekBar.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutSeekBar(Config_LayoutSeekBar& outConfig) {
    outConfig.SeekBarMargin = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarMargin");
    outConfig.Height = LoadOrWriteInt(L"Layout_SeekBar", L"Height");
    outConfig.BottomOffset = LoadOrWriteInt(L"Layout_SeekBar", L"BottomOffset");
    outConfig.SeekBarFgColor = LoadOrWriteString(L"Layout_SeekBar", L"SeekBarFgColor");
    outConfig.SeekBarFgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarFgOpacity");
    outConfig.SeekBarBgColor = LoadOrWriteString(L"Layout_SeekBar", L"SeekBarBgColor");
    outConfig.SeekBarBgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarBgOpacity");
    outConfig.TimeFontSize = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeFontSize");
    outConfig.TimeLetterSpacing = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeLetterSpacing");
    outConfig.TimeMarginRight = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeMarginRight");
    outConfig.TextColor = LoadOrWriteString(L"Layout_SeekBar", L"TextColor");
    outConfig.TextOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"TextOpacity");
}
