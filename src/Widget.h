#pragma once
#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_1.h>
#include <wincodec.h>
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

/**
 * @brief 各UI要素（Widget）の基底インターフェース
 */
class IWidget {
public:
    virtual ~IWidget() = default;

    /**
     * @brief デバイス依存リソースの作成を行う
     */
    virtual void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) = 0;

    /**
     * @brief デバイス依存リソースを解放する
     */
    virtual void ReleaseResources() = 0;

    /**
     * @brief アニメーションや状態の更新を行う（毎フレーム呼ばれる）
     */
    virtual void UpdateAnimation(const WidgetContext& ctx) = 0;

    /**
     * @brief レイアウトの更新を行う
     */
    virtual void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) = 0;
    
    // 描画
    virtual void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) = 0;

    // スクロール操作の受け取り（デフォルトは何もしない）
    virtual void AddScroll(float delta) {}
    virtual float GetScrollY() const { return 0.0f; }
};
