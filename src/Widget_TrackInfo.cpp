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
      config->GetBaseFontFamily().c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetTitleFontSize(), L"ja-jp", &m_titleTextFormat);
  dwriteFactory->CreateTextFormat(
      config->GetBaseFontFamily().c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetArtistFontSize(), L"ja-jp", &m_artistTextFormat);

  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_titleTextFormat.Get());
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_artistTextFormat.Get());

  m_dwriteFactory = dwriteFactory;

  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_fallbackBlackBrush);
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
}

void TrackInfoWidget::UpdateAnimation(const WidgetContext &ctx) {
  bool isDrumAnimating = (ctx.drumRelativePosition != 0.0f);
  auto currentArt = (ctx.drumBuffer.count(0) > 0 && ctx.drumBuffer.at(0).artBitmap) ? ctx.drumBuffer.at(0).artBitmap : nullptr;
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
}

void TrackInfoWidget::UpdateLayout(const WidgetContext &ctx,
                                   const ConfigManager *config) {
  if (!config)
    return;

  if (m_dwriteFactory) {
    std::wstring currentTitle = ctx.drumBuffer.count(0) > 0 ? ctx.drumBuffer.at(0).trackTitle : L"";
    std::wstring currentArtist = ctx.drumBuffer.count(0) > 0 ? ctx.drumBuffer.at(0).trackArtist : L"";

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
  if (config && config->GetShowNowPlaying()) {
    D2D1_SIZE_F rtSize = context->GetSize();
    float logicWidth = rtSize.width / ctx.dpiScale;
    float logicHeight = rtSize.height / ctx.dpiScale;

    D2D1_SIZE_F bitmapSize = (ctx.drumBuffer.count(0) > 0 && ctx.drumBuffer.at(0).artBitmap)
                                 ? ctx.drumBuffer.at(0).artBitmap->GetSize()
                                 : D2D1::SizeF(0.0f, 0.0f);
    TrackInfoLayout layout = LayoutCalculator::CalculateTrackInfoLayout(
        logicWidth, logicHeight, config, bitmapSize);

    float slotHeight = config->GetArtSize() + config->GetShadowOffsetY() + 1.0f;

    D2D1_RECT_F drumClipRect = layout.clipRect;
    drumClipRect.top = layout.fallbackArtRect.top;
    drumClipRect.bottom = layout.fallbackArtRect.top + slotHeight;
    context->PushAxisAlignedClip(drumClipRect, D2D1_ANTIALIAS_MODE_ALIASED);

    auto drawDrumItem = [&](int relativeIndex, double diffOffset) {
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

      if (ctx.drumBuffer.count(relativeIndex) > 0) {
          const auto& slot = ctx.drumBuffer.at(relativeIndex);
          art = slot.artBitmap.Get();
          textTitle = slot.trackTitle;
          textArtist = slot.trackArtist;
      }

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

      bool drawGlass = (art == nullptr);
      float artOpacity = 1.0f;
      float glassAlphaMultiplier = 1.0f;

      if (drawGlass) {
          artOpacity = 0.0f;
      } else if (relativeIndex == 0 && m_artCrossfadeProgress < 1.0f) {
          drawGlass = true;
          artOpacity = m_artCrossfadeProgress;
          glassAlphaMultiplier = 1.0f - m_artCrossfadeProgress;
      }

      if (drawGlass && m_fallbackBlackBrush) {
        m_fallbackBlackBrush->SetOpacity(config->GetFallbackArtOpacity() * glassAlphaMultiplier);
        context->FillRectangle(&layout.fallbackArtRect, m_fallbackBlackBrush.Get());
      }

      if (artOpacity > 0.0f && art) {
        D2D1_RECT_F itemArtDestRect = layout.fallbackArtRect;
        D2D1_RECT_F itemArtShadowRect = layout.fallbackArtRect;
        D2D1_SIZE_F bitmapSize = art->GetSize();
        if (bitmapSize.width > 0 && bitmapSize.height > 0) {
            float size = static_cast<float>(config->GetArtSize());
            float scaleX = size / bitmapSize.width;
            float scaleY = size / bitmapSize.height;
            float scale = (scaleX < scaleY) ? scaleX : scaleY;
            
            float drawWidth = bitmapSize.width * scale;
            float drawHeight = bitmapSize.height * scale;
            
            float drawX = layout.fallbackArtRect.left + (size - drawWidth) / 2.0f;
            float drawY = layout.fallbackArtRect.top + (size - drawHeight) / 2.0f;
            
            itemArtDestRect = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);
            itemArtShadowRect = D2D1::RectF(
                drawX + config->GetShadowOffsetX(),
                drawY + config->GetShadowOffsetY(),
                drawX + drawWidth + config->GetShadowOffsetX(),
                drawY + drawHeight + config->GetShadowOffsetY()
            );
        }

        if (m_shadowBrush && config->GetEnableShadow()) {
          m_shadowBrush->SetOpacity(config->GetShadowOpacity() * artOpacity);
          context->FillRectangle(&itemArtShadowRect, m_shadowBrush.Get());
        }
        context->DrawBitmap(art, &itemArtDestRect, artOpacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
      }

      if (m_textBrush && titleLayout && artistLayout) {
        titleLayout->SetMaxWidth(layout.titleRect.right - layout.titleRect.left);
        titleLayout->SetMaxHeight(layout.titleRect.bottom - layout.titleRect.top);
        artistLayout->SetMaxWidth(layout.artistRect.right - layout.artistRect.left);
        artistLayout->SetMaxHeight(layout.artistRect.bottom - layout.artistRect.top);

        float shadowOpacity = config->GetEnableShadow() ? config->GetShadowOpacity() : 0.0f;
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
        double diffOffset = static_cast<double>(i) - ctx.drumRelativePosition;
        drawDrumItem(i, diffOffset);
    }

    context->PopAxisAlignedClip();
  }
}
