#include "Widget_LogoMenu.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "resource.h"
#include "Window.h"
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
        config->GetLogoMenuFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetLogoMenuVisualizerFontSize(),
        L"ja-jp", &m_indicatorTextFormat);

    dwriteFactory->CreateTextFormat(
        config->GetLogoMenuFontFamily().c_str(), nullptr,
        DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetLogoMenuLockIconFontSize(),
        L"ja-jp", &m_lockIconTextFormat);
    if (m_indicatorTextFormat) {
      m_indicatorTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      m_indicatorTextFormat->SetParagraphAlignment(
          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }
    if (m_lockIconTextFormat) {
      m_lockIconTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      m_lockIconTextFormat->SetParagraphAlignment(
          DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }

    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                   &m_iconBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.4f),
                                   &m_inactiveIconBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                   &m_typingTextBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                   &m_shadowBrush);
  }
}

void LogoMenuWidget::ReleaseResources() {
  m_iconTextFormat.Reset();
  m_typingTextFormat.Reset();
  m_indicatorTextFormat.Reset();
  m_lockIconTextFormat.Reset();
  m_iconBrush.Reset();
  m_inactiveIconBrush.Reset();
  m_typingTextBrush.Reset();
  m_dwriteFactory.Reset();
}

void LogoMenuWidget::UpdateAnimation(const WidgetContext &ctx) {
  float duration = ctx.config ? ctx.config->GetLogoMenuScrollDuration() : 0.5f;
  float speed = (duration > 0.0f) ? (1.0f / duration) : 2.0f;
  if (ctx.isLogoMenuHovered || ctx.isHovered) {
    m_logoMenuLeaveTimer = ctx.config ? ctx.config->GetMenuLeaveDelay() : 3.0f;
  } else if (m_logoMenuLeaveTimer > 0.0f) {
    m_logoMenuLeaveTimer -= ctx.deltaTime;
  }

  if (ctx.isLogoMenuHovered || ctx.isHovered || m_logoMenuLeaveTimer > 0.0f) {
    m_menuProgress += ctx.deltaTime * speed;
    if (m_menuProgress > 1.0f)
      m_menuProgress = 1.0f;
  } else {
    m_menuProgress -= ctx.deltaTime * speed;
    if (m_menuProgress < 0.0f)
      m_menuProgress = 0.0f;
  }

  if (ctx.outIsLogoMenuExpanded) {
      *ctx.outIsLogoMenuExpanded = ctx.isLogoMenuHovered || ctx.isHovered || (m_logoMenuLeaveTimer > 0.0f);
  }

  if (ctx.logoMenuItems && ctx.config) {
    size_t count = ctx.logoMenuItems->size();
    if (m_hoverAlpha.size() != count) {
      m_hoverAlpha.resize(count, 0.0f);
      m_rippleProgress.resize(count, 0.0f);
      m_isRippling.resize(count, false);
    }

    float fadeOutSpeed = ctx.config->GetHoverFadeOutSpeed() * ctx.deltaTime;
    float fadeInSpeed = 10.0f * ctx.deltaTime;

    for (size_t i = 0; i < count; ++i) {
      if (ctx.clickedLogoMenuIndex == static_cast<int>(i)) {
          m_isRippling[i] = true;
          m_rippleProgress[i] = 0.0f;
      }
      if (m_isRippling[i]) {
          m_rippleProgress[i] += ctx.deltaTime * 3.0f;
          if (m_rippleProgress[i] >= 1.0f) {
              m_isRippling[i] = false;
              m_rippleProgress[i] = 1.0f;
          }
      }

      if (ctx.logoMenuHoveredIndex == static_cast<int>(i)) {
        m_hoverAlpha[i] += fadeInSpeed;
        if (m_hoverAlpha[i] > 1.0f) m_hoverAlpha[i] = 1.0f;
      } else {
        m_hoverAlpha[i] -= fadeOutSpeed;
        if (m_hoverAlpha[i] < 0.0f) m_hoverAlpha[i] = 0.0f;
      }
    }
  }
}

void LogoMenuWidget::UpdateLayout(const WidgetContext &ctx,
                                  const ConfigManager *config) {}

void LogoMenuWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                          const ConfigManager *config) {
  if (!config || m_menuProgress <= 0.0f || !ctx.logoMenuItems)
    return;

  float easedProgress = CubicEaseOut(m_menuProgress);

  D2D1_SIZE_F rtSize = context->GetSize();
  float logicWidth = rtSize.width / ctx.dpiScale;

  LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
      logicWidth, config, easedProgress, ctx.logoMenuItems->size());

  D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE;

  for (size_t i = 0; i < layout.items.size(); ++i) {
    const auto &item = (*ctx.logoMenuItems)[i];
    const auto &itemLayout = layout.items[i];

    bool active = true;
    std::wstring iconText = item.iconText;
    bool hasIndicator = false;
    std::wstring indicatorText;

    if (item.commandId == Window::ID_LOGO_VISUALIZER) {
      int mode = config->GetVisualizerMode();
      active = (mode != 0);
      if (mode == 1 || mode == 2) {
        hasIndicator = true;
        indicatorText = mode == 1 ? L"1" : L"2";
      }
    } else if (item.commandId == Window::ID_LOGO_BG_MODE) {
      int mode = config->GetBackgroundArtMode();
      active = (mode != 1);
      if (mode == 2) {
        hasIndicator = true;
        indicatorText = L"2";
      }
    } else if (item.commandId == Window::ID_LOGO_RESIZE_MODE) {
      active = config->GetEnableResize();
    } else if (item.commandId == Window::ID_LOGO_LOCK_POS) {
      active = config->GetLockWindowPosition();
      hasIndicator = true;
      indicatorText = active ? L"🔒" : L"🔓";
    } else if (item.commandId == Window::ID_LOGO_SHUFFLE) {
      if (config->GetShuffleMode()) {
        iconText = L"🔀";
      } else {
        iconText = L"➡️";
      }
    } else if (item.isToggle && !item.toggleState) {
      active = false;
    }

    auto GetBlendedColor = [&](bool isActive, int index) {
        D2D1_COLOR_F baseColor = isActive ? D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f) : D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.4f);
        D2D1_COLOR_F hoverColor = ParseHexColor(config->GetHoverIconColor());
        float t = (index >= 0 && index < static_cast<int>(m_hoverAlpha.size())) ? m_hoverAlpha[index] : 0.0f;
        return D2D1_COLOR_F{
            baseColor.r + (hoverColor.r - baseColor.r) * t,
            baseColor.g + (hoverColor.g - baseColor.g) * t,
            baseColor.b + (hoverColor.b - baseColor.b) * t,
            baseColor.a + (hoverColor.a - baseColor.a) * t
        };
    };

    ID2D1SolidColorBrush *brush =
        active ? m_iconBrush.Get() : m_inactiveIconBrush.Get();
    brush->SetColor(GetBlendedColor(active, static_cast<int>(i)));

    context->DrawText(
        iconText.c_str(), static_cast<UINT32>(iconText.length()),
        m_iconTextFormat.Get(), itemLayout.hitRect, brush, options);

    if (m_isRippling[i]) {
        float rp = m_rippleProgress[i];
        float scale = 1.0f + rp * 0.5f;
        float opacity = 0.5f * (1.0f - rp);
        
        D2D1_COLOR_F baseColor = GetBlendedColor(active, static_cast<int>(i));
        baseColor.a = opacity;
        ID2D1SolidColorBrush *rippleBrush = active ? m_iconBrush.Get() : m_inactiveIconBrush.Get();
        rippleBrush->SetColor(baseColor);
        
        D2D1_POINT_2F center = {
            itemLayout.hitRect.left + (itemLayout.hitRect.right - itemLayout.hitRect.left) / 2.0f,
            itemLayout.hitRect.top + (itemLayout.hitRect.bottom - itemLayout.hitRect.top) / 2.0f
        };
        
        D2D1_MATRIX_3X2_F oldTransform;
        context->GetTransform(&oldTransform);
        context->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale, center) * oldTransform);
        
        context->DrawText(
            iconText.c_str(), static_cast<UINT32>(iconText.length()),
            m_iconTextFormat.Get(), itemLayout.hitRect, rippleBrush, options);
            
        context->SetTransform(oldTransform);
    }

    if (hasIndicator) {
        D2D1_RECT_F indRect = itemLayout.hitRect;
        bool isLockPos = (item.commandId == Window::ID_LOGO_LOCK_POS);
        
        indRect.left =
            itemLayout.position.x + (isLockPos ? config->GetLogoMenuLockIconOffsetX() : config->GetLogoMenuVisualizerIconOffsetX());
        indRect.top =
            itemLayout.position.y + (isLockPos ? config->GetLogoMenuLockIconOffsetY() : config->GetLogoMenuVisualizerIconOffsetY());

        // Create a temporary layout for shadow/outline effect
        IDWriteTextFormat* format = isLockPos && m_lockIconTextFormat ? m_lockIconTextFormat.Get() : m_indicatorTextFormat.Get();
        if (m_dwriteFactory && format) {
          Microsoft::WRL::ComPtr<IDWriteTextLayout> modeLayout;
          m_dwriteFactory->CreateTextLayout(indicatorText.c_str(), static_cast<UINT32>(indicatorText.length()),
                                            format, 50.0f,
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
            m_iconBrush->SetColor(GetBlendedColor(true, static_cast<int>(i)));
            context->DrawTextLayout(D2D1::Point2F(indRect.left, indRect.top),
                                    modeLayout.Get(), m_iconBrush.Get());
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
    } else if (hoveredItem.commandId == Window::ID_LOGO_BG_MODE) {
      int mode = config->GetBackgroundArtMode();
      if (mode == 1)
        textToDraw = L"背景表示: 非表示";
      else if (mode == 2)
        textToDraw = L"背景表示: デフォルト固定";
      else
        textToDraw = L"背景表示: 再生中のアートワーク";
    } else if (hoveredItem.commandId == Window::ID_LOGO_RESIZE_MODE) {
      if (config->GetEnableResize())
        textToDraw = L"リサイズモード: 有効";
      else
        textToDraw = L"リサイズモード: 無効";
    } else if (hoveredItem.commandId == Window::ID_LOGO_LOCK_POS) {
      if (config->GetLockWindowPosition())
        textToDraw = L"画面位置固定: 有効";
      else
        textToDraw = L"画面位置固定: 無効";
    } else if (hoveredItem.commandId == Window::ID_LOGO_PLAYLIST_POS) {
      if (config->GetPlaylistPosition() == 0) {
        textToDraw = L"プレイリスト配置: 画面左";
      } else {
        textToDraw = L"プレイリスト配置: 画面右";
      }
    } else if (hoveredItem.commandId == Window::ID_LOGO_SHUFFLE) {
      if (config->GetShuffleMode()) {
        textToDraw = L"再生モード：シャッフル";
      } else {
        textToDraw = L"再生モード：プレイリスト登録順";
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

void LogoMenuWidget::ForceClearHoverDelay() {
    m_logoMenuLeaveTimer = 0.0f;
}
