#include "Widget_PlaybackControls.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <sstream>

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

    std::vector<float> pts;
    std::wstringstream ss(config->GetLayoutPlaybackControls().SkipIconPoints);
    std::wstring item;
    while (std::getline(ss, item, L',')) {
        try { pts.push_back(std::stof(item)); } catch(...) {}
    }
    if (pts.size() < 6) pts = {0.25f, -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.25f, 0.5f, -0.25f, 0.0f};

    d2dFactory->CreatePathGeometry(&m_chevronLeftGeometry);
    if (m_chevronLeftGeometry && pts.size() >= 6) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_chevronLeftGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      sink->BeginFigure(D2D1::Point2F(pts[0], pts[1]), D2D1_FIGURE_BEGIN_FILLED);
      for (size_t i = 2; i < pts.size() - 1; i += 2) {
          sink->AddLine(D2D1::Point2F(pts[i], pts[i+1]));
      }
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }

    d2dFactory->CreatePathGeometry(&m_chevronRightGeometry);
    if (m_chevronRightGeometry && pts.size() >= 6) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_chevronRightGeometry->Open(&sink);
      sink->SetFillMode(D2D1_FILL_MODE_WINDING);
      sink->BeginFigure(D2D1::Point2F(-pts[0], pts[1]), D2D1_FIGURE_BEGIN_FILLED);
      for (size_t i = 2; i < pts.size() - 1; i += 2) {
          sink->AddLine(D2D1::Point2F(-pts[i], pts[i+1]));
      }
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }
  }

  if (dwriteFactory) {
    Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
    dwriteFactory->CreateTextFormat(
        L"Meiryo", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaybackControls().SkipTextFontSize, L"en-us", &textFormat);
    if (textFormat) {
      dwriteFactory->CreateTextLayout(L"10", 2, textFormat.Get(), 50.0f, 20.0f,
                                      &m_indicatorTextLayout);
    }
  }
}

void PlaybackControlsWidget::ReleaseResources() {
  m_controlBrush.Reset();
  m_playIconGeometry.Reset();
  m_prevIconGeometry.Reset();
  m_chevronLeftGeometry.Reset();
  m_chevronRightGeometry.Reset();
  m_indicatorTextLayout.Reset();
}

