#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>

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
