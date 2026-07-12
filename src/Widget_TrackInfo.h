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
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_fallbackBlackBrush;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_trackCountTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_trackCountTextLayout;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    size_t m_lastTotalTracks = static_cast<size_t>(-1);
    size_t m_lastCurrentTrackIndex = static_cast<size_t>(-1);
};
