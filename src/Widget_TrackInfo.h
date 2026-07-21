#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>
#include <string>

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

    float m_artCrossfadeProgress = 1.0f;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_lastArtBitmap;
    bool m_wasDrumAnimating = false;
    int m_animatingOldIndexOffset = 0;

    std::array<float, 3> m_thumbFadeAlpha = {0.0f, 0.0f, 0.0f};

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_fallbackBlackBrush;

    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;

};
