#include "Widget_Playlist.h"
#include "ConfigManager.h"
#include "WidgetCommon.h"

void PlaylistWidget::CreateResources(ID2D1DeviceContext *context,
                                     IWICImagingFactory *wicFactory,
                                     IDWriteFactory *dwriteFactory,
                                     const ConfigManager *config) {
  if (!config)
    return;
  m_dwriteFactory = dwriteFactory;

  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaylist().PlaylistTitleFontSize, L"ja-jp",
      &m_playlistTitleTextFormat);

  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaylist().PlaylistArtistFontSize, L"ja-jp",
      &m_playlistArtistTextFormat);

  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaylist().PlaylistTimeFontSize, L"en-us",
      &m_playlistTimeTextFormat);

  auto ApplyTrimming = [&](Microsoft::WRL::ComPtr<IDWriteTextFormat> &format) {
    if (!format)
      return;
    DWRITE_TRIMMING trimmingOptions = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0,
                                       0};
    Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
    if (SUCCEEDED(dwriteFactory->CreateEllipsisTrimmingSign(format.Get(),
                                                            &ellipsis))) {
      format->SetTrimming(&trimmingOptions, ellipsis.Get());
      format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    }
  };
  ApplyTrimming(m_playlistTitleTextFormat);
  ApplyTrimming(m_playlistArtistTextFormat);
  ApplyTrimming(m_playlistTimeTextFormat);

  if (m_playlistTimeTextFormat)
    m_playlistTimeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaylist().ToolbarTextFontSize,
      L"ja-jp", &m_toolbarTextFormat);
  if (m_toolbarTextFormat) {
    m_toolbarTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_toolbarTextFormat->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaylist().ToolbarTextFontSize,
      L"ja-jp", &m_toolbarCountTextFormat);
  if (m_toolbarCountTextFormat) {
    m_toolbarCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    m_toolbarCountTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      config->GetUICommonParm().BaseFontFamily.c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetLayoutPlaylist().TrackCountFontSize,
      L"en-us", &m_trackCountTextFormat);
  if (m_trackCountTextFormat) {
    m_trackCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_trackCountTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      L"Segoe UI Emoji", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetLayoutPlaylist().ToolbarIconSize, L"ja-jp", &m_toolbarIconFormat);
  if (m_toolbarIconFormat) {
    m_toolbarIconFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_toolbarIconFormat->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      L"Segoe UI Emoji", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetLayoutPlaylist().PinSubIconFontSize, L"ja-jp", &m_pinSubIconFormat);
  if (m_pinSubIconFormat) {
    m_pinSubIconFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pinSubIconFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  auto ParseHexColor = [](const std::wstring &hexColor) -> D2D1_COLOR_F {
    if (hexColor.empty() || hexColor[0] != L'#')
      return D2D1::ColorF(D2D1::ColorF::White);
    try {
      unsigned int hexValue = std::stoul(hexColor.substr(1), nullptr, 16);
      if (hexColor.length() == 7) {
        return D2D1::ColorF(((hexValue >> 16) & 0xFF) / 255.0f,
                            ((hexValue >> 8) & 0xFF) / 255.0f,
                            (hexValue & 0xFF) / 255.0f);
      }
    } catch (...) {
    }
    return D2D1::ColorF(D2D1::ColorF::White);
  };

  context->CreateSolidColorBrush(
      ParseHexColor(config->GetLayoutPlaylist().PlaylistArtistColor), &m_playlistArtistBrush);
  context->CreateSolidColorBrush(ParseHexColor(config->GetLayoutPlaylist().PlaylistTimeColor),
                                 &m_playlistTimeBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_playlistBgBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_playlistHighlightBrush);

  context->CreateSolidColorBrush(WidgetCommon::HexToColorF(config->GetLayoutPlaylist().TrackCountBoxBaseColor),
                                 &m_trackCountBoxBrush);
  context->CreateSolidColorBrush(WidgetCommon::HexToColorF(config->GetLayoutPlaylist().TrackCountBoxFontColor),
                                 &m_trackCountTextBrush);
  context->CreateSolidColorBrush(
      ParseHexColor(config->GetLayoutPlaylist().PlaylistGripLineColor),
      &m_playlistGripLineBrush);
  context->CreateSolidColorBrush(
      ParseHexColor(config->GetLayoutPlaylist().PlaylistGripArrowColor),
      &m_playlistGripArrowBrush);

  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
  context->GetFactory(&d2dFactory);

  if (d2dFactory) {
    d2dFactory->CreatePathGeometry(&m_playlistGripArrowGeometry);
    if (m_playlistGripArrowGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      if (SUCCEEDED(m_playlistGripArrowGeometry->Open(&sink))) {
        float width = config->GetLayoutPlaylist().PlaylistGripArrowWidth;
        float height = config->GetLayoutPlaylist().PlaylistGripArrowHeight;
        sink->BeginFigure(D2D1::Point2F(0.0f, -height / 2.0f),
                          D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(-width, 0.0f));
        sink->AddLine(D2D1::Point2F(0.0f, height / 2.0f));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
      }
    }

    d2dFactory->CreatePathGeometry(&m_playlistGripArrowRightGeometry);
    if (m_playlistGripArrowRightGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      if (SUCCEEDED(m_playlistGripArrowRightGeometry->Open(&sink))) {
        float width = config->GetLayoutPlaylist().PlaylistGripArrowWidth;
        float height = config->GetLayoutPlaylist().PlaylistGripArrowHeight;
        sink->BeginFigure(D2D1::Point2F(0.0f, -height / 2.0f),
                          D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(width, 0.0f));
        sink->AddLine(D2D1::Point2F(0.0f, height / 2.0f));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
      }
    }
  }

  D2D1_GRADIENT_STOP stops[2];
  D2D1_COLOR_F playingColor = ParseHexColor(config->GetUICommonParm().FocusColor);
  stops[0].color = playingColor;
  stops[0].color.a = 0.0f;
  stops[0].position = 0.0f;
  stops[1].color = playingColor;
  stops[1].color.a = 0.8f;
  stops[1].position = 1.0f;
  
  context->CreateGradientStopCollection(stops, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &m_radarGradientStops);
  if (m_radarGradientStops) {
      context->CreateLinearGradientBrush(
          D2D1::LinearGradientBrushProperties(D2D1::Point2F(0,0), D2D1::Point2F(100,0)),
          m_radarGradientStops.Get(),
          &m_radarGradientBrush
      );
  }
}

void PlaylistWidget::ReleaseResources() {
  m_playlistTitleTextFormat.Reset();
  m_playlistArtistTextFormat.Reset();
  m_playlistTimeTextFormat.Reset();
  m_toolbarTextFormat.Reset();
  m_toolbarCountTextFormat.Reset();
  m_trackCountTextFormat.Reset();
  m_toolbarIconFormat.Reset();
  m_pinSubIconFormat.Reset();
  m_playlistArtistBrush.Reset();
  m_playlistTimeBrush.Reset();
  m_playlistGripLineBrush.Reset();
  m_playlistGripArrowBrush.Reset();
  m_playlistGripArrowGeometry.Reset();
  m_playlistGripArrowRightGeometry.Reset();
  m_playlistBgBrush.Reset();
  m_playlistHighlightBrush.Reset();
  m_trackCountBoxBrush.Reset();
  m_trackCountTextBrush.Reset();
  m_textBrush.Reset();
  m_shadowBrush.Reset();
  m_radarGradientStops.Reset();
  m_radarGradientBrush.Reset();
  m_dwriteFactory.Reset();
}
