#pragma once

#include <d2d1_1.h>
#include <string>
#include <vector>

// 前方宣言
class ConfigManager;

struct BackgroundLayout {
    D2D1_RECT_F destRect;
    D2D1_RECT_F srcRect;
    D2D1_RECT_F overlayRect;
};

struct VisualizerLayout {
    D2D1_RECT_F drawRect;
};

struct AppLogoLayout {
    D2D1_RECT_F destRect;
    D2D1_POINT_2F shadowOffset;
};

struct TrackInfoLayout {
    D2D1_RECT_F fallbackArtRect;
    D2D1_RECT_F artDestRect;
    D2D1_RECT_F artShadowRect;

    D2D1_RECT_F titleRect;
    D2D1_RECT_F titleShadowRect;

    D2D1_RECT_F artistRect;
    D2D1_RECT_F artistShadowRect;

    D2D1_POINT_2F trackCountOrigin;
    D2D1_POINT_2F trackCountShadowOrigin;
    float trackCountMaxWidth;
    float trackCountMaxHeight;
};


struct SeekBarLayout {
    D2D1_RECT_F bgRect;
    D2D1_RECT_F fgRect;
    float textMaxWidth;
    float textMaxHeight;
    D2D1_POINT_2F textOrigin;
};

struct PlaybackControlsLayout {
    float centerX;
    float centerY;
    float size;
    float spacing;
    float half;
};

struct VolumeControlLayout {
    float volX;
    float volY;
    float volSize;
    float spkW;
    float spkH;
    float spkConeW;
    float spkConeH;

    float shadowX;
    float shadowY;

    float textX;
    float textY;
    float textMaxWidth;
    float textMaxHeight;
};

struct NextTrackLayout {
    D2D1_RECT_F labelRect;
    D2D1_RECT_F labelShadowRect;

    D2D1_RECT_F fallbackArtRect;
    D2D1_RECT_F artDestRect;
    D2D1_RECT_F artShadowRect;

    D2D1_RECT_F titleRect;
    D2D1_RECT_F titleShadowRect;

    D2D1_RECT_F artistRect;
    D2D1_RECT_F artistShadowRect;
};

/**
 * @brief 各UI要素のレイアウト（座標やサイズなど）を計算するためのクラス
 * 
 * 状態を持たず、入力値から論理ピクセルでの座標・矩形情報を算出し返却する。
 */

struct PlaylistItemLayout {
    D2D1_RECT_F hlRect;
    D2D1_RECT_F titleRect;
    D2D1_RECT_F artistRect;
    D2D1_POINT_2F timeOrigin;
    float timeMaxWidth;
    float timeMaxHeight;
};

struct PlaylistLayout {
    float playlistWidth;
    float playlistHeight;
    float playlistX;
    float playlistY;

    D2D1_RECT_F bgRect;
    D2D1_RECT_F clipRect;

    float gripX;
    float gripLineWidth;
    float gripShadowX;
    float gripShadowY;

    float itemHeight;
    float startY;
    float newManualScrollY;
};

struct ResizeGripLayout {
    D2D1_POINT_2F pt1;
    D2D1_POINT_2F pt2;
    D2D1_POINT_2F pt3;
};

class LayoutCalculator {
public:
    static BackgroundLayout CalculateBackgroundLayout(float logicalWidth, float logicalHeight, D2D1_SIZE_F bitmapSize);
    static VisualizerLayout CalculateVisualizerLayout(float logicalWidth, float logicalHeight);
    
    static AppLogoLayout CalculateAppLogoLayout(const ConfigManager* config);
    static TrackInfoLayout CalculateTrackInfoLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, D2D1_SIZE_F bitmapSize);
    static NextTrackLayout CalculateNextTrackLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, D2D1_SIZE_F bitmapSize);

    static SeekBarLayout CalculateSeekBarLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, float progress);
    static PlaybackControlsLayout CalculatePlaybackControlsLayout(float logicalWidth, float logicalHeight, const ConfigManager* config);
    static VolumeControlLayout CalculateVolumeControlLayout(float logicalWidth, float logicalHeight, const ConfigManager* config);

    static PlaylistLayout CalculatePlaylistLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, float slideX, float manualScrollY, size_t currentTrackIndex, size_t totalTracks);
    static PlaylistItemLayout CalculatePlaylistItemLayout(const PlaylistLayout& baseLayout, const ConfigManager* config, float currentY);
    static ResizeGripLayout CalculateResizeGripLayout(float logicalWidth, float logicalHeight);


};

