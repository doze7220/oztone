#pragma once
#include <windows.h>
#include <d2d1.h>
#include <string>
#include <vector>
#include <optional>

#include "PlaylistManager.h"
#include "Window.h"

class ConfigManager;

struct PlaylistSummary {
    std::wstring filepath;
    std::wstring displayName;
    size_t trackCount = 0;
    std::wstring totalTimeString = L"---";
};

/**
 * @brief 描画・更新に必要な「そのフレームで全Widgetに共通する情報」をまとめたコンテキスト
 * 
 * Widget固有の状態・キャッシュ・リソースはWidget自身が保持すること。
 * このコンテキストを巨大なGod Objectにしないこと。
 */
struct WidgetContext {
    float deltaTime;
    bool isHovered;
    bool isLogoMenuHovered;
    int logoMenuHoveredIndex;
    const std::vector<Window::LogoMenuItem>* logoMenuItems;
    bool isControlHovered;
    bool isVolumeHovered;
    bool isPlaylistHovered;
    int playlistToolbarHoveredIndex;
    bool isPlaylistPinnedHovered;
    bool isPlaylistListViewMode;
    bool isPlaying;
    float progress;
    const std::vector<float>* spectrum;
    float volume;
    size_t currentTrackIndex;
    size_t totalTracks;
    const std::vector<TrackMetadata>* shuffleMetadataList;

    float dpiScale;

    float controlAlpha;
    float osdVolumeAlpha;
    float flyTextAlpha;
    std::wstring flyTextString;
    const ConfigManager* config;

    std::wstring timeString;

    std::wstring trackTitle;
    std::wstring trackArtist;
    ID2D1Bitmap* currentArtBitmap;
    
    bool nextIsReady;
    std::wstring nextTrackTitle;
    std::wstring nextTrackArtist;
    ID2D1Bitmap* nextArtBitmap;
    
    std::optional<size_t> focusedPlaylistIndex;
    float logoMenuHoverAlpha = 0.0f;
    int playbackHoveredIndex = -1;
    int playlistHoveredItemIndex = -1;

    const std::vector<PlaylistSummary>* availablePlaylistsCache;
};

// ヘックスカラー（"#RRGGBB" または "#AARRGGBB"）をD2D1_COLOR_Fに変換するユーティリティ
inline D2D1_COLOR_F ParseHexColor(const std::wstring& hexColor) {
    if (hexColor.empty() || hexColor[0] != L'#') {
        return D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
    }

    std::wstring hexStr = hexColor.substr(1);
    try {
        uint32_t val = std::stoul(hexStr, nullptr, 16);
        if (hexStr.length() == 6) {
            float r = ((val >> 16) & 0xFF) / 255.0f;
            float g = ((val >> 8) & 0xFF) / 255.0f;
            float b = (val & 0xFF) / 255.0f;
            return D2D1::ColorF(r, g, b, 1.0f);
        } else if (hexStr.length() == 8) {
            float a = ((val >> 24) & 0xFF) / 255.0f;
            float r = ((val >> 16) & 0xFF) / 255.0f;
            float g = ((val >> 8) & 0xFF) / 255.0f;
            float b = (val & 0xFF) / 255.0f;
            return D2D1::ColorF(r, g, b, a);
        }
    } catch (...) {
        // Fallback
    }
    return D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
}
