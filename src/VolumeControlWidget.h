#pragma once
#include "Widget.h"
#include "WidgetContext.h"
#include <wrl/client.h>

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
