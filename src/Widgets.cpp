#include "Widgets.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "resource.h"
#include <filesystem>

namespace {
bool LoadBitmapResourceHelper(IWICImagingFactory *wicFactory,
                              ID2D1DeviceContext *d2dContext,
                              const std::wstring &filename, int resourceId,
                              ID2D1Bitmap **ppBitmap) {
  HRESULT hr = S_OK;
  Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;

  hr = wicFactory->CreateDecoderFromFilename(
      filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
      &decoder);

  if (FAILED(hr)) {
    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC imageResHandle =
        FindResource(hModule, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!imageResHandle) {
      return false;
    }

    HGLOBAL imageResDataHandle = LoadResource(hModule, imageResHandle);
    if (!imageResDataHandle) {
      return false;
    }

    void *pImageFile = LockResource(imageResDataHandle);
    DWORD imageFileSize = SizeofResource(hModule, imageResHandle);
    if (!pImageFile || imageFileSize == 0) {
      return false;
    }

    Microsoft::WRL::ComPtr<IWICStream> stream;
    hr = wicFactory->CreateStream(&stream);
    if (FAILED(hr)) {
      return false;
    }

    hr = stream->InitializeFromMemory(reinterpret_cast<BYTE *>(pImageFile),
                                      imageFileSize);
    if (FAILED(hr)) {
      return false;
    }

    hr = wicFactory->CreateDecoderFromStream(
        stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) {
      return false;
    }
  }

  Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
  hr = decoder->GetFrame(0, &frame);
  if (FAILED(hr)) {
    return false;
  }

  Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
  hr = wicFactory->CreateFormatConverter(&converter);
  if (FAILED(hr)) {
    return false;
  }

  hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
                             WICBitmapDitherTypeNone, nullptr, 0.0f,
                             WICBitmapPaletteTypeMedianCut);
  if (FAILED(hr)) {
    return false;
  }

  hr =
      d2dContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, ppBitmap);
  return SUCCEEDED(hr);
}
} // namespace

// ==========================================
// AppLogoWidget
// ==========================================
void AppLogoWidget::CreateResources(ID2D1DeviceContext *context,
                                    IWICImagingFactory *wicFactory,
                                    IDWriteFactory *dwriteFactory,
                                    const ConfigManager *config) {
  LoadBitmapResourceHelper(wicFactory, context, L"app_logo.png", IDI_APP_LOGO,
                           &m_appLogoBitmap);
  LoadBitmapResourceHelper(wicFactory, context, L"app_logo_hover.png",
                           IDI_APP_LOGO_HOVER, &m_appLogoHoverBitmap);

  context->CreateEffect(CLSID_D2D1Shadow, &m_shadowEffect);
}

void AppLogoWidget::ReleaseResources() {
  m_appLogoBitmap.Reset();
  m_appLogoHoverBitmap.Reset();
  m_shadowEffect.Reset();
}

void AppLogoWidget::UpdateAnimation(const WidgetContext &ctx) {}
void AppLogoWidget::UpdateLayout(const WidgetContext &ctx,
                                 const ConfigManager *config) {}

void AppLogoWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                         const ConfigManager *config) {
  ID2D1Bitmap *bitmapToDraw = (ctx.isHovered || ctx.isLogoMenuHovered)
                                  ? m_appLogoHoverBitmap.Get()
                                  : m_appLogoBitmap.Get();
  if (config && config->GetShowAppLogo() && bitmapToDraw) {
    AppLogoLayout layout = LayoutCalculator::CalculateAppLogoLayout(config);

    if (m_shadowEffect && config->GetEnableShadow()) {
      m_shadowEffect->SetInput(0, bitmapToDraw);
      m_shadowEffect->SetValue(
          D2D1_SHADOW_PROP_COLOR,
          D2D1::Vector4F(0.0f, 0.0f, 0.0f, config->GetShadowOpacity()));
      context->DrawImage(m_shadowEffect.Get(), &layout.shadowOffset, nullptr,
                         D2D1_INTERPOLATION_MODE_LINEAR,
                         D2D1_COMPOSITE_MODE_SOURCE_OVER);
    }

    context->DrawBitmap(bitmapToDraw, &layout.destRect, 1.0f,
                        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
  }
}

// ==========================================
// LogoMenuWidget
// ==========================================
float CubicEaseOut(float t) {
  float f = (t - 1.0f);
  return f * f * f + 1.0f;
}

void LogoMenuWidget::CreateResources(ID2D1DeviceContext *context,
                                     IWICImagingFactory *wicFactory,
                                     IDWriteFactory *dwriteFactory,
                                     const ConfigManager *config) {
  m_dwriteFactory = dwriteFactory;

  if (config) {
    dwriteFactory->CreateTextFormat(
        config->GetLogoMenuFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetLogoMenuIconSize(), L"ja-jp",
        &m_iconTextFormat);
    if (m_iconTextFormat) {
      m_iconTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
      m_iconTextFormat->SetParagraphAlignment(
          DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    dwriteFactory->CreateTextFormat(
        config->GetLogoMenuTypingFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetLogoMenuTypingFontSize(),
        L"ja-jp", &m_typingTextFormat);
    if (m_typingTextFormat) {
      m_typingTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      m_typingTextFormat->SetParagraphAlignment(
          DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    dwriteFactory->CreateTextFormat(
        config->GetLogoMenuTypingFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetLogoMenuVisualizerFontSize(),
        L"ja-jp", &m_indicatorTextFormat);
    if (m_indicatorTextFormat) {
      m_indicatorTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      m_indicatorTextFormat->SetParagraphAlignment(
          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }

    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                   &m_iconBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.4f),
                                   &m_inactiveIconBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red),
                                   &m_lineBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                   &m_typingTextBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                   &m_shadowBrush);
  }
  LoadBitmapResourceHelper(wicFactory, context, L"app_logo_back.png",
                           IDI_APP_LOGO_BACK, &m_appLogoBackBitmap);
  context->CreateEffect(CLSID_D2D1Shadow, &m_shadowEffect);
}

void LogoMenuWidget::ReleaseResources() {
  m_iconTextFormat.Reset();
  m_typingTextFormat.Reset();
  m_indicatorTextFormat.Reset();
  m_iconBrush.Reset();
  m_inactiveIconBrush.Reset();
  m_lineBrush.Reset();
  m_typingTextBrush.Reset();
  m_appLogoBackBitmap.Reset();
  m_shadowEffect.Reset();
  m_dwriteFactory.Reset();
}

void LogoMenuWidget::UpdateAnimation(const WidgetContext &ctx) {
  float duration = ctx.config ? ctx.config->GetLogoMenuScrollDuration() : 0.5f;
  float speed = (duration > 0.0f) ? (1.0f / duration) : 2.0f;
  if (ctx.isLogoMenuHovered || ctx.isHovered) {
    m_menuProgress += ctx.deltaTime * speed;
    if (m_menuProgress > 1.0f)
      m_menuProgress = 1.0f;
  } else {
    m_menuProgress -= ctx.deltaTime * speed;
    if (m_menuProgress < 0.0f)
      m_menuProgress = 0.0f;
  }
}

void LogoMenuWidget::UpdateLayout(const WidgetContext &ctx,
                                  const ConfigManager *config) {}

void LogoMenuWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                          const ConfigManager *config) {
  if (!config || m_menuProgress <= 0.0f || !ctx.logoMenuItems)
    return;

  float easedProgress = CubicEaseOut(m_menuProgress);

  LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
      config, easedProgress, ctx.logoMenuItems->size());

  D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE;

  for (size_t i = 0; i < layout.items.size(); ++i) {
    const auto &item = (*ctx.logoMenuItems)[i];
    const auto &itemLayout = layout.items[i];

    bool active = true;
    if (item.commandId == Window::ID_LOGO_VISUALIZER) {
      active = (config->GetVisualizerMode() != 0);
    } else if (item.isToggle && !item.toggleState) {
      active = false;
    }

    if (i == ctx.logoMenuHoveredIndex && m_appLogoBackBitmap) {
      D2D1_SIZE_F bitmapSize = m_appLogoBackBitmap->GetSize();
      float logoScale = 1.0f; // Scale if needed, but let's assume it fits well.
      float bgWidth = bitmapSize.width * logoScale;
      float bgHeight = bitmapSize.height * logoScale;
      D2D1_RECT_F bgRect = D2D1::RectF(itemLayout.position.x - bgWidth / 2.0f,
                                       itemLayout.position.y - bgHeight / 2.0f,
                                       itemLayout.position.x + bgWidth / 2.0f,
                                       itemLayout.position.y + bgHeight / 2.0f);

      if (m_shadowEffect) {
        m_shadowEffect->SetInput(0, m_appLogoBackBitmap.Get());
        m_shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR,
                                 D2D1::Vector4F(1.0f, 1.0f, 1.0f, 0.8f));
        m_shadowEffect->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION,
                                 5.0f);
        D2D1_POINT_2F offset =
            D2D1::Point2F(itemLayout.position.x - bgWidth / 2.0f,
                          itemLayout.position.y - bgHeight / 2.0f);
        context->DrawImage(m_shadowEffect.Get(), &offset, nullptr,
                           D2D1_INTERPOLATION_MODE_LINEAR,
                           D2D1_COMPOSITE_MODE_SOURCE_OVER);
      }
      context->DrawBitmap(m_appLogoBackBitmap.Get(), &bgRect,
                          config->GetLogoMenuIconHoverBgAlpha(),
                          D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    }

    ID2D1SolidColorBrush *brush =
        active ? m_iconBrush.Get() : m_inactiveIconBrush.Get();

    context->DrawText(
        item.iconText.c_str(), static_cast<UINT32>(item.iconText.length()),
        m_iconTextFormat.Get(), itemLayout.hitRect, brush, options);

    if (!active) {
      float cx = itemLayout.position.x;
      float cy = itemLayout.position.y;
      float strikeLength = config->GetLogoMenuStrikeLength();
      float strikeThickness = config->GetLogoMenuStrikeThickness();
      float dx = strikeLength * 0.70710678f / 2.0f;
      float dy = strikeLength * 0.70710678f / 2.0f;

      context->DrawLine(D2D1::Point2F(cx - dx, cy + dy),
                        D2D1::Point2F(cx + dx, cy - dy), m_lineBrush.Get(),
                        strikeThickness);
    }

    if (item.commandId == Window::ID_LOGO_VISUALIZER) {
      int mode = config->GetVisualizerMode();
      if (mode == 1 || mode == 2) {
        std::wstring modeStr = mode == 1 ? L"1" : L"2";
        D2D1_RECT_F indRect = itemLayout.hitRect;
        indRect.left =
            itemLayout.position.x + config->GetLogoMenuVisualizerIconOffsetX();
        indRect.top =
            itemLayout.position.y + config->GetLogoMenuVisualizerIconOffsetY();

        // Create a temporary layout for shadow/outline effect
        if (m_dwriteFactory && m_indicatorTextFormat) {
          Microsoft::WRL::ComPtr<IDWriteTextLayout> modeLayout;
          m_dwriteFactory->CreateTextLayout(modeStr.c_str(), 1,
                                            m_indicatorTextFormat.Get(), 50.0f,
                                            50.0f, &modeLayout);
          if (modeLayout) {
            modeLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            modeLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
            context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                           &blackBrush);

            // Black outline
            if (blackBrush) {
              context->DrawTextLayout(
                  D2D1::Point2F(indRect.left - 1.0f, indRect.top),
                  modeLayout.Get(), blackBrush.Get());
              context->DrawTextLayout(
                  D2D1::Point2F(indRect.left + 1.0f, indRect.top),
                  modeLayout.Get(), blackBrush.Get());
              context->DrawTextLayout(
                  D2D1::Point2F(indRect.left, indRect.top - 1.0f),
                  modeLayout.Get(), blackBrush.Get());
              context->DrawTextLayout(
                  D2D1::Point2F(indRect.left, indRect.top + 1.0f),
                  modeLayout.Get(), blackBrush.Get());
            }

            // White body
            context->DrawTextLayout(D2D1::Point2F(indRect.left, indRect.top),
                                    modeLayout.Get(), m_iconBrush.Get());
          }
        }
      }
    }
  }

  std::wstring textToDraw = L"Beyond the r'ai'nbow awaits the game you "
                            L"envisioned in your lullaby days.";
  if (ctx.logoMenuHoveredIndex >= 0 &&
      ctx.logoMenuHoveredIndex < ctx.logoMenuItems->size()) {
    const auto &hoveredItem = (*ctx.logoMenuItems)[ctx.logoMenuHoveredIndex];
    if (hoveredItem.commandId == Window::ID_LOGO_VISUALIZER) {
      int mode = config->GetVisualizerMode();
      if (mode == 1)
        textToDraw = L"ビジュアライザ: PRISM BEAT";
      else if (mode == 2)
        textToDraw = L"ビジュアライザ: HALO DUST";
      else
        textToDraw = L"ビジュアライザ: OFF";
    } else if (hoveredItem.commandId == Window::ID_LOGO_PLAYLIST_POS) {
      if (config->GetPlaylistPosition() == 0) {
        textToDraw = L"プレイリスト配置: 画面左";
      } else {
        textToDraw = L"プレイリスト配置: 画面右";
      }
    } else {
      textToDraw = hoveredItem.labelText;
    }
  }

  float letterSpacing = config->GetLogoMenuTypingLetterSpacing();
  if (letterSpacing != 0.0f && m_dwriteFactory) {
    Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
    HRESULT hr = m_dwriteFactory->CreateTextLayout(
        textToDraw.c_str(), static_cast<UINT32>(textToDraw.length()),
        m_typingTextFormat.Get(),
        layout.typingTextRect.right - layout.typingTextRect.left,
        layout.typingTextRect.bottom - layout.typingTextRect.top, &textLayout);

    if (SUCCEEDED(hr)) {
      Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
      if (SUCCEEDED(textLayout.As(&textLayout1))) {
        DWRITE_TEXT_RANGE textRange = {
            0, static_cast<UINT32>(textToDraw.length())};
        textLayout1->SetCharacterSpacing(0.0f, letterSpacing, 0.0f, textRange);
      }

      if (m_shadowBrush) {
        m_shadowBrush->SetOpacity(config->GetLogoMenuDescShadowOpacity());
        context->DrawTextLayout(
            D2D1::Point2F(layout.typingTextRect.left +
                              config->GetLogoMenuDescShadowOffsetX(),
                          layout.typingTextRect.top +
                              config->GetLogoMenuDescShadowOffsetY()),
            textLayout.Get(), m_shadowBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE);
      }
      context->DrawTextLayout(
          D2D1::Point2F(layout.typingTextRect.left, layout.typingTextRect.top),
          textLayout.Get(), m_typingTextBrush.Get(),
          D2D1_DRAW_TEXT_OPTIONS_NONE);
    }
  } else {
    if (m_shadowBrush) {
      m_shadowBrush->SetOpacity(config->GetLogoMenuDescShadowOpacity());
      D2D1_RECT_F shadowRect = layout.typingTextRect;
      shadowRect.left += config->GetLogoMenuDescShadowOffsetX();
      shadowRect.right += config->GetLogoMenuDescShadowOffsetX();
      shadowRect.top += config->GetLogoMenuDescShadowOffsetY();
      shadowRect.bottom += config->GetLogoMenuDescShadowOffsetY();
      context->DrawText(textToDraw.c_str(),
                        static_cast<UINT32>(textToDraw.length()),
                        m_typingTextFormat.Get(), shadowRect,
                        m_shadowBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE);
    }
    context->DrawText(textToDraw.c_str(),
                      static_cast<UINT32>(textToDraw.length()),
                      m_typingTextFormat.Get(), layout.typingTextRect,
                      m_typingTextBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE);
  }
}

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

  auto ApplyTrimming = [&](Microsoft::WRL::ComPtr<IDWriteTextFormat> &format) {
    if (!format)
      return;
    DWRITE_TRIMMING trimmingOptions = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0,
                                       0};
    Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
    if (SUCCEEDED(dwriteFactory->CreateEllipsisTrimmingSign(format.Get(),
                                                            &ellipsis))) {
      format->SetTrimming(&trimmingOptions, ellipsis.Get());
      format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    }
  };
  ApplyTrimming(m_titleTextFormat);
  ApplyTrimming(m_artistTextFormat);

  m_dwriteFactory = dwriteFactory;
  dwriteFactory->CreateTextFormat(
      config->GetTrackCountFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetTrackCountFontSize(), L"en-us",
      &m_trackCountTextFormat);
  ApplyTrimming(m_trackCountTextFormat);

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

      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->DrawTextLayout(D2D1::Point2F(layout.titleShadowRect.left,
                                              layout.titleShadowRect.top),
                                m_titleTextLayout.Get(), m_shadowBrush.Get());
      }

      context->DrawTextLayout(
          D2D1::Point2F(layout.titleRect.left, layout.titleRect.top),
          m_titleTextLayout.Get(), m_textBrush.Get());

      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->DrawTextLayout(D2D1::Point2F(layout.artistShadowRect.left,
                                              layout.artistShadowRect.top),
                                m_artistTextLayout.Get(), m_shadowBrush.Get());
      }

      context->DrawTextLayout(
          D2D1::Point2F(layout.artistRect.left, layout.artistRect.top),
          m_artistTextLayout.Get(), m_textBrush.Get());
    }

    if (m_trackCountTextLayout && m_textBrush) {
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetTrackCountShadowOpacity());
        context->DrawTextLayout(layout.trackCountShadowOrigin,
                                m_trackCountTextLayout.Get(),
                                m_shadowBrush.Get());
      }
      context->DrawTextLayout(layout.trackCountOrigin,
                              m_trackCountTextLayout.Get(), m_textBrush.Get());
    }
  }
}

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

  auto ApplyTrimming = [&](Microsoft::WRL::ComPtr<IDWriteTextFormat> &format) {
    if (!format)
      return;
    DWRITE_TRIMMING trimmingOptions = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0,
                                       0};
    Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
    if (SUCCEEDED(dwriteFactory->CreateEllipsisTrimmingSign(format.Get(),
                                                            &ellipsis))) {
      format->SetTrimming(&trimmingOptions, ellipsis.Get());
      format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    }
  };
  ApplyTrimming(m_nextLabelTextFormat);
  ApplyTrimming(m_nextTitleTextFormat);
  ApplyTrimming(m_nextArtistTextFormat);

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
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->DrawText(labelText.c_str(),
                          static_cast<UINT32>(labelText.length()),
                          m_nextLabelTextFormat.Get(), &layout.labelShadowRect,
                          m_shadowBrush.Get());
      }

      context->DrawText(
          labelText.c_str(), static_cast<UINT32>(labelText.length()),
          m_nextLabelTextFormat.Get(), &layout.labelRect, m_textBrush.Get());
    }

    if (!ctx.nextIsReady) {
      // 繝ｭ繝ｼ繝我ｸｭ
      if (m_fallbackBlackBrush) {
        m_fallbackBlackBrush->SetOpacity(config->GetNextFallbackArtOpacity());
        context->FillRectangle(&layout.fallbackArtRect,
                               m_fallbackBlackBrush.Get());
      }

      std::wstring loadingText = L"Loading...";
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->DrawText(loadingText.c_str(),
                          static_cast<UINT32>(loadingText.length()),
                          m_nextTitleTextFormat.Get(), &layout.titleShadowRect,
                          m_shadowBrush.Get());
      }

      context->DrawText(
          loadingText.c_str(), static_cast<UINT32>(loadingText.length()),
          m_nextTitleTextFormat.Get(), &layout.titleRect, m_textBrush.Get());

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
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->DrawText(nextText.c_str(),
                          static_cast<UINT32>(nextText.length()),
                          m_nextTitleTextFormat.Get(), &layout.titleShadowRect,
                          m_shadowBrush.Get());
      }

      context->DrawText(
          nextText.c_str(), static_cast<UINT32>(nextText.length()),
          m_nextTitleTextFormat.Get(), &layout.titleRect, m_textBrush.Get());

      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        context->DrawText(ctx.nextTrackArtist.c_str(),
                          static_cast<UINT32>(ctx.nextTrackArtist.length()),
                          m_nextArtistTextFormat.Get(),
                          &layout.artistShadowRect, m_shadowBrush.Get());
      }

      context->DrawText(ctx.nextTrackArtist.c_str(),
                        static_cast<UINT32>(ctx.nextTrackArtist.length()),
                        m_nextArtistTextFormat.Get(), &layout.artistRect,
                        m_textBrush.Get());
    }
  }
}
void SeekBarWidget::CreateResources(ID2D1DeviceContext *context,
                                    IWICImagingFactory *wicFactory,
                                    IDWriteFactory *dwriteFactory,
                                    const ConfigManager *config) {
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_seekBarBgBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_seekBarFgBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_textBrush);

  m_dwriteFactory = dwriteFactory;

  if (config) {
    dwriteFactory->CreateTextFormat(
        config->GetSeekBarTimeFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetSeekBarTimeFontSize(), L"ja-jp",
        &m_timeTextFormat);
    if (m_timeTextFormat) {
      m_timeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
      m_timeTextFormat->SetParagraphAlignment(
          DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
  }
}

void SeekBarWidget::ReleaseResources() {
  m_seekBarBgBrush.Reset();
  m_seekBarFgBrush.Reset();
  m_textBrush.Reset();
  m_timeTextFormat.Reset();
  m_timeTextLayout.Reset();
  m_dwriteFactory.Reset();
}

void SeekBarWidget::UpdateAnimation(const WidgetContext &ctx) {}

void SeekBarWidget::UpdateLayout(const WidgetContext &ctx,
                                 const ConfigManager *config) {}

void SeekBarWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                         const ConfigManager *config) {
  if (!config || !config->GetShowSeekBar() || !m_textBrush || !m_timeTextFormat)
    return;

  D2D1_SIZE_F rtSize = context->GetSize();
  float logicWidth = rtSize.width / ctx.dpiScale;
  float logicHeight = rtSize.height / ctx.dpiScale;

  SeekBarLayout layout = LayoutCalculator::CalculateSeekBarLayout(
      logicWidth, logicHeight, config, ctx.progress);

  bool rebuildTime = !m_timeTextLayout || (m_lastTimeString != ctx.timeString);
  m_lastTimeString = ctx.timeString;
  if (rebuildTime && m_dwriteFactory && m_timeTextFormat) {
    m_timeTextLayout.Reset();
    m_dwriteFactory->CreateTextLayout(
        ctx.timeString.c_str(), static_cast<UINT32>(ctx.timeString.length()),
        m_timeTextFormat.Get(), layout.textMaxWidth, layout.textMaxHeight,
        &m_timeTextLayout);
    if (m_timeTextLayout) {
      Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
      if (SUCCEEDED(m_timeTextLayout.As(&textLayout1))) {
        DWRITE_TEXT_RANGE textRange = {
            0, static_cast<UINT32>(ctx.timeString.length())};
        textLayout1->SetCharacterSpacing(
            0.0f, config->GetSeekBarTimeLetterSpacing(), 0.0f, textRange);
      }
    }
  }

  float dimFactor = 1.0f - (ctx.controlAlpha * 0.5f);

  if (m_seekBarBgBrush) {
    m_seekBarBgBrush->SetOpacity(config->GetSeekBarBgOpacity() * dimFactor);
    context->FillRectangle(&layout.bgRect, m_seekBarBgBrush.Get());
  }

  if (m_seekBarFgBrush) {
    m_seekBarFgBrush->SetOpacity(dimFactor);
    context->FillRectangle(&layout.fgRect, m_seekBarFgBrush.Get());
  }

  if (m_timeTextLayout) {
    context->DrawTextLayout(layout.textOrigin, m_timeTextLayout.Get(),
                            m_seekBarFgBrush ? m_seekBarFgBrush.Get()
                                             : m_textBrush.Get(),
                            D2D1_DRAW_TEXT_OPTIONS_NONE);
  }
}

