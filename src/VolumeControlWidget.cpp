#include "VolumeControlWidget.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

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

    context->CreateSolidColorBrush(ParseHexColor(config->GetVolumeTooltipBgColor()), &m_tooltipBgBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_tooltipIconBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.3f, 0.3f, 1.0f), &m_tooltipWheelBrush);
  }

  context->GetFactory(&m_d2dFactory);

  if (m_d2dFactory) {
    m_d2dFactory->CreatePathGeometry(&m_speakerIconGeometry);
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

    m_d2dFactory->CreatePathGeometry(&m_tooltipStrokeGeometry);
    if (m_tooltipStrokeGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      m_tooltipStrokeGeometry->Open(&sink);
      
      float mx = -0.13f; // Mouse center offset for centering the whole group
      float rx = 0.15f, ry = 0.25f, r = 0.1f;
      
      // Mouse outline (rounded rect approximation)
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

      // ┴ shape button divider
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
      
      float ax = 0.20f; // Arrow center X
      float aw = 0.08f; // Arrow half width

      // Up arrow
      sink->BeginFigure(D2D1::Point2F(ax-aw, -0.05f), D2D1_FIGURE_BEGIN_FILLED);
      sink->AddLine(D2D1::Point2F(ax, -0.25f));
      sink->AddLine(D2D1::Point2F(ax+aw, -0.05f));
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      
      // Down arrow
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
      
      float mx = -0.13f; // Mouse center X
      float wx = 0.03f;  // Wheel half width

      // Wheel (Red rect)
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

      float w = config ? config->GetVolumeTooltipWidth() : 50.0f;
      float h = config ? config->GetVolumeTooltipHeight() : 26.0f;
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

void VolumeControlWidget::ReleaseResources() {
  m_controlBrush.Reset();
  m_shadowBrush.Reset();
  m_speakerIconGeometry.Reset();
  m_volumeTextFormat.Reset();
  m_volTextLayout.Reset();
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

void VolumeControlWidget::UpdateAnimation(const WidgetContext &ctx) {
  if (ctx.isVolumeHovered) {
    m_tooltipAlpha = 1.0f;
  } else {
    float fadeOutSpeed = ctx.config ? ctx.config->GetHoverFadeOutSpeed() : 3.0f;
    m_tooltipAlpha -= ctx.deltaTime * fadeOutSpeed;
    if (m_tooltipAlpha < 0.0f) m_tooltipAlpha = 0.0f;
  }
}
void VolumeControlWidget::UpdateLayout(const WidgetContext &ctx,
                                       const ConfigManager *config) {
  if (!config || !m_dwriteFactory) return;

  D2D1_SIZE_F renderTargetSize = ctx.currentArtBitmap ? D2D1_SIZE_F{1024, 512} : D2D1_SIZE_F{1024, 512}; // Need actual context size, but we can just use DPI scale logic
}

void VolumeControlWidget::Draw(ID2D1DeviceContext *context,
                               const WidgetContext &ctx,
                               const ConfigManager *config) {
  float finalAlpha = (std::max)(ctx.controlAlpha, ctx.osdVolumeAlpha);
  if (finalAlpha <= 0.0f || !config || !config->GetShowVolumeControl())
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
    m_controlBrush->SetOpacity(finalAlpha);

    D2D1::Matrix3x2F oldTransform;
    context->GetTransform(&oldTransform);
    D2D1_MATRIX_3X2_F spkTransform =
        D2D1::Matrix3x2F::Scale(layout.volSize, layout.volSize) *
        D2D1::Matrix3x2F::Translation(layout.volX, layout.volY);

    if (m_shadowBrush && config->GetVolumeEnableShadow()) {
      m_shadowBrush->SetOpacity(config->GetVolumeShadowOpacity() *
                                finalAlpha);

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

    if (m_tooltipAlpha > 0.0f && m_tooltipGeometry && m_tooltipStrokeGeometry) {
      float tooltipAlphaFinal = m_tooltipAlpha * finalAlpha;
      if (m_tooltipBgBrush) {
        m_tooltipBgBrush->SetOpacity(tooltipAlphaFinal * config->GetVolumeTooltipBgOpacity());
        context->SetTransform(D2D1::Matrix3x2F::Translation(layout.tooltipBoxX, layout.tooltipBoxY) * oldTransform);
        context->FillGeometry(m_tooltipGeometry.Get(), m_tooltipBgBrush.Get());
      }
      
      float iconSize = config->GetVolumeTooltipIconSize();
      float cx = layout.tooltipBoxX + layout.tooltipBoxW / 2.0f;
      float cy = layout.tooltipBoxY + layout.tooltipBoxH / 2.0f;
      context->SetTransform(D2D1::Matrix3x2F::Scale(iconSize, iconSize) * D2D1::Matrix3x2F::Translation(cx, cy) * oldTransform);

      if (m_tooltipIconBrush) {
        m_tooltipIconBrush->SetOpacity(tooltipAlphaFinal);
        context->DrawGeometry(m_tooltipStrokeGeometry.Get(), m_tooltipIconBrush.Get(), 1.5f / iconSize);
        if (m_tooltipFillGeometry) {
          context->FillGeometry(m_tooltipFillGeometry.Get(), m_tooltipIconBrush.Get());
        }
      }
      if (m_tooltipWheelBrush && m_tooltipWheelGeometry) {
        m_tooltipWheelBrush->SetOpacity(tooltipAlphaFinal);
        context->FillGeometry(m_tooltipWheelGeometry.Get(), m_tooltipWheelBrush.Get());
      }
    }
  }
}
