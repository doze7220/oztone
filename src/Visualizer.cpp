#include "Visualizer.h"
#include "Visualizer_PrismBeat.h"
#include "Visualizer_HaloDust.h"
#include "ConfigManager.h"

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
    float normalizeFactor = (peakAmplitude > 0.0f) ? (1.0f / peakAmplitude) : 1.0f;

    float b0 = 1.0f, b25 = 1.0f, b50 = 1.0f, b75 = 1.0f, b100 = 1.0f;
    if (m_config) {
        b0 = m_config->GetBandGain0();
        b25 = m_config->GetBandGain25();
        b50 = m_config->GetBandGain50();
        b75 = m_config->GetBandGain75();
        b100 = m_config->GetBandGain100();
    }

    for (size_t i = 0; i < validSize; ++i) {
        float val = spectrum[i] * normalizeFactor;

        float pos = static_cast<float>(i) / static_cast<float>(validSize > 1 ? validSize - 1 : 1);
        float eqMultiplier = 1.0f;
        
        if (pos < 0.25f) {
            float t = pos / 0.25f;
            eqMultiplier = b0 + t * (b25 - b0);
        } else if (pos < 0.5f) {
            float t = (pos - 0.25f) / 0.25f;
            eqMultiplier = b25 + t * (b50 - b25);
        } else if (pos < 0.75f) {
            float t = (pos - 0.5f) / 0.25f;
            eqMultiplier = b50 + t * (b75 - b50);
        } else {
            float t = (pos - 0.75f) / 0.25f;
            eqMultiplier = b75 + t * (b100 - b75);
        }

        processedSpectrum[i] = val * eqMultiplier;
    }

    // mode: 0 = OFF, 1 = PrismBeat, 2 = HaloDust
    if (mode == 1 && m_prismBeat) {
        m_prismBeat->Draw(context, processedSpectrum, drawRect, trackTitle, trackArtist);
    } else if (mode == 2 && m_haloDust) {
        m_haloDust->Draw(context, processedSpectrum, drawRect, trackTitle, trackArtist);
    }
}
