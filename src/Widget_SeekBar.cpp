#include "Widget_SeekBar.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"

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
    m_seekBarBgBrush->SetColor(WidgetCommon::HexToColorF(config->GetSeekBarBgColor()));
    m_seekBarBgBrush->SetOpacity(config->GetSeekBarBgOpacity() * dimFactor);
    context->FillRectangle(&layout.bgRect, m_seekBarBgBrush.Get());
  }

  if (m_seekBarFgBrush) {
    m_seekBarFgBrush->SetColor(WidgetCommon::HexToColorF(config->GetSeekBarFgColor()));
    m_seekBarFgBrush->SetOpacity(config->GetSeekBarFgOpacity() * dimFactor);
    context->FillRectangle(&layout.fgRect, m_seekBarFgBrush.Get());
  }

  if (m_timeTextLayout && m_textBrush) {
    m_textBrush->SetColor(WidgetCommon::HexToColorF(config->GetSeekBarTimeTextColor()));
    m_textBrush->SetOpacity(config->GetSeekBarTimeTextOpacity() * dimFactor);
    context->DrawTextLayout(layout.textOrigin, m_timeTextLayout.Get(),
                            m_textBrush.Get(),
                            D2D1_DRAW_TEXT_OPTIONS_NONE);
  }
}
