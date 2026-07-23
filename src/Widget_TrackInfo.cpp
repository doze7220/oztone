#include "Widget_TrackInfo.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <cwchar>

// ==========================================
// TrackInfoWidget
// ==========================================
void TrackInfoWidget::CreateResources(ID2D1DeviceContext *context,
                                      IWICImagingFactory *wicFactory,
                                      IDWriteFactory *dwriteFactory,
                                      const ConfigManager *config) {
  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetLayoutNowPlaying().TitleFontSize, L"ja-jp", &m_titleTextFormat);
  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetLayoutNowPlaying().ArtistFontSize, L"ja-jp", &m_artistTextFormat);

  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_titleTextFormat.Get());
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_artistTextFormat.Get());

  m_dwriteFactory = dwriteFactory;

  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_fallbackBlackBrush);

  if (config) {
    context->CreateSolidColorBrush(ParseHexColor(config->GetLayoutTooltip().TooltipBgColor), &m_tooltipBgBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_tooltipIconBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.3f, 0.3f, 1.0f), &m_tooltipWheelBrush);
  }

  context->GetFactory(&m_d2dFactory);

  if (m_d2dFactory) {
    m_d2dFactory->CreatePathGeometry(&m_tooltipStrokeGeometry);
    if (m_tooltipStrokeGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_tooltipStrokeGeometry->Open(&sink);
      
      float mx = -0.13f;
      float rx = 0.15f, ry = 0.25f, r = 0.1f;
      
      sink->BeginFigure(D2D1::Point2F(mx-rx+r, -ry), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(mx+rx-r, -ry));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(mx+rx, -ry+r), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      sink->AddLine(D2D1::Point2F(mx+rx, ry-r));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(mx+rx-r, ry), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      sink->AddLine(D2D1::Point2F(mx-rx+r, ry));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(mx-rx, ry-r), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      sink->AddLine(D2D1::Point2F(mx-rx, -ry+r));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(mx-rx+r, -ry), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);

      sink->BeginFigure(D2D1::Point2F(mx-rx, -0.05f), D2D1_FIGURE_BEGIN_HOLLOW);
      sink->AddLine(D2D1::Point2F(mx+rx, -0.05f));
      sink->EndFigure(D2D1_FIGURE_END_OPEN);
      
      sink->BeginFigure(D2D1::Point2F(mx, -ry), D2D1_FIGURE_BEGIN_HOLLOW);
      sink->AddLine(D2D1::Point2F(mx, -0.05f));
      sink->EndFigure(D2D1_FIGURE_END_OPEN);

      sink->Close();
    }

    m_d2dFactory->CreatePathGeometry(&m_tooltipFillGeometry);
    if (m_tooltipFillGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_tooltipFillGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      
      float ax = 0.20f;
      float aw = 0.08f;

      sink->BeginFigure(D2D1::Point2F(ax-aw, -0.05f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(ax, -0.25f));
      sink->AddLine(D2D1::Point2F(ax+aw, -0.05f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      
      sink->BeginFigure(D2D1::Point2F(ax-aw, 0.05f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(ax, 0.25f));
      sink->AddLine(D2D1::Point2F(ax+aw, 0.05f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      
      sink->Close();
    }

    m_d2dFactory->CreatePathGeometry(&m_tooltipWheelGeometry);
    if (m_tooltipWheelGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_tooltipWheelGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      
      float mx = -0.13f;
      float wx = 0.03f;

      sink->BeginFigure(D2D1::Point2F(mx-wx, -0.20f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(mx+wx, -0.20f));
      sink->AddLine(D2D1::Point2F(mx+wx, -0.08f));
      sink->AddLine(D2D1::Point2F(mx-wx, -0.08f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      
      sink->Close();
    }

    m_d2dFactory->CreatePathGeometry(&m_tooltipGeometry);
    if (m_tooltipGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_tooltipGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);

      float w = config ? config->GetLayoutTooltip().TooltipWidth : 50.0f;
      float h = config ? config->GetLayoutTooltip().TooltipHeight : 26.0f;
      float r = 4.0f;

      sink->BeginFigure(D2D1::Point2F(r, 0.0f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(w - r, 0.0f));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(w, r), D2D1::SizeF(r, r), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      sink->AddLine(D2D1::Point2F(w, h - r));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(w - r, h), D2D1::SizeF(r, r), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      
      sink->AddLine(D2D1::Point2F(w / 2.0f + 6.0f, h));
      sink->AddLine(D2D1::Point2F(w / 2.0f, h + 6.0f));
      sink->AddLine(D2D1::Point2F(w / 2.0f - 6.0f, h));

      sink->AddLine(D2D1::Point2F(r, h));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.0f, h - r), D2D1::SizeF(r, r), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      sink->AddLine(D2D1::Point2F(0.0f, r));
      sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(r, 0.0f), D2D1::SizeF(r, r), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
      
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }
  }
}

void TrackInfoWidget::ReleaseResources() {
  m_titleTextFormat.Reset();
  m_artistTextFormat.Reset();
  m_titleTextLayout.Reset();
  m_artistTextLayout.Reset();
  m_shadowBrush.Reset();
  m_textBrush.Reset();
  m_fallbackBlackBrush.Reset();
  m_dwriteFactory.Reset();

  m_tooltipBgBrush.Reset();
  m_tooltipIconBrush.Reset();
  m_tooltipWheelBrush.Reset();
  m_tooltipStrokeGeometry.Reset();
  m_tooltipFillGeometry.Reset();
  m_tooltipWheelGeometry.Reset();
  m_tooltipGeometry.Reset();
  m_d2dFactory.Reset();
}

void TrackInfoWidget::UpdateAnimation(const WidgetContext &ctx) {
  bool isDrumAnimating = (std::abs(ctx.drumRelativePosition - static_cast<float>(ctx.animatingTargetIndex)) > 0.001f);
  auto currentArt = ctx.drumSlots[ctx.currentDrumSlotIndex].artBitmap ? ctx.drumSlots[ctx.currentDrumSlotIndex].artBitmap : nullptr;
  if (!isDrumAnimating) {
    if (!m_lastArtBitmap && currentArt) {
      m_artCrossfadeProgress = 0.0f;
    } else if (m_artCrossfadeProgress < 1.0f) {
      m_artCrossfadeProgress += ctx.deltaTime * 3.0f; // 300ms fade
      if (m_artCrossfadeProgress > 1.0f) m_artCrossfadeProgress = 1.0f;
    }
    m_lastArtBitmap = currentArt;
  } else {
    m_artCrossfadeProgress = 1.0f;
  }
  m_wasDrumAnimating = isDrumAnimating;

  for (int i = 0; i < 3; ++i) {
      if (!ctx.drumSlots[i].artBitmap) {
          m_thumbFadeAlpha[i] = 0.0f;
      } else {
          m_thumbFadeAlpha[i] += 2.0f * ctx.deltaTime;
          if (m_thumbFadeAlpha[i] > 1.0f) {
              m_thumbFadeAlpha[i] = 1.0f;
          }
      }
  }

  float fadeOutSpeed = ctx.config ? ctx.config->GetUICommonParm().HoverFadeOutSpeed : 3.0f;
  float fadeInSpeed = 10.0f;
  
  if (ctx.isTrackInfoHovered && !ctx.isJogDialing) {
    m_hoverAlpha += ctx.deltaTime * fadeInSpeed;
    if (m_hoverAlpha > 1.0f) m_hoverAlpha = 1.0f;
  } else {
    m_hoverAlpha -= ctx.deltaTime * fadeOutSpeed;
    if (m_hoverAlpha < 0.0f) m_hoverAlpha = 0.0f;
  }
}

void TrackInfoWidget::UpdateLayout(const WidgetContext &ctx,
                                   const ConfigManager *config) {
  if (!config)
    return;

  if (m_dwriteFactory) {
    std::wstring currentTitle = ctx.drumSlots[ctx.currentDrumSlotIndex].trackTitle;
    std::wstring currentArtist = ctx.drumSlots[ctx.currentDrumSlotIndex].trackArtist;

    if (m_titleTextFormat &&
        (!m_titleTextLayout || m_lastTitle != currentTitle)) {
      m_lastTitle = currentTitle;
      m_titleTextLayout.Reset();
      if (!currentTitle.empty()) {
        m_dwriteFactory->CreateTextLayout(
            currentTitle.c_str(), static_cast<UINT32>(currentTitle.length()),
            m_titleTextFormat.Get(), 4000.0f, 1000.0f, &m_titleTextLayout);
      }
    }

    if (m_artistTextFormat &&
        (!m_artistTextLayout || m_lastArtist != currentArtist)) {
      m_lastArtist = currentArtist;
      m_artistTextLayout.Reset();
      if (!currentArtist.empty()) {
        m_dwriteFactory->CreateTextLayout(
            currentArtist.c_str(),
            static_cast<UINT32>(currentArtist.length()),
            m_artistTextFormat.Get(), 4000.0f, 1000.0f, &m_artistTextLayout);
      }
    }
  }


}

void TrackInfoWidget::Draw(ID2D1DeviceContext *context,
                           const WidgetContext &ctx,
                           const ConfigManager *config) {
  if (config && config->GetVisibility().ShowNowPlaying) {
    D2D1_SIZE_F rtSize = context->GetSize();
    float logicWidth = rtSize.width / ctx.dpiScale;
    float logicHeight = rtSize.height / ctx.dpiScale;

    D2D1_SIZE_F bitmapSize = ctx.drumSlots[ctx.currentDrumSlotIndex].artBitmap
                                 ? ctx.drumSlots[ctx.currentDrumSlotIndex].artBitmap->GetSize()
                                 : D2D1::SizeF(0.0f, 0.0f);
    TrackInfoLayout layout = LayoutCalculator::CalculateTrackInfoLayout(
        logicWidth, logicHeight, config, bitmapSize);

    float slotHeight = config->GetLayoutNowPlaying().ArtSize + config->GetUICommonParm().ShadowOffsetY + 1.0f;

    D2D1_RECT_F drumClipRect = layout.clipRect;
    drumClipRect.top = layout.fallbackArtRect.top;
    drumClipRect.bottom = layout.fallbackArtRect.top + slotHeight;
    context->PushAxisAlignedClip(drumClipRect, D2D1_ANTIALIAS_MODE_ALIASED);

    auto drawDrumItem = [&](int absoluteIndex, double diffOffset) {
      if (std::abs(diffOffset) > 2.0) return;

      float offsetY = static_cast<float>(diffOffset) * slotHeight;
      D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(0.0f, offsetY);
      D2D1::Matrix3x2F originalTransform;
      context->GetTransform(&originalTransform);
      context->SetTransform(transform * originalTransform);

      ID2D1Bitmap* art = nullptr;
      IDWriteTextLayout* titleLayout = nullptr;
      IDWriteTextLayout* artistLayout = nullptr;

      Microsoft::WRL::ComPtr<IDWriteTextLayout> tempTitleLayout;
      Microsoft::WRL::ComPtr<IDWriteTextLayout> tempArtistLayout;

      std::wstring textTitle;
      std::wstring textArtist;

      int slotIndex = (absoluteIndex % 3 + 3) % 3;
      const auto& slot = ctx.drumSlots[slotIndex];
      art = slot.artBitmap.Get();
      textTitle = slot.trackTitle;
      textArtist = slot.trackArtist;

      if (textTitle == m_lastTitle && textArtist == m_lastArtist) {
          titleLayout = m_titleTextLayout.Get();
          artistLayout = m_artistTextLayout.Get();
      } else {
          if (!textTitle.empty() && m_dwriteFactory && m_titleTextFormat) {
              m_dwriteFactory->CreateTextLayout(
                  textTitle.c_str(), static_cast<UINT32>(textTitle.length()),
                  m_titleTextFormat.Get(), 4000.0f, 1000.0f, &tempTitleLayout);
              titleLayout = tempTitleLayout.Get();
          }
          if (!textArtist.empty() && m_dwriteFactory && m_artistTextFormat) {
              m_dwriteFactory->CreateTextLayout(
                  textArtist.c_str(), static_cast<UINT32>(textArtist.length()),
                  m_artistTextFormat.Get(), 4000.0f, 1000.0f, &tempArtistLayout);
              artistLayout = tempArtistLayout.Get();
          }

      }

      if (m_fallbackBlackBrush) {
        float fallbackOpacity = config->GetLayoutNowPlaying().FallbackArtOpacity * (1.0f - m_thumbFadeAlpha[slotIndex]);
        m_fallbackBlackBrush->SetOpacity(fallbackOpacity);
        context->FillRectangle(&layout.fallbackArtRect, m_fallbackBlackBrush.Get());
      }

      float artOpacity = m_thumbFadeAlpha[slotIndex];
      if (absoluteIndex == ctx.animatingTargetIndex && m_artCrossfadeProgress < 1.0f) {
          artOpacity *= m_artCrossfadeProgress;
      }

      if (artOpacity > 0.0f && art) {
        D2D1_RECT_F itemArtDestRect = layout.fallbackArtRect;
        D2D1_RECT_F itemArtShadowRect = layout.fallbackArtRect;
        D2D1_SIZE_F artSize = art->GetSize();
        if (artSize.width > 0 && artSize.height > 0) {
            float size = static_cast<float>(config->GetLayoutNowPlaying().ArtSize);
            float scaleX = size / artSize.width;
            float scaleY = size / artSize.height;
            float scale = (scaleX < scaleY) ? scaleX : scaleY;
            
            float drawWidth = artSize.width * scale;
            float drawHeight = artSize.height * scale;
            
            float drawX = layout.fallbackArtRect.left + (size - drawWidth) / 2.0f;
            float drawY = layout.fallbackArtRect.top + (size - drawHeight) / 2.0f;
            
            itemArtDestRect = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);
            itemArtShadowRect = D2D1::RectF(
                drawX + config->GetUICommonParm().ShadowOffsetX,
                drawY + config->GetUICommonParm().ShadowOffsetY,
                drawX + drawWidth + config->GetUICommonParm().ShadowOffsetX,
                drawY + drawHeight + config->GetUICommonParm().ShadowOffsetY
            );
        }

        if (m_shadowBrush && config->GetUICommonParm().EnableShadow) {
          m_shadowBrush->SetOpacity(config->GetUICommonParm().ShadowOpacity * artOpacity);
          context->FillRectangle(&itemArtShadowRect, m_shadowBrush.Get());
        }
        context->DrawBitmap(art, &itemArtDestRect, artOpacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
      }

      if (m_textBrush && titleLayout && artistLayout) {
        D2D1_COLOR_F baseColor = D2D1::ColorF(D2D1::ColorF::White);
        D2D1_COLOR_F hoverColor = config ? ParseHexColor(config->GetUICommonParm().FocusColor) : baseColor;
        float t = (absoluteIndex == ctx.animatingTargetIndex) ? m_hoverAlpha : 0.0f;
        m_textBrush->SetColor(D2D1::ColorF(
            baseColor.r + (hoverColor.r - baseColor.r) * t,
            baseColor.g + (hoverColor.g - baseColor.g) * t,
            baseColor.b + (hoverColor.b - baseColor.b) * t,
            1.0f
        ));
        titleLayout->SetMaxWidth(layout.titleRect.right - layout.titleRect.left);
        titleLayout->SetMaxHeight(layout.titleRect.bottom - layout.titleRect.top);
        artistLayout->SetMaxWidth(layout.artistRect.right - layout.artistRect.left);
        artistLayout->SetMaxHeight(layout.artistRect.bottom - layout.artistRect.top);

        float shadowOpacity = config->GetUICommonParm().EnableShadow ? config->GetUICommonParm().ShadowOpacity : 0.0f;
        WidgetCommon::DrawShadowedTextLayout(
            context, titleLayout, m_textBrush.Get(), m_shadowBrush.Get(),
            D2D1::Point2F(layout.titleRect.left, layout.titleRect.top),
            D2D1::Point2F(layout.titleShadowRect.left, layout.titleShadowRect.top),
            shadowOpacity);

        WidgetCommon::DrawShadowedTextLayout(
            context, artistLayout, m_textBrush.Get(), m_shadowBrush.Get(),
            D2D1::Point2F(layout.artistRect.left, layout.artistRect.top),
            D2D1::Point2F(layout.artistShadowRect.left, layout.artistShadowRect.top),
            shadowOpacity);
      }



      context->SetTransform(originalTransform);
    };

    int startSlot = static_cast<int>(std::floor(ctx.drumRelativePosition - 1.5f));
    int endSlot   = static_cast<int>(std::ceil(ctx.drumRelativePosition + 1.5f));

    for (int i = startSlot; i <= endSlot; ++i) {
        if (ctx.totalTracks == 0 && i != 0) continue;
        double diffOffset = ctx.drumRelativePosition - static_cast<double>(i);
        drawDrumItem(i, diffOffset);
    }

    context->PopAxisAlignedClip();

    if (m_hoverAlpha > 0.0f && m_tooltipGeometry && m_tooltipStrokeGeometry) {
      D2D1_MATRIX_3X2_F oldTransform;
      context->GetTransform(&oldTransform);
      
      float tooltipW = config->GetLayoutTooltip().TooltipWidth;
      float tooltipH = config->GetLayoutTooltip().TooltipHeight;
      float offsetX = config->GetLayoutNowPlaying().TooltipOffsetX;
      float offsetY = config->GetLayoutVolumeControl().TooltipOffsetY;
      
      float artCenterX = layout.fallbackArtRect.left + (layout.fallbackArtRect.right - layout.fallbackArtRect.left) / 2.0f;
      float artCenterY = layout.fallbackArtRect.top + (layout.fallbackArtRect.bottom - layout.fallbackArtRect.top) / 2.0f;
      
      float tooltipX = artCenterX + offsetX;
      float tooltipY = artCenterY + offsetY;

      WidgetCommon::DrawMouseScrollTooltip(
          context,
          tooltipX, tooltipY,
          tooltipW, tooltipH,
          m_tooltipGeometry.Get(),
          m_tooltipStrokeGeometry.Get(),
          m_tooltipFillGeometry.Get(),
          m_tooltipWheelGeometry.Get(),
          m_tooltipBgBrush.Get(),
          m_tooltipIconBrush.Get(),
          m_tooltipWheelBrush.Get(),
          m_hoverAlpha,
          config->GetLayoutTooltip().TooltipBgOpacity,
          config->GetLayoutTooltip().TooltipIconSize,
          oldTransform
      );
    }
  }
}
