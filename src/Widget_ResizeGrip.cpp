#include "Widget_ResizeGrip.h"
#include "WidgetCommon.h"
#include "Config/ConfigManager.h"

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
  if (config && config->GetWindow().EnableResize && m_resizeGripBrush &&
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
