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
      config->GetTitleFontFamily().c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetTitleFontSize(), L"ja-jp", &m_titleTextFormat);
  dwriteFactory->CreateTextFormat(
      config->GetArtistFontFamily().c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetArtistFontSize(), L"ja-jp", &m_artistTextFormat);

  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_titleTextFormat.Get());
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_artistTextFormat.Get());

  m_dwriteFactory = dwriteFactory;
  dwriteFactory->CreateTextFormat(
      config->GetTrackCountFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetTrackCountFontSize(), L"en-us",
      &m_trackCountTextFormat);
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_trackCountTextFormat.Get());

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
  m_trackCountTextFormat.Reset();
  m_trackCountTextLayout.Reset();
  m_dwriteFactory.Reset();
}

void TrackInfoWidget::UpdateAnimation(const WidgetContext &ctx) {}
void TrackInfoWidget::UpdateLayout(const WidgetContext &ctx,
                                   const ConfigManager *config) {
  if (!config)
    return;

  if (m_dwriteFactory) {
    if (m_titleTextFormat &&
        (!m_titleTextLayout || m_lastTitle != ctx.trackTitle)) {
      m_lastTitle = ctx.trackTitle;
      m_titleTextLayout.Reset();
      m_dwriteFactory->CreateTextLayout(
          ctx.trackTitle.c_str(), static_cast<UINT32>(ctx.trackTitle.length()),
          m_titleTextFormat.Get(), 4000.0f, 1000.0f, &m_titleTextLayout);
    }

    if (m_artistTextFormat &&
        (!m_artistTextLayout || m_lastArtist != ctx.trackArtist)) {
      m_lastArtist = ctx.trackArtist;
      m_artistTextLayout.Reset();
      m_dwriteFactory->CreateTextLayout(
          ctx.trackArtist.c_str(),
          static_cast<UINT32>(ctx.trackArtist.length()),
          m_artistTextFormat.Get(), 4000.0f, 1000.0f, &m_artistTextLayout);
    }
  }

  if (m_dwriteFactory && m_trackCountTextFormat &&
      (!m_trackCountTextLayout || m_lastTotalTracks != ctx.totalTracks ||
       m_lastCurrentTrackIndex != ctx.currentTrackIndex)) {
    m_lastTotalTracks = ctx.totalTracks;
    m_lastCurrentTrackIndex = ctx.currentTrackIndex;
    m_trackCountTextLayout.Reset();

    wchar_t trackCountBuf[64];
    if (ctx.totalTracks == 0) {
      swprintf_s(trackCountBuf, L"---/---");
    } else {
      swprintf_s(trackCountBuf, L"%zu/%zu", ctx.currentTrackIndex + 1,
                 ctx.totalTracks);
    }
    std::wstring trackCountStr(trackCountBuf);

    if (m_trackCountTextFormat) {
      int align = config->GetTrackCountTextAlignment();
      if (align == 0)
        m_trackCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      else if (align == 2)
        m_trackCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
      else
        m_trackCountTextFormat->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_TRAILING);
    }

    m_dwriteFactory->CreateTextLayout(
        trackCountStr.c_str(), static_cast<UINT32>(trackCountStr.length()),
        m_trackCountTextFormat.Get(), 200.0f, 50.0f, &m_trackCountTextLayout);

    if (m_trackCountTextLayout) {
      Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
      if (SUCCEEDED(m_trackCountTextLayout.As(&textLayout1))) {
        DWRITE_TEXT_RANGE textRange = {
            0, static_cast<UINT32>(trackCountStr.length())};
        textLayout1->SetCharacterSpacing(
            0.0f, config->GetTrackCountLetterSpacing(), 0.0f, textRange);
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

    D2D1_SIZE_F bitmapSize = ctx.currentArtBitmap
                                 ? ctx.currentArtBitmap->GetSize()
                                 : D2D1::SizeF(0.0f, 0.0f);
    TrackInfoLayout layout = LayoutCalculator::CalculateTrackInfoLayout(
        logicWidth, logicHeight, config, bitmapSize);

    if (ctx.currentArtBitmap) {
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->FillRectangle(&layout.artShadowRect, m_shadowBrush.Get());
      }

      context->DrawBitmap(ctx.currentArtBitmap, &layout.artDestRect, 1.0f,
                          D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    } else {
      if (m_fallbackBlackBrush) {
        m_fallbackBlackBrush->SetOpacity(config->GetFallbackArtOpacity());
        context->FillRectangle(&layout.fallbackArtRect,
                               m_fallbackBlackBrush.Get());
      }
    }

    if (m_textBrush && m_titleTextLayout && m_artistTextLayout) {
      m_titleTextLayout->SetMaxWidth(layout.titleRect.right -
                                     layout.titleRect.left);
      m_titleTextLayout->SetMaxHeight(layout.titleRect.bottom -
                                      layout.titleRect.top);
      m_artistTextLayout->SetMaxWidth(layout.artistRect.right -
                                      layout.artistRect.left);
      m_artistTextLayout->SetMaxHeight(layout.artistRect.bottom -
                                       layout.artistRect.top);

      float shadowOpacity = config->GetEnableShadow() ? config->GetShadowOpacity() : 0.0f;
      WidgetCommon::DrawShadowedTextLayout(
          context, m_titleTextLayout.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          D2D1::Point2F(layout.titleRect.left, layout.titleRect.top),
          D2D1::Point2F(layout.titleShadowRect.left, layout.titleShadowRect.top),
          shadowOpacity);

      WidgetCommon::DrawShadowedTextLayout(
          context, m_artistTextLayout.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          D2D1::Point2F(layout.artistRect.left, layout.artistRect.top),
          D2D1::Point2F(layout.artistShadowRect.left, layout.artistShadowRect.top),
          shadowOpacity);
    }

    if (m_trackCountTextLayout && m_textBrush) {
      float tcShadowOpacity = config->GetEnableShadow() ? config->GetTrackCountShadowOpacity() : 0.0f;
      WidgetCommon::DrawShadowedTextLayout(
          context, m_trackCountTextLayout.Get(), m_textBrush.Get(), m_shadowBrush.Get(),
          layout.trackCountOrigin, layout.trackCountShadowOrigin, tcShadowOpacity);
    }
  }
}
