#include "Config_LayoutTooltip.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutTooltip(Config_LayoutTooltip& outConfig) {
    outConfig.TooltipIconSize = LoadOrWriteFloat(L"Layout_Tooltip", L"TooltipIconSize");
    outConfig.TooltipBgColor = LoadOrWriteString(L"Layout_Tooltip", L"TooltipBgColor");
    outConfig.TooltipBgOpacity = LoadOrWriteFloat(L"Layout_Tooltip", L"TooltipBgOpacity");
    outConfig.TooltipWidth = LoadOrWriteFloat(L"Layout_Tooltip", L"TooltipWidth");
    outConfig.TooltipHeight = LoadOrWriteFloat(L"Layout_Tooltip", L"TooltipHeight");
}
