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

class GlobalHotkeysWidget : public IWidget {
public:
    GlobalHotkeysWidget() = default;
    ~GlobalHotkeysWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_keyTextLayout;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_actionTextLayout;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_coreBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_glowBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;

    std::wstring m_lastKeyString;
    std::wstring m_lastActionString;
    bool m_wasShow = false;

    std::wstring GetKeyName(int mod, int vk);
    std::wstring GetActionName(int actionId);
    void GenerateHotkeysStrings(const ConfigManager* config, std::wstring& outKeys, std::wstring& outActions);
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
