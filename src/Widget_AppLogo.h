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

    float m_logoHoverAlpha = 0.0f;
    float m_rippleProgress = 0.0f;
    bool m_isRippling = false;
};
