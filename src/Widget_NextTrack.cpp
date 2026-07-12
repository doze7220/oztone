#include "Widget_NextTrack.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <filesystem>

// ==========================================
// NextTrackWidget
// ==========================================
void NextTrackWidget::CreateResources(ID2D1DeviceContext *context,
                                      IWICImagingFactory *wicFactory,
                                      IDWriteFactory *dwriteFactory,
                                      const ConfigManager *config) {
  dwriteFactory->CreateTextFormat(
      config->GetNextLabelFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetNextLabelFontSize(), L"ja-jp",
      &m_nextLabelTextFormat);
  dwriteFactory->CreateTextFormat(
      config->GetNextTitleFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetNextTitleFontSize(), L"ja-jp",
      &m_nextTitleTextFormat);
  dwriteFactory->CreateTextFormat(
      config->GetNextArtistFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetNextArtistFontSize(), L"ja-jp",
      &m_nextArtistTextFormat);

  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_nextLabelTextFormat.Get());
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_nextTitleTextFormat.Get());
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_nextArtistTextFormat.Get());

  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_fallbackBlackBrush);
}

void NextTrackWidget::ReleaseResources() {
  m_nextLabelTextFormat.Reset();
  m_nextTitleTextFormat.Reset();
  m_nextArtistTextFormat.Reset();
  m_shadowBrush.Reset();
  m_textBrush.Reset();
  m_fallbackBlackBrush.Reset();
}

void NextTrackWidget::UpdateAnimation(const WidgetContext &ctx) {}
void NextTrackWidget::UpdateLayout(const WidgetContext &ctx,
                                   const ConfigManager *config) {}

void NextTrackWidget::Draw(ID2D1DeviceContext *context,
                           const WidgetContext &ctx,
                           const ConfigManager *config) {
  if (config && config->GetShowNextTrack() && config->GetEnableNextTrack() &&
      m_textBrush && m_nextTitleTextFormat && m_nextArtistTextFormat) {
    D2D1_SIZE_F renderTargetSize = context->GetSize();
    float logicWidth = renderTargetSize.width / ctx.dpiScale;
    float logicHeight = renderTargetSize.height / ctx.dpiScale;

    D2D1_SIZE_F bitmapSize = ctx.nextArtBitmap ? ctx.nextArtBitmap->GetSize()
                                               : D2D1::SizeF(0.0f, 0.0f);
    NextTrackLayout layout = LayoutCalculator::CalculateNextTrackLayout(
        logicWidth, logicHeight, config, bitmapSize);

    if (m_nextLabelTextFormat) {
      std::wstring labelText = L"Next Track";
      float shadowOpacity = config->GetEnableShadow() ? config->GetShadowOpacity() : 0.0f;
      WidgetCommon::DrawShadowedText(
          context, labelText, m_nextLabelTextFormat.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          layout.labelRect, layout.labelShadowRect, shadowOpacity);
    }

    if (!ctx.nextIsReady) {
      // 繝ｭ繝ｼ繝我ｸｭ
      if (m_fallbackBlackBrush) {
        m_fallbackBlackBrush->SetOpacity(config->GetNextFallbackArtOpacity());
        context->FillRectangle(&layout.fallbackArtRect,
                               m_fallbackBlackBrush.Get());
      }

      std::wstring loadingText = L"Loading...";
      float shadowOpacity = config->GetEnableShadow() ? config->GetShadowOpacity() : 0.0f;
      WidgetCommon::DrawShadowedText(
          context, loadingText, m_nextTitleTextFormat.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          layout.titleRect, layout.titleShadowRect, shadowOpacity);

    } else {
      if (ctx.nextArtBitmap) {
        if (m_shadowBrush && config->GetEnableShadow()) {
          m_shadowBrush->SetOpacity(config->GetShadowOpacity());
          context->FillRectangle(&layout.artShadowRect, m_shadowBrush.Get());
        }

        context->DrawBitmap(ctx.nextArtBitmap, &layout.artDestRect, 1.0f,
                            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
      } else {
        if (m_fallbackBlackBrush) {
          m_fallbackBlackBrush->SetOpacity(config->GetNextFallbackArtOpacity());
          context->FillRectangle(&layout.fallbackArtRect,
                                 m_fallbackBlackBrush.Get());
        }
      }

      std::wstring nextText = ctx.nextTrackTitle;
      float shadowOpacity = config->GetEnableShadow() ? config->GetShadowOpacity() : 0.0f;
      WidgetCommon::DrawShadowedText(
          context, nextText, m_nextTitleTextFormat.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          layout.titleRect, layout.titleShadowRect, shadowOpacity);

      WidgetCommon::DrawShadowedText(
          context, ctx.nextTrackArtist, m_nextArtistTextFormat.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          layout.artistRect, layout.artistShadowRect, shadowOpacity);
    }
  }
}
