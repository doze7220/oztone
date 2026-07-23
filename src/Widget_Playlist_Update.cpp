#include "Widget_Playlist.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <filesystem>

void PlaylistWidget::UpdateAnimation(const WidgetContext &ctx) {
  if (!ctx.config)
    return;

  float configPlaylistWidth =
      static_cast<float>(ctx.config->GetLayoutPlaylist().PlaylistWidth);
  if (m_playlistSlideX > configPlaylistWidth * 2.0f)
    m_playlistSlideX = configPlaylistWidth;

  bool isExpanded = ctx.isPlaylistHovered || ctx.config->GetLayoutPlaylist().IsPlaylistPinned;
  
  if (isExpanded) {
      m_playlistLeaveTimer = ctx.config->GetLayoutPlaylist().PlaylistLeaveDelay;
  } else {
      if (m_playlistLeaveTimer > 0.0f) {
          m_playlistLeaveTimer -= ctx.deltaTime;
          isExpanded = true;
      }
  }

  float targetSlideX = isExpanded ? 0.0f : configPlaylistWidth;
  m_playlistSlideX += (targetSlideX - m_playlistSlideX) * 0.2f;

  if (ctx.outIsPlaylistExpanded) {
      *ctx.outIsPlaylistExpanded = isExpanded;
  }

  if (!isExpanded) {
    m_playlistManualScrollY = 0.0f;
  } else {
    float logicWidth = ctx.logicalWidth;
    float logicHeight = ctx.logicalHeight;
    if (logicWidth == 0.0f || logicHeight == 0.0f) {
        D2D1_SIZE_F renderTargetSize =
            D2D1::SizeF(ctx.config->GetWindow().WindowWidth * ctx.dpiScale,
                        ctx.config->GetWindow().WindowHeight * ctx.dpiScale);
        logicWidth = renderTargetSize.width / ctx.dpiScale;
        logicHeight = renderTargetSize.height / ctx.dpiScale;
    }

    size_t activeIndex = ctx.currentTrackIndex;
    size_t activeTotal = ctx.totalTracks;

    if (ctx.isPlaylistListViewMode) {
      if (ctx.availablePlaylistsCache) {
        activeTotal = ctx.availablePlaylistsCache->size();
        std::wstring currentPlaylist = ctx.config->GetPlaylist().DefaultPlaylistPath;
        activeIndex = 0;
        for (size_t i = 0; i < ctx.availablePlaylistsCache->size(); ++i) {
          if ((*ctx.availablePlaylistsCache)[i].filepath == currentPlaylist) {
            activeIndex = i;
            break;
          }
        }
      } else {
        activeTotal = 0;
        activeIndex = 0;
      }
    }

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicWidth, logicHeight, ctx.config, m_playlistSlideX,
        m_playlistManualScrollY, activeIndex, activeTotal);
    m_playlistManualScrollY = layout.newManualScrollY;
  }

  if (m_lastTrackIndex != static_cast<size_t>(-1) && m_lastTrackIndex != ctx.currentTrackIndex && !ctx.isPlaylistListViewMode) {
      m_isScanlineActive = true;
      m_scanlineProgress = 0.0f;
  }
  m_lastTrackIndex = ctx.currentTrackIndex;

  if (m_isScanlineActive) {
      m_scanlineProgress += ctx.deltaTime * 2.0f;
      if (m_scanlineProgress >= 1.0f) {
          m_isScanlineActive = false;
          m_scanlineProgress = 1.0f;
      }
  }

  float fadeOutSpeed = ctx.config->GetUICommonParm().HoverFadeOutSpeed * ctx.deltaTime;
  float fadeInSpeed = 10.0f * ctx.deltaTime;

  for (auto it = m_hoverAlpha.begin(); it != m_hoverAlpha.end(); ) {
      if (it->first == ctx.playlistHoveredItemIndex) {
          it->second += fadeInSpeed;
          if (it->second > 1.0f) it->second = 1.0f;
          ++it;
      } else {
          it->second -= fadeOutSpeed;
          if (it->second <= 0.0f) {
              it = m_hoverAlpha.erase(it);
          } else {
              ++it;
          }
      }
  }

  if (ctx.playlistHoveredItemIndex >= 0 && m_hoverAlpha.find(ctx.playlistHoveredItemIndex) == m_hoverAlpha.end()) {
      m_hoverAlpha[ctx.playlistHoveredItemIndex] = fadeInSpeed > 1.0f ? 1.0f : fadeInSpeed;
  }
}

void PlaylistWidget::UpdateLayout(const WidgetContext &ctx,
                                  const ConfigManager *config) {}
