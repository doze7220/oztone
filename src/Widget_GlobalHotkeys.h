#pragma once
#include "Widget.h"
#include <wrl/client.h>
#include <d2d1effects.h>

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
