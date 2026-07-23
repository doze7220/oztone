#include "Config_LayoutTrackInfo.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutTrackInfo(Config_LayoutTrackInfo& outConfig) {
    outConfig.BaseX = LoadOrWriteInt(L"Layout_TrackInfo", L"BaseX");
    outConfig.BaseBottomOffset = LoadOrWriteInt(L"Layout_TrackInfo", L"BaseBottomOffset");
    outConfig.ArtOffsetX = LoadOrWriteInt(L"Layout_TrackInfo", L"ArtOffsetX");
    outConfig.ArtOffsetY = LoadOrWriteInt(L"Layout_TrackInfo", L"ArtOffsetY");
    outConfig.ArtSize = LoadOrWriteInt(L"Layout_TrackInfo", L"ArtSize");
    outConfig.FallbackArtOpacity = LoadOrWriteFloat(L"Layout_TrackInfo", L"FallbackArtOpacity");
    outConfig.TitleOffsetX = LoadOrWriteInt(L"Layout_TrackInfo", L"TitleOffsetX");
    outConfig.TitleOffsetY = LoadOrWriteInt(L"Layout_TrackInfo", L"TitleOffsetY");
    outConfig.TitleFontSize = LoadOrWriteFloat(L"Layout_TrackInfo", L"TitleFontSize");
    outConfig.ArtistOffsetX = LoadOrWriteInt(L"Layout_TrackInfo", L"ArtistOffsetX");
    outConfig.ArtistOffsetY = LoadOrWriteInt(L"Layout_TrackInfo", L"ArtistOffsetY");
    outConfig.ArtistFontSize = LoadOrWriteFloat(L"Layout_TrackInfo", L"ArtistFontSize");
    outConfig.TooltipOffsetX = LoadOrWriteFloat(L"Layout_TrackInfo", L"TooltipOffsetX");
    outConfig.TooltipOffsetY = LoadOrWriteFloat(L"Layout_TrackInfo", L"TooltipOffsetY");
}
