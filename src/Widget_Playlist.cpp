#include "Widget_Playlist.h"

PlaylistWidget::PlaylistWidget()
    : m_playlistSlideX(9999.0f), m_playlistManualScrollY(0.0f) {}

void PlaylistWidget::AddScroll(float delta) {
  m_playlistManualScrollY += delta;
}

float PlaylistWidget::GetScrollY() const { return m_playlistManualScrollY; }

void PlaylistWidget::ForceClearHoverDelay() {
    m_playlistLeaveTimer = 0.0f;
}

