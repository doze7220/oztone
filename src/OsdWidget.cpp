#include "OsdWidget.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

void OsdWidget::CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) {
    if (!config) return;

    m_dwriteFactory = dwriteFactory;

    context->CreateSolidColorBrush(ParseHexColor(config->GetOsdTextColor()), &m_textBrush);
    context->CreateSolidColorBrush(ParseHexColor(config->GetOsdShadowColor()), &m_shadowBrush);

    dwriteFactory->CreateTextFormat(
        config->GetOsdFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        config->GetOsdFontSize(),
        L"ja-jp",
        &m_textFormat
    );

    if (m_textFormat) {
        m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
}

void OsdWidget::ReleaseResources() {
    m_textBrush.Reset();
    m_shadowBrush.Reset();
    m_textFormat.Reset();
    m_textLayout.Reset();
    m_dwriteFactory.Reset();
    m_lastString.clear();
}

void OsdWidget::UpdateAnimation(const WidgetContext& ctx) {
    // 減衰処理は Renderer 側で行われているので何もしない
}

void OsdWidget::UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) {
    // 何もしない
}

void OsdWidget::RebuildTextLayout(const std::wstring& text, const ConfigManager* config) {
    m_textLayout.Reset();
    if (!m_dwriteFactory || !m_textFormat || text.empty()) return;

    float maxWidth = 2000.0f; // 画面幅より十分大きい値
    float maxHeight = 1000.0f;

    m_dwriteFactory->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        m_textFormat.Get(),
        maxWidth,
        maxHeight,
        &m_textLayout
    );
}

void OsdWidget::Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) {
    if (ctx.flyTextAlpha <= 0.0f || ctx.flyTextString.empty() || !config) return;

    if (m_lastString != ctx.flyTextString) {
        RebuildTextLayout(ctx.flyTextString, config);
        m_lastString = ctx.flyTextString;
    }

    if (!m_textLayout) return;

    DWRITE_TEXT_METRICS metrics;
    m_textLayout->GetMetrics(&metrics);

    D2D1_SIZE_F renderTargetSize = context->GetSize();
    float logicWidth = renderTargetSize.width / ctx.dpiScale;
    float logicHeight = renderTargetSize.height / ctx.dpiScale;

    OsdLayout layout = LayoutCalculator::CalculateOsdLayout(logicWidth, logicHeight, metrics.width, metrics.height, config);

    // テキストレイアウト自体のサイズを指定サイズに制限（アライメント用）
    m_textLayout->SetMaxWidth(metrics.width);
    m_textLayout->SetMaxHeight(metrics.height);

    if (m_shadowBrush && config->GetOsdShadowOpacity() > 0.0f) {
        m_shadowBrush->SetOpacity(ctx.flyTextAlpha * config->GetOsdShadowOpacity());
        context->DrawTextLayout(D2D1::Point2F(layout.shadowRect.left, layout.shadowRect.top), m_textLayout.Get(), m_shadowBrush.Get());
    }

    if (m_textBrush) {
        m_textBrush->SetOpacity(ctx.flyTextAlpha);
        context->DrawTextLayout(D2D1::Point2F(layout.textRect.left, layout.textRect.top), m_textLayout.Get(), m_textBrush.Get());
    }
}
