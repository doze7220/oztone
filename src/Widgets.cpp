#include "Widgets.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "resource.h"
#include <filesystem>

namespace {
enum ActionID {
  ACTION_NEXT_TRACK = 0,
  ACTION_PREV_TRACK,
  ACTION_PLAY_PAUSE,
  ACTION_STOP,
  ACTION_VOL_UP_5,
  ACTION_VOL_DOWN_5,
  ACTION_VOL_UP_25,
  ACTION_VOL_DOWN_25,
  ACTION_NEXT_PLAYLIST,
  ACTION_PREV_PLAYLIST,
  ACTION_PIN_TOP,
  ACTION_PIN_BOTTOM,
  ACTION_EXIT_APP
};

} // namespace

// ==========================================
// AppLogoWidget
// ==========================================
void AppLogoWidget::CreateResources(ID2D1DeviceContext *context,
                                    IWICImagingFactory *wicFactory,
                                    IDWriteFactory *dwriteFactory,
                                    const ConfigManager *config) {
  WidgetCommon::LoadBitmapResource(wicFactory, context, L"app_logo.png", IDI_APP_LOGO,
                                   &m_appLogoBitmap);
  WidgetCommon::LoadBitmapResource(wicFactory, context, L"app_logo_hover.png",
                                   IDI_APP_LOGO_HOVER, &m_appLogoHoverBitmap);

  context->CreateEffect(CLSID_D2D1Shadow, &m_shadowEffect);
}

void AppLogoWidget::ReleaseResources() {
  m_appLogoBitmap.Reset();
  m_appLogoHoverBitmap.Reset();
  m_shadowEffect.Reset();
}

void AppLogoWidget::UpdateAnimation(const WidgetContext &ctx) {
    if (ctx.isLogoClicked) {
        m_isRippling = true;
        m_rippleProgress = 0.0f;
    }
    if (m_isRippling) {
        m_rippleProgress += ctx.deltaTime * 3.0f;
        if (m_rippleProgress >= 1.0f) {
            m_isRippling = false;
            m_rippleProgress = 1.0f;
        }
    }
}
void AppLogoWidget::UpdateLayout(const WidgetContext &ctx,
                                 const ConfigManager *config) {}

void AppLogoWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                         const ConfigManager *config) {
  ID2D1Bitmap *bitmapToDraw = (ctx.isHovered || ctx.isLogoMenuHovered)
                                  ? m_appLogoHoverBitmap.Get()
                                  : m_appLogoBitmap.Get();
  if (config && config->GetShowAppLogo() && bitmapToDraw) {
    D2D1_SIZE_F rtSize = context->GetSize();
    float logicWidth = rtSize.width / ctx.dpiScale;
    AppLogoLayout layout =
        LayoutCalculator::CalculateAppLogoLayout(logicWidth, config);

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

    if (m_isRippling) {
        D2D1_POINT_2F center = {
            layout.destRect.left + (layout.destRect.right - layout.destRect.left) / 2.0f,
            layout.destRect.top + (layout.destRect.bottom - layout.destRect.top) / 2.0f
        };
        float scale = 1.0f + m_rippleProgress * 0.5f;
        float opacity = 0.5f * (1.0f - m_rippleProgress);
        
        D2D1_MATRIX_3X2_F oldTransform;
        context->GetTransform(&oldTransform);
        context->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale, center) * oldTransform);
        
        context->DrawBitmap(bitmapToDraw, &layout.destRect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
        
        context->SetTransform(oldTransform);
    }
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

// ================= GlobalHotkeysWidget =================

void GlobalHotkeysWidget::CreateResources(ID2D1DeviceContext *context,
                                          IWICImagingFactory *wicFactory,
                                          IDWriteFactory *dwriteFactory,
                                          const ConfigManager *config) {
  if (!config)
    return;
  m_dwriteFactory = dwriteFactory;

  m_keyTextLayout.Reset();
  m_actionTextLayout
      .Reset(); // Force recreation of text layout when resources are recreated

  dwriteFactory->CreateTextFormat(
      config->GetGlobalHotkeysFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetGlobalHotkeysFontSize(), L"en-us",
      &m_textFormat);

  if (m_textFormat) {
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  }

  context->CreateSolidColorBrush(
      WidgetCommon::HexToColorF(config->GetGlobalHotkeysCoreColor(), 0.9f), &m_coreBrush);
  context->CreateSolidColorBrush(
      WidgetCommon::HexToColorF(config->GetGlobalHotkeysGlowColor(),
                                config->GetGlobalHotkeysGlowOpacity()),
      &m_glowBrush);
  context->CreateSolidColorBrush(
      WidgetCommon::HexToColorF(config->GetGlobalHotkeysShadowColor(), 1.0f), &m_shadowBrush);
}

void GlobalHotkeysWidget::ReleaseResources() {
  m_textFormat.Reset();
  m_keyTextLayout.Reset();
  m_actionTextLayout.Reset();
  m_dwriteFactory.Reset();
  m_coreBrush.Reset();
  m_glowBrush.Reset();
  m_shadowBrush.Reset();
}

void GlobalHotkeysWidget::UpdateAnimation(const WidgetContext &ctx) {}

std::wstring GlobalHotkeysWidget::GetKeyName(int mod, int vk) {
  if (vk == 0)
    return L"None";
  std::wstring result;
  if (mod & MOD_CONTROL)
    result += L"CTRL + ";
  if (mod & MOD_SHIFT)
    result += L"SHIFT + ";
  if (mod & MOD_ALT)
    result += L"ALT + ";
  if (mod & MOD_WIN)
    result += L"WIN + ";

  if (vk >= 'A' && vk <= 'Z') {
    result += static_cast<wchar_t>(vk);
  } else if (vk >= '0' && vk <= '9') {
    result += static_cast<wchar_t>(vk);
  } else {
    switch (vk) {
    case VK_LEFT:
      result += L"←";
      break;
    case VK_RIGHT:
      result += L"→";
      break;
    case VK_UP:
      result += L"↑";
      break;
    case VK_DOWN:
      result += L"↓";
      break;
    case VK_SPACE:
      result += L"SPACE";
      break;
    case VK_ESCAPE:
      result += L"ESC";
      break;
    case VK_RETURN:
      result += L"ENTER";
      break;
    case VK_HOME:
      result += L"HOME";
      break;
    case VK_END:
      result += L"END";
      break;
    case VK_PRIOR:
      result += L"PAGE UP";
      break;
    case VK_NEXT:
      result += L"PAGE DOWN";
      break;
    case VK_INSERT:
      result += L"INSERT";
      break;
    case VK_DELETE:
      result += L"DELETE";
      break;
    case VK_OEM_PLUS:
      result += L"+";
      break;
    case VK_OEM_MINUS:
      result += L"-";
      break;
    default:
      result += L"Key(" + std::to_wstring(vk) + L")";
      break;
    }
  }
  return result;
}

std::wstring GlobalHotkeysWidget::GetActionName(int actionId) {
  switch (actionId) {
  case ACTION_NEXT_TRACK:
    return L": 次の曲";
  case ACTION_PREV_TRACK:
    return L": 前の曲";
  case ACTION_PLAY_PAUSE:
    return L": 再生/一時停止";
  case ACTION_STOP:
    return L": 停止";
  case ACTION_VOL_UP_5:
    return L": 音量 +5%";
  case ACTION_VOL_DOWN_5:
    return L": 音量 -5%";
  case ACTION_VOL_UP_25:
    return L": 音量 +25%";
  case ACTION_VOL_DOWN_25:
    return L": 音量 -25%";
  case ACTION_NEXT_PLAYLIST:
    return L": 前プレイリスト";
  case ACTION_PREV_PLAYLIST:
    return L": 次プレイリスト";
  case ACTION_PIN_TOP:
    return L": 最前面固定";
  case ACTION_PIN_BOTTOM:
    return L": 最背面固定";
  case ACTION_EXIT_APP:
    return L": 終了";
  default:
    return L"";
  }
}

void GlobalHotkeysWidget::GenerateHotkeysStrings(const ConfigManager *config,
                                                 std::wstring &outKeys,
                                                 std::wstring &outActions) {
  outKeys.clear();
  outActions.clear();
  auto append = [&](int actionId, int mod, int vk) {
    if (vk != 0) {
      outKeys += GetKeyName(mod, vk) + L"\n";
      outActions += GetActionName(actionId) + L"\n";
    }
  };
  append(ACTION_NEXT_TRACK, config->GetModifierNextTrack(),
         config->GetVKNextTrack());
  append(ACTION_PREV_TRACK, config->GetModifierPrevTrack(),
         config->GetVKPrevTrack());
  append(ACTION_PLAY_PAUSE, config->GetModifierPlayPause(),
         config->GetVKPlayPause());
  append(ACTION_STOP, config->GetModifierStop(), config->GetVKStop());
  append(ACTION_VOL_UP_5, config->GetModifierVolUp5(), config->GetVKVolUp5());
  append(ACTION_VOL_DOWN_5, config->GetModifierVolDown5(),
         config->GetVKVolDown5());
  append(ACTION_VOL_UP_25, config->GetModifierVolUp25(),
         config->GetVKVolUp25());
  append(ACTION_VOL_DOWN_25, config->GetModifierVolDown25(),
         config->GetVKVolDown25());
  append(ACTION_NEXT_PLAYLIST, config->GetModifierNextPlaylist(),
         config->GetVKNextPlaylist());
  append(ACTION_PREV_PLAYLIST, config->GetModifierPrevPlaylist(),
         config->GetVKPrevPlaylist());
  append(ACTION_PIN_TOP, config->GetModifierActiveTopMost(),
         config->GetVKActiveTopMost());
  append(ACTION_PIN_BOTTOM, config->GetModifierActiveBottom(),
         config->GetVKActiveBottom());
  append(ACTION_EXIT_APP, config->GetModifierExitApp(), config->GetVKExitApp());
}

void GlobalHotkeysWidget::UpdateLayout(const WidgetContext &ctx,
                                       const ConfigManager *config) {
  // Keep it empty because we don't have logicalWidth here.
  // We will update text layout in Draw.
}

void GlobalHotkeysWidget::Draw(ID2D1DeviceContext *context,
                               const WidgetContext &ctx,
                               const ConfigManager *config) {
  if (!config || !config->GetShowHotkeys())
    return;

  D2D1_SIZE_F rtSize = context->GetSize();
  float logicWidth = rtSize.width / ctx.dpiScale;

  bool show = config->GetShowHotkeys();
  std::wstring keyStr, actionStr;
  if (show) {
    GenerateHotkeysStrings(config, keyStr, actionStr);
  }

  if (keyStr != m_lastKeyString || actionStr != m_lastActionString ||
      show != m_wasShow || !m_keyTextLayout || !m_actionTextLayout) {
    m_keyTextLayout.Reset();
    m_actionTextLayout.Reset();
    if (show && (!keyStr.empty() || !actionStr.empty())) {
      GlobalHotkeysLayout layout =
          LayoutCalculator::CalculateGlobalHotkeysLayout(logicWidth, config);

      auto createLayout =
          [&](const std::wstring &text, D2D1_RECT_F rect,
              Microsoft::WRL::ComPtr<IDWriteTextLayout> &outLayout) {
            float w = rect.right - rect.left;
            float h = rect.bottom - rect.top;
            m_dwriteFactory->CreateTextLayout(
                text.c_str(), static_cast<UINT32>(text.length()),
                m_textFormat.Get(), w, h, &outLayout);
            if (outLayout) {
              Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
              if (SUCCEEDED(outLayout.As(&textLayout1))) {
                textLayout1->SetLineSpacing(
                    DWRITE_LINE_SPACING_METHOD_UNIFORM,
                    config->GetGlobalHotkeysLineSpacing(),
                    config->GetGlobalHotkeysFontSize());
              }
            }
          };

      createLayout(keyStr, layout.keyColumnRect, m_keyTextLayout);
      createLayout(actionStr, layout.actionColumnRect, m_actionTextLayout);
    }
    m_lastKeyString = keyStr;
    m_lastActionString = actionStr;
    m_wasShow = show;
  }

  if (!m_keyTextLayout || !m_actionTextLayout)
    return;

  GlobalHotkeysLayout layout =
      LayoutCalculator::CalculateGlobalHotkeysLayout(logicWidth, config);

  auto drawText = [&](Microsoft::WRL::ComPtr<IDWriteTextLayout> &textLayout,
                      D2D1_POINT_2F origin) {
    WidgetCommon::DrawShadowedTextLayout(
        context, textLayout.Get(), nullptr, m_shadowBrush.Get(),
        origin, D2D1::Point2F(origin.x + 2.0f, origin.y + 2.0f), config->GetGlobalHotkeysShadowOpacity());

    if (m_glowBrush) {
      float glowOffsets[] = {1.5f, 3.0f};
      for (float offset : glowOffsets) {
        context->DrawTextLayout(D2D1::Point2F(origin.x - offset, origin.y),
                                textLayout.Get(), m_glowBrush.Get());
        context->DrawTextLayout(D2D1::Point2F(origin.x + offset, origin.y),
                                textLayout.Get(), m_glowBrush.Get());
        context->DrawTextLayout(D2D1::Point2F(origin.x, origin.y - offset),
                                textLayout.Get(), m_glowBrush.Get());
        context->DrawTextLayout(D2D1::Point2F(origin.x, origin.y + offset),
                                textLayout.Get(), m_glowBrush.Get());
      }
    }

    if (m_coreBrush) {
      context->DrawTextLayout(origin, textLayout.Get(), m_coreBrush.Get());
    }
  };

  drawText(m_keyTextLayout,
           D2D1::Point2F(layout.keyColumnRect.left, layout.keyColumnRect.top));
  drawText(m_actionTextLayout, D2D1::Point2F(layout.actionColumnRect.left,
                                             layout.actionColumnRect.top));
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
