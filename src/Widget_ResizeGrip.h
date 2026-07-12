#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>

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
