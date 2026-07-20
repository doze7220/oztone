#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <dwrite.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <array>
#include <map>
#include <functional>
#include "PlaylistManager.h"
#include "TrackDatabase.h"
#include "Visualizer.h"
#include "Widget.h"
#include "Window.h"
#include "Renderer_TrackDrum.h"

class ConfigManager;
class ThumbnailManager;
class BackgroundManager;

/**
 * @brief Direct3D 11 と Direct2D を用いたハイブリッド描画エンジン
 * 
 * 将来的な3Dビジュアライザと2D UI(アルバムアート等)を共存させるため、
 * D3D11のSwapChainからDXGIサーフェスを取り出し、D2D1のレンダーターゲットを作成する。
 * ウィンドウ透過にはDirectCompositionを使用し、アルファブレンドされたスワップチェインを合成する。
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * @brief DirectX関連のデバイスやスワップチェインを初期化する
     * @param hwnd ウィンドウハンドル
     * @param config 設定マネージャの参照
     * @return 成功ならtrue
     */
    bool Initialize(HWND hwnd, const ConfigManager& config);

    /**
     * @brief アニメーション状態の更新を行う（UIフェードやスライド等の変数を更新する）
     */
    void UpdateAnimation(float deltaTime, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode, int playbackHoveredIndex, int playlistHoveredItemIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isLogoClicked, int clickedLogoMenuIndex, int clickedPlaybackIndex, bool* outIsPlaylistExpanded = nullptr, bool* outIsLogoMenuExpanded = nullptr);

    /**
     * @brief 描画処理（毎フレーム呼ばれる）
     */
    void Render(bool isHovered, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isPlaylistListViewMode, bool isPlaying, float progress, const std::vector<float>& spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>& shuffleMetadataList, int playlistToolbarHoveredIndex, const std::vector<PlaylistSummary>* availablePlaylistsCache = nullptr);


    /**
     * @brief 変動テキストレイアウトの更新
     */
    void UpdateTextLayouts(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>& shuffleMetadataList);

    /**
     * @brief ウィンドウサイズ変更時にスワップチェインとバッファをリサイズする
     * @param width 新しい幅
     * @param height 新しい高さ
     */
    void Resize(UINT width, UINT height);

    /**
     * @brief 設定ファイルが更新された際に各種リソースを再構築する
     */
    void ReloadResources();



    void SetBackgroundManager(BackgroundManager* manager);

    void SetThumbnailManager(ThumbnailManager* manager);

    TrackDrum& GetTrackDrum() { return m_trackDrum; }

    ID2D1DeviceContext* GetD2DContext() const { return m_d2dContext.Get(); }
    IWICImagingFactory* GetWicFactory() const { return m_wicFactory.Get(); }
    
    // 背景フレーミング情報の受け渡し
    void SetBackgroundFraming(float offsetX, float offsetY, float scale);
    void ClampArtFraming(float& scale, float& offsetX, float& offsetY);

    /**
     * @brief フォーカスされているプレイリストインデックスを設定する
     */
    void SetFocusedPlaylistIndex(std::optional<size_t> idx);

    /**
     * @brief シャッフルインデックス配列を同期する
     */
    void SetShuffleIndices(const std::vector<size_t>& indices);

    /**
     * @brief プレイリストのスクロール量を加算する
     */
    void AddPlaylistScroll(float delta);

    /**
     * @brief プレイリストの現在の手動スクロールオフセットを取得する
     */
    float GetPlaylistManualScrollY() const;

    /**
     * @brief ボリューム操作時のOSD表示をトリガーする
     */
    void TriggerVolumeOsd();

    /**
     * @brief ホバー展開中のUIの格納アニメーションを即座に開始させる
     */
    void ForceClearHoverDelays();

    /**
     * @brief 画面中央のフライテキスト（通知）をトリガーする
     */
    void TriggerFlyText(const std::wstring& text);



private:
    // D3D11 リソース
    Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;

    // DirectComposition リソース（ウィンドウ透過合成用）
    Microsoft::WRL::ComPtr<IDCompositionDevice> m_dcompDevice;
    Microsoft::WRL::ComPtr<IDCompositionTarget> m_dcompTarget;
    Microsoft::WRL::ComPtr<IDCompositionVisual> m_dcompVisual;

    // D2D1 リソース
    Microsoft::WRL::ComPtr<ID2D1Factory1> m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device> m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_d2dContext;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;

    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;


    // キャッシュ済みブラシ群
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_bgDarkenBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_fallbackBlackBrush;

    // キャッシュ済みテキストレイアウト

    std::wstring m_lastTimeString;
    float m_lastVolume = -1.0f;
    size_t m_lastCurrentTrackIndex = static_cast<size_t>(-1);
    size_t m_lastTotalTracks = static_cast<size_t>(-1);
    bool m_forceTextLayoutUpdate = false;

    // WIC および 画像リソース

    Microsoft::WRL::ComPtr<IWICImagingFactory> m_wicFactory;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_placeholderArtBitmap;




    std::optional<size_t> m_focusedPlaylistIndex;
    std::vector<size_t> m_shuffleIndices;

    ThumbnailManager* m_thumbnailManager = nullptr;
    BackgroundManager* m_backgroundManager = nullptr;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_lastCurrentWicImage;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_lastOldWicImage;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_currentBgBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_oldBgBitmap;

    HWND m_hwnd;
    const ConfigManager* m_config;
    float m_dpiScale = 1.0f;
    
    float m_bgOffsetX = 0.0f;
    float m_bgOffsetY = 0.0f;
    float m_bgScale = 1.0f;
    
    float m_controlAlpha = 0.0f;
    float m_controlLeaveTimer = 0.0f;
    float m_osdVolumeAlpha = 0.0f;
    float m_osdVolumeWaitTimer = 0.0f;
    float m_flyTextAlpha = 0.0f;
    float m_flyTextWaitTimer = 0.0f;
    std::wstring m_flyTextString;
    Visualizer m_visualizer;
    TrackDrum m_trackDrum;

    std::vector<std::unique_ptr<IWidget>> m_widgets;

    void DrawBackground();
    void DrawVisualizer(const std::vector<float>& spectrum, const TrackMetadata* currentMeta);

    /**
     * @brief 画像をファイルまたはリソースからロードする
     */
    bool LoadBitmapResource(const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap);

    WidgetContext BuildAnimationContext(float deltaTime, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode, int playbackHoveredIndex, int playlistHoveredItemIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isLogoClicked, int clickedLogoMenuIndex, int clickedPlaybackIndex, bool* outIsPlaylistExpanded, bool* outIsLogoMenuExpanded) const;
    WidgetContext BuildLayoutContext(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>* shuffleMetadataList) const;
    WidgetContext BuildRenderContext(bool isHovered, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isPlaylistListViewMode, bool isPlaying, float progress, const std::vector<float>* spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>* shuffleMetadataList, int playlistToolbarHoveredIndex, const std::vector<PlaylistSummary>* availablePlaylistsCache) const;
};
