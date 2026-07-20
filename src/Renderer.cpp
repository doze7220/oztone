#include "Renderer.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr), m_dpiScale(1.0f), m_controlAlpha(0.0f), m_osdVolumeAlpha(0.0f), m_flyTextAlpha(0.0f) {}

Renderer::~Renderer() {}



void Renderer::SetBackgroundManager(BackgroundManager* manager) {
    m_backgroundManager = manager;
}

void Renderer::SetThumbnailManager(ThumbnailManager* manager) {
    m_thumbnailManager = manager;
}

void Renderer::SetBackgroundFraming(float offsetX, float offsetY, float scale) {
    m_bgOffsetX = offsetX;
    m_bgOffsetY = offsetY;
    m_bgScale = scale;
}

void Renderer::ClampArtFraming(float& scale, float& offsetX, float& offsetY) {
    if (!m_d2dContext || !m_currentBgBitmap) return;

    D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
    float logicWidth = rtSize.width / m_dpiScale;
    float logicHeight = rtSize.height / m_dpiScale;

    D2D1_SIZE_F imgSize = m_currentBgBitmap->GetSize();
    if (imgSize.width <= 0.0f || imgSize.height <= 0.0f) return;

    float baseScaleX = logicWidth / imgSize.width;
    float baseScaleY = logicHeight / imgSize.height;
    float baseScale = (std::max)(baseScaleX, baseScaleY);

    float actualScale = baseScale * scale;
    float scaledWidth = imgSize.width * actualScale;
    float scaledHeight = imgSize.height * actualScale;

    float maxOffsetX = (scaledWidth - logicWidth) / 2.0f;
    float maxOffsetY = (scaledHeight - logicHeight) / 2.0f;

    if (maxOffsetX < 0.0f) maxOffsetX = 0.0f;
    if (maxOffsetY < 0.0f) maxOffsetY = 0.0f;

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
