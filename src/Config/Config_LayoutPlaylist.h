#pragma once
#include <string>

struct Config_LayoutPlaylist {
    int PlaylistPosition = 1;
    bool IsPlaylistPinned = false;
    int PlaylistHoverWidth = 80;
    int PlaylistWidth = 250;
    int PlaylistItemOffsetY = 38;

    float PlaylistTitleFontSize = 15.0f;
    int PlaylistTitleOffsetX = 2;
    int PlaylistTitleOffsetY = 2;

    float PlaylistArtistFontSize = 12.0f;
    std::wstring PlaylistArtistColor = L"#888888";
    int PlaylistArtistOffsetX = 2;
    int PlaylistArtistOffsetY = 20;

    float PlaylistTimeFontSize = 12.0f;
    std::wstring PlaylistTimeColor = L"#888888";
    int PlaylistTimeOffsetX = 10;
    int PlaylistTimeOffsetY = 20;
    float PlaylistTimeLetterSpacing = -1.0f;

    float PlaylistThumbSize = 37.0f;
    float PlaylistThumbOffsetX = 14.0f;
    float PlaylistThumbOffsetY = 1.0f;

    float PlaylistBgOpacity = 0.8f;
    float PlaylistGripOffset = 5.0f;
    float PlaylistGripLineWidth = 1.0f;
    std::wstring PlaylistGripLineColor = L"#AAAAAA";
    float PlaylistGripArrowHeight = 35.0f;
    float PlaylistGripArrowWidth = 15.0f;
    std::wstring PlaylistGripArrowColor = L"#AAAAAA";

    float ToolbarHeight = 52.0f;
    float ToolbarIconSize = 18.0f;
    float ToolbarIconSpacing = 10.0f;
    float ToolbarTextOffsetY = 35.0f;
    float ToolbarTextFontSize = 12.0f;

    int PinSubIconOffsetX = 6;
    int PinSubIconOffsetY = 6;
    float PinSubIconFontSize = 10.0f;

    float TrackCountOffsetX = 1.0f;
    float TrackCountBoxWidth = 10.0f;
    float TrackCountUnderLineX = 11.0f;
    float TrackCountUnderLineWidth = 1.0f;
    float TrackCountFontSize = 10.0f;
    float TrackCountLetterSpacing = -1.0f;
    std::wstring TrackCountBoxFontColor = L"#000000";
    std::wstring TrackCountBoxBaseColor = L"#FFFFFF";
    float TrackCountBoxBaseOpacity = 0.6f;

    float PlaylistLeaveDelay = 0.5f;
};
