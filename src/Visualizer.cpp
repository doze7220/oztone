#include "Visualizer.h"
#include "Visualizer_PrismBeat.h"
#include "Visualizer_HaloDust.h"
#include "ConfigManager.h"
#include <cmath>
#include <algorithm>

Visualizer::Visualizer() : m_initialized(false) {
    m_prismBeat = std::make_unique<VisualizerPrismBeat>();
    m_haloDust = std::make_unique<VisualizerHaloDust>();
}

Visualizer::~Visualizer() {
    ReleaseResources();
}

bool Visualizer::Initialize(ID2D1DeviceContext* context) {
    if (m_initialized) return true;

    if (m_prismBeat) m_prismBeat->Initialize(context);
    if (m_haloDust) m_haloDust->Initialize(context);

    m_initialized = true;
    return true;
}

void Visualizer::ReleaseResources() {
    if (m_prismBeat) m_prismBeat->ReleaseResources();
    if (m_haloDust) m_haloDust->ReleaseResources();
    m_initialized = false;
}

void Visualizer::Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist, float peakAmplitude, float maxFrequency) {
    if (!m_initialized) {
        Initialize(context);
    }
    if (spectrum.empty() || !m_initialized) return;

    int mode = m_config ? m_config->GetVisualizerMode() : 0;
    if (mode == 0) return;

    size_t validSize = spectrum.size();
    if (maxFrequency > 0.0f && maxFrequency < spectrum.size()) {
        validSize = static_cast<size_t>(maxFrequency) + 1;
        if (validSize > spectrum.size()) validSize = spectrum.size();
    }

    std::vector<float> processedSpectrum(validSize);
    float b0 = 1.0f, b25 = 1.0f, b50 = 1.0f, b75 = 1.0f, b100 = 1.0f;
    if (m_config) {
        b0 = m_config->GetBandGain0();
        b25 = m_config->GetBandGain25();
        b50 = m_config->GetBandGain50();
        b75 = m_config->GetBandGain75();
        b100 = m_config->GetBandGain100();
    }

    for (size_t i = 0; i < validSize; ++i) {
        // ゼロ除算および log10(0) の -inf を回避するため 1.0f を加算する
        float logI = std::log10(static_cast<float>(i) + 1.0f);
        float logMax = std::log10(static_cast<float>(maxFrequency) + 1.0f);
        
        // 対数スケール上での進行割合 (0.0f 〜 1.0f)
        float ratio = (logMax > 0.0f) ? (logI / logMax) : 0.0f;
        ratio = std::clamp(ratio, 0.0f, 1.0f);

        // ピンクノイズ補正とノーマライズ
        float pinkNoiseWeight = 1.0f + (ratio * 15.0f);
        float normalized = (peakAmplitude > 0.001f) ? ((spectrum[i] * pinkNoiseWeight) / peakAmplitude) : 0.0f;
        normalized = std::clamp(normalized, 0.0f, 1.0f);

        float eqMultiplier = 1.0f;
        
        if (ratio <= 0.25f) {
            float localRatio = ratio / 0.25f;
            eqMultiplier = std::lerp(b0, b25, localRatio);
        } else if (ratio <= 0.50f) {
            float localRatio = (ratio - 0.25f) / 0.25f;
            eqMultiplier = std::lerp(b25, b50, localRatio);
        } else if (ratio <= 0.75f) {
            float localRatio = (ratio - 0.50f) / 0.25f;
            eqMultiplier = std::lerp(b50, b75, localRatio);
        } else {
            float localRatio = (ratio - 0.75f) / 0.25f;
            eqMultiplier = std::lerp(b75, b100, localRatio);
        }

        processedSpectrum[i] = std::clamp(normalized * eqMultiplier, 0.0f, 1.0f);
    }

    // mode: 0 = OFF, 1 = PrismBeat, 2 = HaloDust
    if (mode == 1 && m_prismBeat) {
        m_prismBeat->Draw(context, processedSpectrum, drawRect, trackTitle, trackArtist);
    } else if (mode == 2 && m_haloDust) {
        m_haloDust->Draw(context, processedSpectrum, drawRect, trackTitle, trackArtist);
    }
}
