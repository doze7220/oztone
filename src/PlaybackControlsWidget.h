#pragma once
#include "Widget.h"
#include "WidgetContext.h"
#include <wrl/client.h>

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
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_chevronLeftGeometry;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_chevronRightGeometry;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_indicatorTextLayout;
    float m_hoverAlpha[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
};
