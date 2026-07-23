#include "Widget_AppLogo.h"
#include "WidgetCommon.h"
#include "Config/ConfigManager.h"
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
}

void AppLogoWidget::ReleaseResources() {
  m_appLogoBitmap.Reset();
  m_appLogoHoverBitmap.Reset();
}

void AppLogoWidget::UpdateAnimation(const WidgetContext &ctx) {
    bool isMenuExpanded = (ctx.outIsLogoMenuExpanded && *ctx.outIsLogoMenuExpanded);
    if (ctx.isHovered || isMenuExpanded) {
        m_logoHoverAlpha += (ctx.config ? ctx.config->GetLayoutAppLogo().LogoFadeSpeed : 5.0f) * ctx.deltaTime;
        if (m_logoHoverAlpha > 1.0f) m_logoHoverAlpha = 1.0f;
    } else {
        m_logoHoverAlpha -= (ctx.config ? ctx.config->GetLayoutAppLogo().LogoFadeSpeed : 5.0f) * ctx.deltaTime;
        if (m_logoHoverAlpha < 0.0f) m_logoHoverAlpha = 0.0f;
    }

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
  if (config && config->GetVisibility().ShowAppLogo && m_appLogoBitmap) {
    D2D1_SIZE_F rtSize = context->GetSize();
    float logicWidth = rtSize.width / ctx.dpiScale;
    AppLogoLayout layout =
        LayoutCalculator::CalculateAppLogoLayout(logicWidth, config);

    float idleOpacity = config->GetLayoutAppLogo().LogoIdleOpacity;
    float baseOpacity = idleOpacity + (1.0f - idleOpacity) * m_logoHoverAlpha;

    context->DrawBitmap(m_appLogoBitmap.Get(), &layout.destRect, baseOpacity,
                        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

    if (m_appLogoHoverBitmap && m_logoHoverAlpha > 0.0f) {
      context->DrawBitmap(m_appLogoHoverBitmap.Get(), &layout.destRect, m_logoHoverAlpha,
                          D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    }

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
        
        ID2D1Bitmap* rippleBmp = (m_logoHoverAlpha >= 0.5f && m_appLogoHoverBitmap) ? m_appLogoHoverBitmap.Get() : m_appLogoBitmap.Get();
        context->DrawBitmap(rippleBmp, &layout.destRect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
        
        context->SetTransform(oldTransform);
    }
  }
}