void PlaybackControlsWidget::CreateResources(ID2D1DeviceContext *context,
                                             IWICImagingFactory *wicFactory,
                                             IDWriteFactory *dwriteFactory,
                                             const ConfigManager *config) {
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_controlBrush);

  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
  context->GetFactory(&d2dFactory);

  if (d2dFactory) {
    d2dFactory->CreatePathGeometry(&m_playIconGeometry);
    if (m_playIconGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_playIconGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      sink->BeginFigure(D2D1::Point2F(-0.5f, -0.5f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(0.5f, 0.0f));
      sink->AddLine(D2D1::Point2F(-0.5f, 0.5f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }

    d2dFactory->CreatePathGeometry(&m_prevIconGeometry);
    if (m_prevIconGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_prevIconGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      sink->BeginFigure(D2D1::Point2F(0.5f, -0.5f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(-0.5f, 0.0f));
      sink->AddLine(D2D1::Point2F(0.5f, 0.5f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }
  }
}

void PlaybackControlsWidget::ReleaseResources() {
  m_controlBrush.Reset();
  m_playIconGeometry.Reset();
  m_prevIconGeometry.Reset();
}

void PlaybackControlsWidget::UpdateAnimation(const WidgetContext &ctx) {}
void PlaybackControlsWidget::UpdateLayout(const WidgetContext &ctx,
                                          const ConfigManager *config) {}

void PlaybackControlsWidget::Draw(ID2D1DeviceContext *context,
                                  const WidgetContext &ctx,
                                  const ConfigManager *config) {
  if (ctx.controlAlpha <= 0.0f || !config || !config->GetShowPlaybackControls())
    return;

  D2D1_SIZE_F renderTargetSize = context->GetSize();
  float logicWidth = renderTargetSize.width / ctx.dpiScale;
  float logicHeight = renderTargetSize.height / ctx.dpiScale;

  PlaybackControlsLayout layout =
      LayoutCalculator::CalculatePlaybackControlsLayout(logicWidth, logicHeight,
                                                        config);

  if (m_controlBrush) {
    m_controlBrush->SetOpacity(ctx.controlAlpha);
    auto DrawTriangle = [&](float cx, float cy, float w, float h, bool right) {
      D2D1_MATRIX_3X2_F oldTransform;
      context->GetTransform(&oldTransform);
      context->SetTransform(D2D1::Matrix3x2F::Scale(w, h) *
                            D2D1::Matrix3x2F::Translation(cx, cy) *
                            oldTransform);
      context->FillGeometry(right ? m_playIconGeometry.Get()
                                  : m_prevIconGeometry.Get(),
                            m_controlBrush.Get());
      context->SetTransform(oldTransform);
    };

    auto DrawRect = [&](float cx, float cy, float w, float h) {
      D2D1_RECT_F rect =
          D2D1::RectF(cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2);
      context->FillRectangle(rect, m_controlBrush.Get());
    };

    float prevX = layout.centerX - layout.spacing;
    DrawRect(prevX - layout.half + layout.size * 0.1f, layout.centerY,
             layout.size * 0.2f, layout.size);
    DrawTriangle(prevX - layout.size * 0.1f, layout.centerY, layout.size * 0.4f,
                 layout.size, false);
    DrawTriangle(prevX + layout.size * 0.3f, layout.centerY, layout.size * 0.4f,
                 layout.size, false);

    if (ctx.isPlaying) {
      DrawRect(layout.centerX - layout.size * 0.2f, layout.centerY,
               layout.size * 0.3f, layout.size);
      DrawRect(layout.centerX + layout.size * 0.2f, layout.centerY,
               layout.size * 0.3f, layout.size);
    } else {
      DrawTriangle(layout.centerX, layout.centerY, layout.size, layout.size,
                   true);
    }

    float nextX = layout.centerX + layout.spacing;
    DrawTriangle(nextX - layout.size * 0.3f, layout.centerY, layout.size * 0.4f,
                 layout.size, true);
    DrawTriangle(nextX + layout.size * 0.1f, layout.centerY, layout.size * 0.4f,
                 layout.size, true);
    DrawRect(nextX + layout.half - layout.size * 0.1f, layout.centerY,
             layout.size * 0.2f, layout.size);
  }
}

void VolumeControlWidget::CreateResources(ID2D1DeviceContext *context,
                                          IWICImagingFactory *wicFactory,
                                          IDWriteFactory *dwriteFactory,
                                          const ConfigManager *config) {
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_controlBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);

  m_dwriteFactory = dwriteFactory;

  if (config) {
    dwriteFactory->CreateTextFormat(
        config->GetVolumeFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetVolumeFontSize(), L"ja-jp",
        &m_volumeTextFormat);
    if (m_volumeTextFormat) {
      // Text alignment setup removed to fix text positioning issue
    }
  }

  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
  context->GetFactory(&d2dFactory);

  if (d2dFactory) {
    d2dFactory->CreatePathGeometry(&m_speakerIconGeometry);
    if (m_speakerIconGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_speakerIconGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      sink->BeginFigure(D2D1::Point2F(0.0f, -0.2f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(0.4f, -0.2f));
      sink->AddLine(D2D1::Point2F(0.4f, 0.2f));
      sink->AddLine(D2D1::Point2F(0.0f, 0.2f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->BeginFigure(D2D1::Point2F(0.4f, -0.2f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(0.8f, -0.5f));
      sink->AddLine(D2D1::Point2F(0.8f, 0.5f));
      sink->AddLine(D2D1::Point2F(0.4f, 0.2f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }
  }
}

void VolumeControlWidget::ReleaseResources() {
  m_controlBrush.Reset();
  m_shadowBrush.Reset();
  m_speakerIconGeometry.Reset();
  m_volumeTextFormat.Reset();
  m_volTextLayout.Reset();
  m_dwriteFactory.Reset();
}

void VolumeControlWidget::UpdateAnimation(const WidgetContext &ctx) {}
void VolumeControlWidget::UpdateLayout(const WidgetContext &ctx,
                                       const ConfigManager *config) {}

void VolumeControlWidget::Draw(ID2D1DeviceContext *context,
                               const WidgetContext &ctx,
                               const ConfigManager *config) {
  if (ctx.controlAlpha <= 0.0f || !config || !config->GetShowVolumeControl())
    return;

  D2D1_SIZE_F renderTargetSize = context->GetSize();
  float logicWidth = renderTargetSize.width / ctx.dpiScale;
  float logicHeight = renderTargetSize.height / ctx.dpiScale;

  VolumeControlLayout layout = LayoutCalculator::CalculateVolumeControlLayout(
      logicWidth, logicHeight, config);

  bool rebuildVolume = !m_volTextLayout || (m_lastVolume != ctx.volume);
  m_lastVolume = ctx.volume;
  if (rebuildVolume && m_dwriteFactory && m_volumeTextFormat) {
    int volPercent = static_cast<int>(ctx.volume * 100.0f + 0.5f);
    wchar_t volBuf[16];
    swprintf_s(volBuf, L"%d%%", volPercent);

    m_volTextLayout.Reset();
    float letterSpacing = config->GetVolumeTextLetterSpacing();
    m_dwriteFactory->CreateTextLayout(
        volBuf, static_cast<UINT32>(wcslen(volBuf)), m_volumeTextFormat.Get(),
        layout.textMaxWidth, layout.textMaxHeight, &m_volTextLayout);
    if (m_volTextLayout && letterSpacing != 0.0f) {
      Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
      if (SUCCEEDED(m_volTextLayout.As(&textLayout1))) {
        DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(wcslen(volBuf))};
        textLayout1->SetCharacterSpacing(0.0f, letterSpacing, 0.0f, textRange);
      }
    }
  }

  if (m_controlBrush) {
    m_controlBrush->SetOpacity(ctx.controlAlpha);

    D2D1::Matrix3x2F oldTransform;
    context->GetTransform(&oldTransform);
    D2D1_MATRIX_3X2_F spkTransform =
        D2D1::Matrix3x2F::Scale(layout.volSize, layout.volSize) *
        D2D1::Matrix3x2F::Translation(layout.volX, layout.volY);

    if (m_shadowBrush && config->GetVolumeEnableShadow()) {
      m_shadowBrush->SetOpacity(config->GetVolumeShadowOpacity() *
                                ctx.controlAlpha);

      context->SetTransform(
          spkTransform * oldTransform *
          D2D1::Matrix3x2F::Translation(layout.shadowX, layout.shadowY));
      context->FillGeometry(m_speakerIconGeometry.Get(), m_shadowBrush.Get());

      context->SetTransform(oldTransform * D2D1::Matrix3x2F::Translation(
                                               layout.shadowX, layout.shadowY));

      if (ctx.volume > 0.0f) {
        float arcX = layout.volX + layout.spkW + layout.spkConeW + 4.0f;
        context->DrawLine(
            D2D1::Point2F(arcX, layout.volY - layout.volSize * 0.2f),
            D2D1::Point2F(arcX, layout.volY + layout.volSize * 0.2f),
            m_shadowBrush.Get(), 2.0f);
      }
      if (ctx.volume > 0.5f) {
        float arcX = layout.volX + layout.spkW + layout.spkConeW + 8.0f;
        context->DrawLine(
            D2D1::Point2F(arcX, layout.volY - layout.volSize * 0.35f),
            D2D1::Point2F(arcX, layout.volY + layout.volSize * 0.35f),
            m_shadowBrush.Get(), 2.0f);
      }

      context->SetTransform(oldTransform);

      if (m_volTextLayout) {
        context->DrawTextLayout(D2D1::Point2F(layout.textX + layout.shadowX,
                                              layout.textY + layout.shadowY),
                                m_volTextLayout.Get(), m_shadowBrush.Get());
      }
    }

    context->SetTransform(spkTransform * oldTransform);
    context->FillGeometry(m_speakerIconGeometry.Get(), m_controlBrush.Get());

    context->SetTransform(oldTransform);

    if (ctx.volume > 0.0f) {
      float arcX = layout.volX + layout.spkW + layout.spkConeW + 4.0f;
      context->DrawLine(
          D2D1::Point2F(arcX, layout.volY - layout.volSize * 0.2f),
          D2D1::Point2F(arcX, layout.volY + layout.volSize * 0.2f),
          m_controlBrush.Get(), 2.0f);
    }
    if (ctx.volume > 0.5f) {
      float arcX = layout.volX + layout.spkW + layout.spkConeW + 8.0f;
      context->DrawLine(
          D2D1::Point2F(arcX, layout.volY - layout.volSize * 0.35f),
          D2D1::Point2F(arcX, layout.volY + layout.volSize * 0.35f),
          m_controlBrush.Get(), 2.0f);
    }

    if (m_volTextLayout) {
      context->DrawTextLayout(D2D1::Point2F(layout.textX, layout.textY),
                              m_volTextLayout.Get(), m_controlBrush.Get());
    }
  }
}

// ================= PlaylistWidget =================

PlaylistWidget::PlaylistWidget()
    : m_playlistSlideX(9999.0f), m_playlistManualScrollY(0.0f) {}

void PlaylistWidget::CreateResources(ID2D1DeviceContext *context,
                                     IWICImagingFactory *wicFactory,
                                     IDWriteFactory *dwriteFactory,
                                     const ConfigManager *config) {
  if (!config)
    return;
  m_dwriteFactory = dwriteFactory;

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistTitleFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistTitleFontSize(), L"ja-jp",
      &m_playlistTitleTextFormat);

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistArtistFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistArtistFontSize(), L"ja-jp",
      &m_playlistArtistTextFormat);

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistTimeFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistTimeFontSize(), L"en-us",
      &m_playlistTimeTextFormat);

  auto ApplyTrimming = [&](Microsoft::WRL::ComPtr<IDWriteTextFormat> &format) {
    if (!format)
      return;
    DWRITE_TRIMMING trimmingOptions = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0,
                                       0};
    Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
    if (SUCCEEDED(dwriteFactory->CreateEllipsisTrimmingSign(format.Get(),
                                                            &ellipsis))) {
      format->SetTrimming(&trimmingOptions, ellipsis.Get());
      format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    }
  };
  ApplyTrimming(m_playlistTitleTextFormat);
  ApplyTrimming(m_playlistArtistTextFormat);
  ApplyTrimming(m_playlistTimeTextFormat);

  if (m_playlistTimeTextFormat)
    m_playlistTimeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistTitleFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistToolbarTextFontSize(),
      L"ja-jp", &m_toolbarTextFormat);
  if (m_toolbarTextFormat) {
    m_toolbarTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_toolbarTextFormat->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      L"Segoe UI Emoji", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetPlaylistToolbarIconSize(), L"ja-jp", &m_toolbarIconFormat);
  if (m_toolbarIconFormat) {
    m_toolbarIconFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_toolbarIconFormat->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  auto ParseHexColor = [](const std::wstring &hexColor) -> D2D1_COLOR_F {
    if (hexColor.empty() || hexColor[0] != L'#')
      return D2D1::ColorF(D2D1::ColorF::White);
    try {
      unsigned int hexValue = std::stoul(hexColor.substr(1), nullptr, 16);
      if (hexColor.length() == 7) {
        return D2D1::ColorF(((hexValue >> 16) & 0xFF) / 255.0f,
                            ((hexValue >> 8) & 0xFF) / 255.0f,
                            (hexValue & 0xFF) / 255.0f);
      }
    } catch (...) {
    }
    return D2D1::ColorF(D2D1::ColorF::White);
  };

  context->CreateSolidColorBrush(
      ParseHexColor(config->GetPlaylistArtistColor()), &m_playlistArtistBrush);
  context->CreateSolidColorBrush(ParseHexColor(config->GetPlaylistTimeColor()),
                                 &m_playlistTimeBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_playlistBgBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_playlistHighlightBrush);
  context->CreateSolidColorBrush(
      ParseHexColor(config->GetPlaylistGripLineColor()),
      &m_playlistGripLineBrush);
  context->CreateSolidColorBrush(
      ParseHexColor(config->GetPlaylistGripArrowColor()),
      &m_playlistGripArrowBrush);

  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
  context->GetFactory(&d2dFactory);

  if (d2dFactory) {
    d2dFactory->CreatePathGeometry(&m_playlistGripArrowGeometry);
    if (m_playlistGripArrowGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      if (SUCCEEDED(m_playlistGripArrowGeometry->Open(&sink))) {
        float width = config->GetPlaylistGripArrowWidth();
        float height = config->GetPlaylistGripArrowHeight();
        sink->BeginFigure(D2D1::Point2F(0.0f, -height / 2.0f),
                          D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(-width, 0.0f));
        sink->AddLine(D2D1::Point2F(0.0f, height / 2.0f));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
      }
    }

    d2dFactory->CreatePathGeometry(&m_playlistGripArrowRightGeometry);
    if (m_playlistGripArrowRightGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      if (SUCCEEDED(m_playlistGripArrowRightGeometry->Open(&sink))) {
        float width = config->GetPlaylistGripArrowWidth();
        float height = config->GetPlaylistGripArrowHeight();
        sink->BeginFigure(D2D1::Point2F(0.0f, -height / 2.0f),
                          D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(width, 0.0f));
        sink->AddLine(D2D1::Point2F(0.0f, height / 2.0f));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
      }
    }
  }
}

void PlaylistWidget::ReleaseResources() {
  m_playlistTitleTextFormat.Reset();
  m_playlistArtistTextFormat.Reset();
  m_playlistTimeTextFormat.Reset();
  m_toolbarTextFormat.Reset();
  m_toolbarIconFormat.Reset();
  m_playlistArtistBrush.Reset();
  m_playlistTimeBrush.Reset();
  m_playlistGripLineBrush.Reset();
  m_playlistGripArrowBrush.Reset();
  m_playlistGripArrowGeometry.Reset();
  m_playlistGripArrowRightGeometry.Reset();
  m_playlistBgBrush.Reset();
  m_playlistHighlightBrush.Reset();
  m_textBrush.Reset();
  m_shadowBrush.Reset();
  m_dwriteFactory.Reset();
}

void PlaylistWidget::UpdateAnimation(const WidgetContext &ctx) {
  if (!ctx.config)
    return;

  float configPlaylistWidth =
      static_cast<float>(ctx.config->GetPlaylistWidth());
  if (m_playlistSlideX > configPlaylistWidth * 2.0f)
    m_playlistSlideX = configPlaylistWidth;

  float targetSlideX = ctx.isPlaylistHovered ? 0.0f : configPlaylistWidth;
  m_playlistSlideX += (targetSlideX - m_playlistSlideX) * 0.2f;

  if (!ctx.isPlaylistHovered) {
    m_playlistManualScrollY = 0.0f;
  } else {
    D2D1_SIZE_F renderTargetSize =
        D2D1::SizeF(ctx.config->GetWindowWidth() * ctx.dpiScale,
                    ctx.config->GetWindowHeight() * ctx.dpiScale);
    float logicWidth = renderTargetSize.width / ctx.dpiScale;
    float logicHeight = renderTargetSize.height / ctx.dpiScale;

    size_t activeIndex = ctx.currentTrackIndex;
    size_t activeTotal = ctx.totalTracks;

    if (ctx.isPlaylistListViewMode && ctx.config) {
      std::vector<std::wstring> playlists = ctx.config->GetAvailablePlaylists();
      activeTotal = playlists.size();
      std::wstring currentPlaylist = ctx.config->GetDefaultPlaylistPath();
      activeIndex = 0;
      for (size_t i = 0; i < playlists.size(); ++i) {
        if (playlists[i] == currentPlaylist) {
          activeIndex = i;
          break;
        }
      }
    }

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicWidth, logicHeight, ctx.config, m_playlistSlideX,
        m_playlistManualScrollY, activeIndex, activeTotal);
    m_playlistManualScrollY = layout.newManualScrollY;
  }
}

void PlaylistWidget::UpdateLayout(const WidgetContext &ctx,
                                  const ConfigManager *config) {}

void PlaylistWidget::AddScroll(float delta) {
  m_playlistManualScrollY += delta;
}

float PlaylistWidget::GetScrollY() const { return m_playlistManualScrollY; }

void PlaylistWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                          const ConfigManager *config) {
  if (!config || !m_textBrush)
    return;

  D2D1_SIZE_F renderTargetSize = context->GetSize();
  float logicWidth = renderTargetSize.width / ctx.dpiScale;
  float logicHeight = renderTargetSize.height / ctx.dpiScale;

  size_t activeIndex = ctx.currentTrackIndex;
  size_t activeTotal = ctx.totalTracks;

  if (ctx.isPlaylistListViewMode && config) {
    std::vector<std::wstring> playlists = config->GetAvailablePlaylists();
    activeTotal = playlists.size();
    std::wstring currentPlaylist = config->GetDefaultPlaylistPath();
    activeIndex = 0;
    for (size_t i = 0; i < playlists.size(); ++i) {
      if (playlists[i] == currentPlaylist) {
        activeIndex = i;
        break;
      }
    }
  }

  PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
      logicWidth, logicHeight, config, m_playlistSlideX,
      m_playlistManualScrollY, activeIndex, activeTotal);

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

      D2D1_MATRIX_3X2_F shadowTransform = D2D1::Matrix3x2F::Translation(
          layout.gripShadowX,
          layout.gripShadowY + layout.playlistHeight / 2.0f);
      context->SetTransform(shadowTransform * D2D1::Matrix3x2F::Scale(
                                                  ctx.dpiScale, ctx.dpiScale));
      context->FillGeometry(arrowGeometry, m_shadowBrush.Get());
      context->SetTransform(
          D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
    }

    context->DrawLine(
        D2D1::Point2F(layout.gripX, layout.playlistY),
        D2D1::Point2F(layout.gripX, layout.playlistY + layout.playlistHeight),
        m_playlistGripLineBrush.Get(), layout.gripLineWidth);

    D2D1_MATRIX_3X2_F arrowTransform = D2D1::Matrix3x2F::Translation(
        layout.gripX, layout.playlistY + layout.playlistHeight / 2.0f);
    context->SetTransform(arrowTransform *
                          D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
    context->FillGeometry(arrowGeometry, m_playlistGripArrowBrush.Get());
    context->SetTransform(D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
  }

  if (m_playlistSlideX < layout.playlistWidth - 0.5f) {
    if (m_playlistBgBrush) {
      m_playlistBgBrush->SetOpacity(config->GetPlaylistBgOpacity());
      context->FillRectangle(&layout.bgRect, m_playlistBgBrush.Get());
    }

    // Draw Toolbar
    std::wstring hoverText = L"";
    std::wstring icons[3];
    if (ctx.isPlaylistListViewMode) {
      icons[0] = L""; // (非表示)
      icons[1] = L"➕";
      icons[2] = L"🗑️";
    } else {
      icons[0] = L"📁";
      icons[1] = L"➖";
      icons[2] = L"🗑️";
    }

    if (ctx.playlistToolbarHoveredIndex >= 0 &&
        ctx.playlistToolbarHoveredIndex <= 2) {
      int idx = ctx.playlistToolbarHoveredIndex;
      if (ctx.isPlaylistListViewMode) {
        if (idx == 0)
          hoverText = L""; // (非表示)
        if (idx == 1)
          hoverText = L"プレイリストを新規作成する";
        if (idx == 2)
          hoverText = L"プレイリストを削除する";
      } else {
        if (idx == 0)
          hoverText = L"プレイリスト一覧を開く";
        if (idx == 1)
          hoverText = L"再生中の曲をプレイリストから削除する";
        if (idx == 2)
          hoverText = L"プレイリストの全曲を削除する";
      }
    }

    if (!hoverText.empty() && m_toolbarTextFormat && m_textBrush) {
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        D2D1_RECT_F sRect = layout.toolbarLayout.textRect;
        sRect.left += 1.0f;
        sRect.top += 1.0f;
        sRect.right += 1.0f;
        sRect.bottom += 1.0f;
        context->DrawText(
            hoverText.c_str(), static_cast<UINT32>(hoverText.length()),
            m_toolbarTextFormat.Get(), &sRect, m_shadowBrush.Get());
      }
      context->DrawText(hoverText.c_str(),
                        static_cast<UINT32>(hoverText.length()),
                        m_toolbarTextFormat.Get(),
                        &layout.toolbarLayout.textRect, m_textBrush.Get());
    }

    for (int i = 0; i < 3; ++i) {
      if (ctx.isPlaylistListViewMode && i == 0)
        continue; // (非表示)

      if (ctx.playlistToolbarHoveredIndex == i && m_playlistHighlightBrush) {
        m_playlistHighlightBrush->SetOpacity(0.2f);
        context->FillRectangle(&layout.toolbarLayout.buttonHitRects[i],
                               m_playlistHighlightBrush.Get());
      }

      if (m_toolbarIconFormat && !icons[i].empty()) {
        context->DrawText(
            icons[i].c_str(), static_cast<UINT32>(icons[i].length()),
            m_toolbarIconFormat.Get(), &layout.toolbarLayout.buttonHitRects[i],
            m_textBrush.Get());
      }
    }

    context->PushAxisAlignedClip(&layout.clipRect,
                                 D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    float currentY = layout.startY;

    if (m_playlistHighlightBrush) {
      m_playlistHighlightBrush->SetOpacity(0.2f);
    }

    if (ctx.isPlaylistListViewMode) {
      std::vector<std::wstring> playlists = config->GetAvailablePlaylists();
      std::wstring currentPlaylist = config->GetDefaultPlaylistPath();

      for (size_t i = 0; i < playlists.size(); ++i) {
        if (currentY + layout.itemHeight > 0 &&
            currentY < layout.playlistHeight) {
          PlaylistItemLayout itemLayout =
              LayoutCalculator::CalculatePlaylistItemLayout(layout, config,
                                                            currentY);

          if (playlists[i] == currentPlaylist && m_playlistHighlightBrush) {
            m_playlistHighlightBrush->SetOpacity(0.2f);
            context->FillRectangle(&itemLayout.hlRect,
                                   m_playlistHighlightBrush.Get());
          }

          std::wstring title;
          try {
            title = std::filesystem::path(playlists[i]).filename().wstring();
          } catch (...) {
            title = L"Unknown";
          }

          context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                            m_playlistTitleTextFormat.Get(),
                            &itemLayout.titleRect, m_textBrush.Get());
        }
        currentY += layout.itemHeight;
      }
    } else {
      for (size_t i = 0; i < ctx.totalTracks && ctx.shuffleMetadataList &&
                         i < ctx.shuffleMetadataList->size();
           ++i) {
        if (currentY + layout.itemHeight > 0 &&
            currentY < layout.playlistHeight) {
          PlaylistItemLayout itemLayout =
              LayoutCalculator::CalculatePlaylistItemLayout(layout, config,
                                                            currentY);

          if (i == ctx.currentTrackIndex && m_playlistHighlightBrush) {
            m_playlistHighlightBrush->SetOpacity(0.2f);
            context->FillRectangle(&itemLayout.hlRect,
                                   m_playlistHighlightBrush.Get());
          }

          const TrackMetadata &meta = (*ctx.shuffleMetadataList)[i];
          std::wstring title;
          std::wstring artist;
          std::wstring timeStr;

          if (meta.isLoaded) {
            title = meta.title;
            artist = meta.artist;
            timeStr = meta.timeString;
          } else {
            try {
              title = std::filesystem::path(meta.filepath).filename().wstring();
            } catch (...) {
              title = L"Unknown";
            }
            artist = L"Unknown Artist";
          }

          context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                            m_playlistTitleTextFormat.Get(),
                            &itemLayout.titleRect, m_textBrush.Get());

          if (!artist.empty()) {
            context->DrawText(
                artist.c_str(), static_cast<UINT32>(artist.length()),
                m_playlistArtistTextFormat.Get(), &itemLayout.artistRect,
                m_playlistArtistBrush ? m_playlistArtistBrush.Get()
                                      : m_textBrush.Get());
          }

          if (!timeStr.empty() && m_playlistTimeTextFormat) {
            D2D1_RECT_F timeRect =
                D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                            itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                            itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
            context->DrawText(timeStr.c_str(),
                              static_cast<UINT32>(timeStr.length()),
                              m_playlistTimeTextFormat.Get(), &timeRect,
                              m_playlistTimeBrush ? m_playlistTimeBrush.Get()
                                                  : m_textBrush.Get());
          }
        }
        currentY += layout.itemHeight;
      }
    }

    context->PopAxisAlignedClip();
  }
}

// ================= ResizeGripWidget =================

void ResizeGripWidget::CreateResources(ID2D1DeviceContext *context,
                                       IWICImagingFactory *wicFactory,
                                       IDWriteFactory *dwriteFactory,
                                       const ConfigManager *config) {
  if (!config)
    return;

  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_resizeGripBrush);

  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
  context->GetFactory(&d2dFactory);

  if (d2dFactory) {
    d2dFactory->CreatePathGeometry(&m_resizeGripGeometry);
    if (m_resizeGripGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      if (SUCCEEDED(m_resizeGripGeometry->Open(&sink))) {
        sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        sink->BeginFigure(D2D1::Point2F(0.0f, -15.0f),
                          D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(0.0f, 0.0f));
        sink->AddLine(D2D1::Point2F(-15.0f, 0.0f));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
      }
    }
  }
}

void ResizeGripWidget::ReleaseResources() {
  m_resizeGripBrush.Reset();
  m_resizeGripGeometry.Reset();
}

void ResizeGripWidget::Draw(ID2D1DeviceContext *context,
                            const WidgetContext &ctx,
                            const ConfigManager *config) {
  if (config && config->GetEnableResize() && m_resizeGripBrush &&
      m_resizeGripGeometry) {
    D2D1_SIZE_F renderTargetSize = context->GetSize();
    float logicWidth = renderTargetSize.width / ctx.dpiScale;
    float logicHeight = renderTargetSize.height / ctx.dpiScale;

    m_resizeGripBrush->SetOpacity(0.5f);
    D2D1_MATRIX_3X2_F oldTransform;
    context->GetTransform(&oldTransform);
    context->SetTransform(
        D2D1::Matrix3x2F::Translation(logicWidth, logicHeight) * oldTransform);
    context->FillGeometry(m_resizeGripGeometry.Get(), m_resizeGripBrush.Get());
    context->SetTransform(oldTransform);
  }
}
