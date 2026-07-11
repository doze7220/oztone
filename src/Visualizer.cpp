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

void Visualizer::Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) {
    if (!m_initialized) {
        Initialize(context);
    }
    if (spectrum.empty() || !m_initialized) return;

    int mode = m_config ? m_config->GetVisualizerMode() : 0;
    
    // mode: 0 = OFF, 1 = PrismBeat, 2 = HaloDust
    if (mode == 1 && m_prismBeat) {
        m_prismBeat->Draw(context, spectrum, drawRect, trackTitle, trackArtist);
    } else if (mode == 2 && m_haloDust) {
        m_haloDust->Draw(context, spectrum, drawRect, trackTitle, trackArtist);
    }
}
