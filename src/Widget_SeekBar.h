#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>
#include <string>

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
