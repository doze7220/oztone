#include "Config_LayoutNowPlaying.h"
#include "ConfigManager.h"

void ConfigManager::LoadSection_LayoutNowPlaying(Config_LayoutNowPlaying& outConfig) {
    outConfig.BaseX = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseX");
    outConfig.BaseBottomOffset = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseBottomOffset");
    outConfig.ArtOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetX");
    outConfig.ArtOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetY");
    outConfig.ArtSize = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtSize");
    outConfig.FallbackArtOpacity = LoadOrWriteFloat(L"Layout_NowPlaying", L"FallbackArtOpacity");
    outConfig.TitleOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetX");
    outConfig.TitleOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetY");
    outConfig.TitleFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"TitleFontSize");
    outConfig.ArtistOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetX");
    outConfig.ArtistOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetY");
    outConfig.ArtistFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"ArtistFontSize");
    outConfig.TooltipOffsetX = LoadOrWriteFloat(L"Layout_NowPlaying", L"TooltipOffsetX");
    outConfig.TooltipOffsetY = LoadOrWriteFloat(L"Layout_NowPlaying", L"TooltipOffsetY");
}
