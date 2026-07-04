#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>

class AppLogoWidget : public IWidget {
public:
    AppLogoWidget() = default;
    ~AppLogoWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoHoverBitmap;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_shadowEffect;
};

class TrackInfoWidget : public IWidget {
public:
    TrackInfoWidget() = default;
    ~TrackInfoWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_titleTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_artistTextFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_fallbackBlackBrush;
};

class NextTrackWidget : public IWidget {
public:
    NextTrackWidget() = default;
    ~NextTrackWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_nextLabelTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_nextTitleTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_nextArtistTextFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_fallbackBlackBrush;
};

class SeekBarWidget : public IWidget {
public:
    SeekBarWidget() = default;
    ~SeekBarWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_seekBarBgBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_seekBarFgBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_timeTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_timeTextLayout;
    std::wstring m_lastTimeString;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
};

class PlaybackControlsWidget : public IWidget {
public:
    PlaybackControlsWidget() = default;
    ~PlaybackControlsWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_controlBrush;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_playIconGeometry;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_prevIconGeometry;
};

class VolumeControlWidget : public IWidget {
public:
    VolumeControlWidget() = default;
    ~VolumeControlWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_controlBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_speakerIconGeometry;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_volumeTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_volTextLayout;
    float m_lastVolume = -1.0f;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
};

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
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_trackCountTextFormat;

    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_trackCountTextLayout;

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistArtistBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistTimeBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistGripLineBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistGripArrowBrush;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_playlistGripArrowGeometry;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistBgBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistHighlightBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;

    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;

    float m_playlistSlideX;
    float m_playlistManualScrollY;

    size_t m_lastTotalTracks;
};

class ResizeGripWidget : public IWidget {
public:
    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override {}
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override {}
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_resizeGripBrush;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_resizeGripGeometry;
};
