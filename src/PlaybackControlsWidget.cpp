#include "PlaybackControlsWidget.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

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