void PlaybackControlsWidget::UpdateAnimation(const WidgetContext &ctx) {
    if (!ctx.config) return;
    float fadeOutSpeed = ctx.config->GetUICommonParm().HoverFadeOutSpeed * ctx.deltaTime;
    float fadeInSpeed = 10.0f * ctx.deltaTime; // Fade in is generally faster

    for (int i = 0; i < 5; ++i) {
        if (ctx.clickedPlaybackIndex == i + 1) { // 1-based index from Window
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

        if (ctx.playbackHoveredIndex == i + 1) { // 1-based index from Window
            m_hoverAlpha[i] += fadeInSpeed;
            if (m_hoverAlpha[i] > 1.0f) m_hoverAlpha[i] = 1.0f;
        } else {
            m_hoverAlpha[i] -= fadeOutSpeed;
            if (m_hoverAlpha[i] < 0.0f) m_hoverAlpha[i] = 0.0f;
        }
    }
}
void PlaybackControlsWidget::UpdateLayout(const WidgetContext &ctx,
                                          const ConfigManager *config) {}

void PlaybackControlsWidget::Draw(ID2D1DeviceContext *context,
                                  const WidgetContext &ctx,
                                  const ConfigManager *config) {
  if (ctx.controlAlpha <= 0.0f || !config || !config->GetVisibility().ShowPlaybackControls)
    return;

  D2D1_SIZE_F renderTargetSize = context->GetSize();
  float logicWidth = renderTargetSize.width / ctx.dpiScale;
  float logicHeight = renderTargetSize.height / ctx.dpiScale;

  PlaybackControlsLayout layout =
      LayoutCalculator::CalculatePlaybackControlsLayout(logicWidth, logicHeight,
                                                        config);

  if (m_controlBrush) {
    auto HexToColorF = [](const std::wstring& hex) {
        if (hex.length() == 7 && hex[0] == L'#') {
            int r, g, b;
            if (swscanf_s(hex.c_str(), L"#%02x%02x%02x", &r, &g, &b) == 3) {
                return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
            }
        }
        return D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);
    };

    auto GetButtonColor = [&](int index) {
        D2D1_COLOR_F baseColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, ctx.controlAlpha);
        D2D1_COLOR_F hoverColor = ParseHexColor(config->GetUICommonParm().FocusColor);
        hoverColor.a = ctx.controlAlpha;
        float t = m_hoverAlpha[index];
        return D2D1::ColorF(
            baseColor.r + (hoverColor.r - baseColor.r) * t,
            baseColor.g + (hoverColor.g - baseColor.g) * t,
            baseColor.b + (hoverColor.b - baseColor.b) * t,
            ctx.controlAlpha
        );
    };

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

    auto DrawChevron = [&](float cx, float cy, float w, float h, bool right) {
      D2D1_MATRIX_3X2_F oldTransform;
      context->GetTransform(&oldTransform);
      context->SetTransform(D2D1::Matrix3x2F::Scale(w, h) *
                            D2D1::Matrix3x2F::Translation(cx, cy) *
                            oldTransform);
      context->FillGeometry(right ? m_chevronRightGeometry.Get()
                                  : m_chevronLeftGeometry.Get(),
                            m_controlBrush.Get());
      context->SetTransform(oldTransform);
    };

    float positions[5] = {layout.centerX - layout.spacing * 2.0f,
                          layout.centerX - layout.spacing, layout.centerX,
                          layout.centerX + layout.spacing,
                          layout.centerX + layout.spacing * 2.0f};

    auto DrawRipple = [&](int index, auto drawFunc) {
        if (m_isRippling[index]) {
            float rp = m_rippleProgress[index];
            float scale = 1.0f + rp * 0.5f;
            float opacity = 0.5f * (1.0f - rp) * ctx.controlAlpha;
            
            D2D1_COLOR_F rippleColor = ParseHexColor(config->GetUICommonParm().FocusColor);
            rippleColor.a = opacity;
            m_controlBrush->SetColor(rippleColor);
            
            float cx = positions[index];
            float cy = layout.centerY;
            D2D1_POINT_2F center = { cx, cy };
            
            D2D1_MATRIX_3X2_F oldTransform;
            context->GetTransform(&oldTransform);
            context->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale, center) * oldTransform);
            
            drawFunc();
            
            context->SetTransform(oldTransform);
        }
    };

    // 1. Prev
    m_controlBrush->SetColor(GetButtonColor(0));
    float prevX = positions[0];
    auto drawPrev = [&]() {
      DrawRect(prevX - layout.half + layout.size * 0.1f, layout.centerY,
               layout.size * 0.2f, layout.size);
      DrawTriangle(prevX - layout.size * 0.1f, layout.centerY, layout.size * 0.4f,
                   layout.size, false);
      DrawTriangle(prevX + layout.size * 0.3f, layout.centerY, layout.size * 0.4f,
                   layout.size, false);
    };
    drawPrev();
    DrawRipple(0, drawPrev);

    // 2. Skip Back
    m_controlBrush->SetColor(GetButtonColor(1));
    float skipBackX = positions[1];
    auto drawSkipBack = [&]() {
      DrawChevron(skipBackX - layout.size * 0.15f, layout.centerY,
                  layout.size * 0.5f, layout.size, false);
      DrawChevron(skipBackX + layout.size * 0.15f, layout.centerY,
                  layout.size * 0.5f, layout.size, false);
      if (m_indicatorTextLayout) {
        float textX = skipBackX + layout.size * config->GetLayoutPlaybackControls().SkipTextOffsetX;
        float textY = layout.centerY + layout.size * config->GetLayoutPlaybackControls().SkipTextOffsetY;
        float shift = config->GetLayoutPlaybackControls().SkipTextShadowShift;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> shadowBrush;
        context->CreateSolidColorBrush(HexToColorF(config->GetUICommonParm().ShadowColor),
                                       &shadowBrush);
        if (shadowBrush) {
            shadowBrush->SetOpacity(config->GetUICommonParm().ShadowOpacity * ctx.controlAlpha);
            context->DrawTextLayout(D2D1::Point2F(textX + shift, textY + shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
            context->DrawTextLayout(D2D1::Point2F(textX - shift, textY - shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
            context->DrawTextLayout(D2D1::Point2F(textX + shift, textY - shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
            context->DrawTextLayout(D2D1::Point2F(textX - shift, textY + shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
        }
        context->DrawTextLayout(D2D1::Point2F(textX, textY),
                                m_indicatorTextLayout.Get(), m_controlBrush.Get());
      }
    };
    drawSkipBack();
    DrawRipple(1, drawSkipBack);

    // 3. Play/Pause
    m_controlBrush->SetColor(GetButtonColor(2));
    auto drawPlay = [&]() {
      if (ctx.isPlaying) {
        DrawRect(layout.centerX - layout.size * 0.2f, layout.centerY,
                 layout.size * 0.3f, layout.size);
        DrawRect(layout.centerX + layout.size * 0.2f, layout.centerY,
                 layout.size * 0.3f, layout.size);
      } else {
        DrawTriangle(layout.centerX, layout.centerY, layout.size, layout.size,
                     true);
      }
    };
    drawPlay();
    DrawRipple(2, drawPlay);

    // 4. Skip Forward
    m_controlBrush->SetColor(GetButtonColor(3));
    float skipFwdX = positions[3];
    auto drawSkipFwd = [&]() {
      DrawChevron(skipFwdX - layout.size * 0.15f, layout.centerY,
                  layout.size * 0.5f, layout.size, true);
      DrawChevron(skipFwdX + layout.size * 0.15f, layout.centerY,
                  layout.size * 0.5f, layout.size, true);
      if (m_indicatorTextLayout) {
        float textX = skipFwdX + layout.size * config->GetLayoutPlaybackControls().SkipTextOffsetX;
        float textY = layout.centerY + layout.size * config->GetLayoutPlaybackControls().SkipTextOffsetY;
        float shift = config->GetLayoutPlaybackControls().SkipTextShadowShift;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> shadowBrush;
        context->CreateSolidColorBrush(HexToColorF(config->GetUICommonParm().ShadowColor),
                                       &shadowBrush);
        if (shadowBrush) {
            shadowBrush->SetOpacity(config->GetUICommonParm().ShadowOpacity * ctx.controlAlpha);
            context->DrawTextLayout(D2D1::Point2F(textX + shift, textY + shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
            context->DrawTextLayout(D2D1::Point2F(textX - shift, textY - shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
            context->DrawTextLayout(D2D1::Point2F(textX + shift, textY - shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
            context->DrawTextLayout(D2D1::Point2F(textX - shift, textY + shift),
                                    m_indicatorTextLayout.Get(), shadowBrush.Get());
        }
        context->DrawTextLayout(D2D1::Point2F(textX, textY),
                                m_indicatorTextLayout.Get(), m_controlBrush.Get());
      }
    };
    drawSkipFwd();
    DrawRipple(3, drawSkipFwd);

    // 5. Next
    m_controlBrush->SetColor(GetButtonColor(4));
    float nextX = positions[4];
    auto drawNext = [&]() {
      DrawTriangle(nextX - layout.size * 0.3f, layout.centerY, layout.size * 0.4f,
                   layout.size, true);
      DrawTriangle(nextX + layout.size * 0.1f, layout.centerY, layout.size * 0.4f,
                   layout.size, true);
      DrawRect(nextX + layout.half - layout.size * 0.1f, layout.centerY,
               layout.size * 0.2f, layout.size);
    };
    drawNext();
    DrawRipple(4, drawNext);
  }
}
