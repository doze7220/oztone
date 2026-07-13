#include "Widget_Playlist.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

void PlaylistWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                          const ConfigManager *config) {
  if (!config || !m_textBrush)
    return;

  // --- 1. 描画用レイアウト情報の構築 ---
  D2D1_SIZE_F renderTargetSize = context->GetSize();
  float logicWidth = renderTargetSize.width / ctx.dpiScale;
  float logicHeight = renderTargetSize.height / ctx.dpiScale;

  size_t activeIndex = ctx.currentTrackIndex;
  size_t activeTotal = ctx.totalTracks;

  if (ctx.isPlaylistListViewMode && config) {
    if (ctx.availablePlaylistsCache) {
      activeTotal = ctx.availablePlaylistsCache->size();
      std::wstring currentPlaylist = config->GetDefaultPlaylistPath();
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
      logicWidth, logicHeight, config, m_playlistSlideX,
      m_playlistManualScrollY, activeIndex, activeTotal);

  // ウィンドウリサイズ時等のビュー高さ変動に伴うスクロール位置の補正
  float viewHeight = layout.clipRect.bottom - layout.clipRect.top;
  if (m_lastViewHeight != 0.0f && m_lastViewHeight != viewHeight) {
      float diff = (viewHeight - m_lastViewHeight) / 2.0f;
      m_playlistManualScrollY -= diff;
      layout = LayoutCalculator::CalculatePlaylistLayout(
          logicWidth, logicHeight, config, m_playlistSlideX,
          m_playlistManualScrollY, activeIndex, activeTotal);
  }
  m_lastViewHeight = viewHeight;

  // --- 2. プレイリスト各要素の描画 (目次) ---
  DrawGrip(context, ctx, config, layout);

  if (m_playlistSlideX < layout.playlistWidth - 0.5f) {
    DrawBackground(context, ctx, config, layout);
    DrawToolbar(context, ctx, config, layout);
    DrawPlaylistItems(context, ctx, config, layout);
  }
}

void PlaylistWidget::DrawGrip(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  ID2D1PathGeometry *arrowGeometry =
      config->GetPlaylistPosition() == 0
          ? m_playlistGripArrowRightGeometry.Get()
          : m_playlistGripArrowGeometry.Get();
  if (m_playlistGripLineBrush && m_playlistGripArrowBrush && arrowGeometry) {
    if (m_shadowBrush && config->GetPlaylistGripShadowOpacity() > 0.0f) {
      m_shadowBrush->SetOpacity(config->GetPlaylistGripShadowOpacity());

      context->DrawLine(D2D1::Point2F(layout.gripShadowX, layout.playlistY),
                        D2D1::Point2F(layout.gripShadowX,
                                      layout.playlistY + layout.playlistHeight),
                        m_shadowBrush.Get(), layout.gripLineWidth);

      if (!config->GetIsPlaylistPinned()) {
        D2D1_MATRIX_3X2_F shadowTransform = D2D1::Matrix3x2F::Translation(
            layout.gripShadowX,
            layout.gripShadowY + layout.playlistHeight / 2.0f);
        context->SetTransform(shadowTransform * D2D1::Matrix3x2F::Scale(
                                                    ctx.dpiScale, ctx.dpiScale));
        context->FillGeometry(arrowGeometry, m_shadowBrush.Get());
        context->SetTransform(
            D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
      }
    }

    context->DrawLine(
        D2D1::Point2F(layout.gripX, layout.playlistY),
        D2D1::Point2F(layout.gripX, layout.playlistY + layout.playlistHeight),
        m_playlistGripLineBrush.Get(), layout.gripLineWidth);

    if (!config->GetIsPlaylistPinned()) {
      D2D1_MATRIX_3X2_F arrowTransform = D2D1::Matrix3x2F::Translation(
          layout.gripX, layout.playlistY + layout.playlistHeight / 2.0f);
      context->SetTransform(arrowTransform *
                            D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
      context->FillGeometry(arrowGeometry, m_playlistGripArrowBrush.Get());
      context->SetTransform(D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
    }
  }
}

void PlaylistWidget::DrawBackground(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  if (m_playlistBgBrush) {
    m_playlistBgBrush->SetOpacity(config->GetPlaylistBgOpacity());
    context->FillRectangle(&layout.bgRect, m_playlistBgBrush.Get());
  }
}
