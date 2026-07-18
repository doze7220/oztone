#include "Renderer.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr), m_dpiScale(1.0f), m_controlAlpha(0.0f), m_osdVolumeAlpha(0.0f), m_flyTextAlpha(0.0f) {}

Renderer::~Renderer() {}

void Renderer::SetTrackInfo(const std::wstring& title, const std::wstring& artist, const std::wstring& trackNoString, DrumMoveType moveType, size_t currentTrackIndex) {
    // 絶対的なバケツリレー
    m_oldDrumSlot = m_nowDrumSlot;

    // 現在データの保存
    m_nowDrumSlot.title = title;
    m_nowDrumSlot.artist = artist;
    m_nowDrumSlot.trackNo = trackNoString;

    // DrumMoveType に応じた現在位置の正確なワープ
    switch (moveType) {
        case DrumMoveType::Next:
        case DrumMoveType::CrossPlaylist:
            m_drumPosition = static_cast<float>(currentTrackIndex) - 1.0f;
            break;
        case DrumMoveType::Prev:
            m_drumPosition = static_cast<float>(currentTrackIndex) + 1.0f;
            break;
        case DrumMoveType::Reset:
            m_drumPosition = static_cast<float>(currentTrackIndex);
            break;
        case DrumMoveType::Jump:
            // 遠距離の回転アニメーションを表現するため、現在位置を維持
            break;
    }

    if (m_config && m_config->GetEnableTrackDrum() && m_lastCurrentTrackIndex != static_cast<size_t>(-1) && m_lastCurrentTrackIndex != static_cast<size_t>(-2)) {
        m_oldTrackTitle = m_trackTitle;
        m_oldTrackArtist = m_trackArtist;
        m_oldTrackIndex = m_lastCurrentTrackIndex;
        m_oldArtBitmap = m_currentArtBitmap;
        m_oldBgOffsetX = m_bgOffsetX;
        m_oldBgOffsetY = m_bgOffsetY;
        m_oldBgScale = m_bgScale;
        m_isDrumAnimating = true;
    } else {
        m_isDrumAnimating = false;
    }

    m_trackTitle = title;
    m_trackArtist = artist;
}

void Renderer::SetAlbumArt(ID2D1Bitmap* bitmap) {
    m_currentArtBitmap = bitmap;
    m_nowDrumSlot.artBitmap = bitmap;
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
        artBitmap = m_currentArtBitmap ? m_currentArtBitmap.Get() : m_placeholderArtBitmap.Get();
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
