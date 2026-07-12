#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>

class LogoMenuWidget : public IWidget {
public:
    LogoMenuWidget() = default;
    ~LogoMenuWidget() override = default;

    void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) override;
    void ReleaseResources() override;
    void UpdateAnimation(const WidgetContext& ctx) override;
    void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) override;
    void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) override;
    void ForceClearHoverDelay() override;

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_iconTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_typingTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_indicatorTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_lockIconTextFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_iconBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_inactiveIconBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_typingTextBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    

    float m_menuProgress = 0.0f;
    float m_logoMenuLeaveTimer = 0.0f;
    std::vector<float> m_hoverAlpha;
    std::vector<float> m_rippleProgress;
    std::vector<bool> m_isRippling;
};
