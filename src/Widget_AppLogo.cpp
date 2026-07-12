#include "Widget_AppLogo.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "resource.h"

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
