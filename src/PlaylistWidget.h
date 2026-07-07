#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>
#include <unordered_map>

class PlaylistWidget : public IWidget {
public:
    PlaylistWidget();
    ~PlaylistWidget() override = default;


    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

    void AddScroll(float delta) override;
    float GetScrollY() const override;

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_playlistTitleTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_playlistArtistTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_playlistTimeTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_toolbarTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_toolbarIconFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pinSubIconFormat;

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistArtistBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistTimeBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistGripLineBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistGripArrowBrush;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_playlistGripArrowGeometry;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_playlistGripArrowRightGeometry;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistBgBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistHighlightBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;

    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;

    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> m_radarGradientStops;
    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> m_radarGradientBrush;

    float m_playlistSlideX;
    float m_playlistLeaveTimer = 0.0f;
    float m_playlistManualScrollY;
    float m_lastViewHeight = 0.0f;

    size_t m_lastTrackIndex = static_cast<size_t>(-1);
    bool m_isScanlineActive = false;
    float m_scanlineProgress = 0.0f;

    std::unordered_map<int, float> m_hoverAlpha;
};
