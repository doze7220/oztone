#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <string>

class OsdWidget : public IWidget {
public:
    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;
    void AddScroll(float delta) override {}
    float GetScrollY() const override { return 0.0f; }

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;

    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_textLayout;
    std::wstring m_lastString;

    void RebuildTextLayout(const std::wstring& text, const ConfigManager* config);
};
