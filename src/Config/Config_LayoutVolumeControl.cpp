#include "Config_LayoutVolumeControl.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutVolumeControl(Config_LayoutVolumeControl& outConfig) {
    outConfig.BaseLeftOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseLeftOffset");
    outConfig.BaseBottomOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseBottomOffset");
    outConfig.IconSize = LoadOrWriteInt(L"Layout_VolumeControl", L"IconSize");
    outConfig.TextOffsetX = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetX");
    outConfig.TextOffsetY = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetY");
    outConfig.FontSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"FontSize");
    outConfig.TextLetterSpacing = LoadOrWriteFloat(L"Layout_VolumeControl", L"TextLetterSpacing");
    outConfig.TooltipOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipOffsetY");
}
