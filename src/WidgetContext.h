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
    bool isPlaylistHovered;
    int playlistToolbarHoveredIndex;
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

    const std::vector<PlaylistSummary>* availablePlaylistsCache;
};
