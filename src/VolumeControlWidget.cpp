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
