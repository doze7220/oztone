#include "Renderer.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr), m_dpiScale(1.0f), m_controlAlpha(0.0f), m_osdVolumeAlpha(0.0f), m_flyTextAlpha(0.0f) {}

Renderer::~Renderer() {}

void Renderer::SetDrumTarget(int relativeDistance, const std::map<int, DrumSlot>& newMetas) {
    std::map<int, DrumSlot> shiftedBuffer;
    for (auto& pair : m_drumBuffer) {
        shiftedBuffer[pair.first - relativeDistance] = pair.second;
    }
    
    for (const auto& pair : newMetas) {
        shiftedBuffer[pair.first].trackTitle = pair.second.trackTitle;
        shiftedBuffer[pair.first].trackArtist = pair.second.trackArtist;
        shiftedBuffer[pair.first].trackNumber = pair.second.trackNumber;
    }
    
    m_drumBuffer = shiftedBuffer;
    m_drumRelativePosition += static_cast<float>(relativeDistance);
    
    if (m_drumBuffer.count(0) > 0) {
        m_drumBuffer[0].artBitmap = nullptr;
    } else {
        m_drumBuffer[0] = DrumSlot();
    }
}

void Renderer::SetAlbumArt(ID2D1Bitmap* bitmap) {
    m_drumBuffer[0].artBitmap = bitmap;
}

void Renderer::SetBackgroundFraming(float offsetX, float offsetY, float scale) {
    m_bgOffsetX = offsetX;
    m_bgOffsetY = offsetY;
    m_bgScale = scale;
}

void Renderer::ClampArtFraming(float scale, float& offsetX, float& offsetY) {
    if (!m_d2dContext) return;
    int bgMode = m_config ? m_config->GetBackgroundArtMode() : 0;
    ID2D1Bitmap* artBitmap = nullptr;
    if (bgMode == 0) {
        artBitmap = (m_drumBuffer.count(0) > 0 && m_drumBuffer[0].artBitmap) ? m_drumBuffer[0].artBitmap.Get() : m_placeholderArtBitmap.Get();
    } else if (bgMode == 2) {
        artBitmap = m_placeholderArtBitmap.Get();
    }
    if (!artBitmap) return;

    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;
    
    float maxOffsetX, maxOffsetY;
    LayoutCalculator::CalculateArtFramingBounds(logicWidth, logicHeight, artBitmap->GetSize(), scale, maxOffsetX, maxOffsetY);

    if (offsetX > maxOffsetX) offsetX = maxOffsetX;
    if (offsetX < -maxOffsetX) offsetX = -maxOffsetX;
    if (offsetY > maxOffsetY) offsetY = maxOffsetY;
    if (offsetY < -maxOffsetY) offsetY = -maxOffsetY;
}

void Renderer::SetFocusedPlaylistIndex(std::optional<size_t> idx) {
    m_focusedPlaylistIndex = idx;
}

void Renderer::AddPlaylistScroll(float delta) {
    for (auto& widget : m_widgets) {
        widget->AddScroll(delta);
    }
}

float Renderer::GetPlaylistManualScrollY() const {
    float scrollY = 0.0f;
    for (auto& widget : m_widgets) {
        scrollY += widget->GetScrollY();
    }
    return scrollY;
}

void Renderer::TriggerVolumeOsd() {
    if (m_config && !m_config->GetEnableOSD()) return;
    m_osdVolumeAlpha = 1.0f;
    m_osdVolumeWaitTimer = m_config ? m_config->GetOsdFadeWait() : 1.0f;
}

void Renderer::TriggerFlyText(const std::wstring& text) {
    if (m_config && !m_config->GetEnableOSD()) return;
    m_flyTextString = text;
    m_flyTextAlpha = 1.0f;
    m_flyTextWaitTimer = m_config ? m_config->GetOsdFadeWait() : 1.0f;
}

void Renderer::ForceClearHoverDelays() {
    m_controlLeaveTimer = 0.0f;
    for (auto& widget : m_widgets) {
        widget->ForceClearHoverDelay();
    }
}

void Renderer::SetShuffleIndices(const std::vector<size_t>& indices) {
    m_shuffleIndices = indices;
}
