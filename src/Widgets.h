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

class LogoMenuWidget : public IWidget {
public:
    LogoMenuWidget() = default;
    ~LogoMenuWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_iconTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_typingTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_indicatorTextFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_iconBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_inactiveIconBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_lineBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_typingTextBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoBackBitmap;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_shadowEffect;

    float m_menuProgress = 0.0f;
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
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_titleTextLayout;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_artistTextLayout;
    std::wstring m_lastTitle;
    std::wstring m_lastArtist;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_fallbackBlackBrush;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_trackCountTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_trackCountTextLayout;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    size_t m_lastTotalTracks = static_cast<size_t>(-1);
    size_t m_lastCurrentTrackIndex = static_cast<size_t>(-1);
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
