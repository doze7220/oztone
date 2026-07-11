#pragma once
#include "IVisualizerStyle.h"
#include <wrl/client.h>

/**
 * @brief 7色ネオンの心電図風ビジュアライザ (PrismBeat)
 */
class VisualizerPrismBeat : public IVisualizerStyle {
public:
    VisualizerPrismBeat();
    ~VisualizerPrismBeat() override;

    bool Initialize(ID2D1DeviceContext* context) override;
    void ReleaseResources() override;
    void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) override;

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_neonBrushes[7];
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_coreBrush;
    std::vector<float> m_smoothedAmplitudes;
    bool m_initialized;
};
