#include "Config_LayoutPlaylist.h"
#include "../ConfigManager.h"

void ConfigManager::LoadSection_LayoutPlaylist(Config_LayoutPlaylist& outConfig) {
    outConfig.PlaylistPosition = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistPosition");
    outConfig.IsPlaylistPinned = LoadOrWriteInt(L"Layout_Playlist", L"IsPlaylistPinned") != 0;
    outConfig.PlaylistHoverWidth = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistHoverWidth");
    outConfig.PlaylistWidth = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistWidth");
    outConfig.PlaylistItemOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistItemOffsetY");

    outConfig.PlaylistTitleFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTitleFontSize");
    outConfig.PlaylistTitleOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTitleOffsetX");
    outConfig.PlaylistTitleOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTitleOffsetY");

    outConfig.PlaylistArtistFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistArtistFontSize");
    outConfig.PlaylistArtistColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistArtistColor");
    outConfig.PlaylistArtistOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistArtistOffsetX");
    outConfig.PlaylistArtistOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistArtistOffsetY");

    outConfig.PlaylistTimeFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTimeFontSize");
    outConfig.PlaylistTimeColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistTimeColor");
    outConfig.PlaylistTimeOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTimeOffsetX");
    outConfig.PlaylistTimeOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTimeOffsetY");
    outConfig.PlaylistTimeLetterSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTimeLetterSpacing");

    outConfig.PlaylistThumbSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistThumbSize");
    outConfig.PlaylistThumbOffsetX = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistThumbOffsetX");
    outConfig.PlaylistThumbOffsetY = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistThumbOffsetY");

    outConfig.PlaylistBgOpacity = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistBgOpacity");
    outConfig.PlaylistGripOffset = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripOffset");
    outConfig.PlaylistGripLineWidth = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripLineWidth");
    outConfig.PlaylistGripLineColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistGripLineColor");
    outConfig.PlaylistGripArrowHeight = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripArrowHeight");
    outConfig.PlaylistGripArrowWidth = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripArrowWidth");
    outConfig.PlaylistGripArrowColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistGripArrowColor");

    outConfig.ToolbarHeight = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarHeight");
    outConfig.ToolbarIconSize = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarIconSize");
    outConfig.ToolbarIconSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarIconSpacing");
    outConfig.ToolbarTextOffsetY = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarTextOffsetY");
    outConfig.ToolbarTextFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarTextFontSize");

    outConfig.PinSubIconOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PinSubIconOffsetX");
    outConfig.PinSubIconOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PinSubIconOffsetY");
    outConfig.PinSubIconFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PinSubIconFontSize");

    outConfig.TrackCountOffsetX = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountOffsetX");
    outConfig.TrackCountBoxWidth = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountBoxWidth");
    outConfig.TrackCountUnderLineX = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountUnderLineX");
    outConfig.TrackCountUnderLineWidth = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountUnderLineWidth");
    outConfig.TrackCountFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountFontSize");
    outConfig.TrackCountLetterSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountLetterSpacing");
    outConfig.TrackCountBoxFontColor = LoadOrWriteString(L"Layout_Playlist", L"TrackCountBoxFontColor");
    outConfig.TrackCountBoxBaseColor = LoadOrWriteString(L"Layout_Playlist", L"TrackCountBoxBaseColor");
    outConfig.TrackCountBoxBaseOpacity = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountBoxBaseOpacity");

    outConfig.PlaylistLeaveDelay = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistLeaveDelay");
}
