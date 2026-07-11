#include "PlaylistWidget.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <filesystem>

PlaylistWidget::PlaylistWidget()
    : m_playlistSlideX(9999.0f), m_playlistManualScrollY(0.0f) {}

void PlaylistWidget::CreateResources(ID2D1DeviceContext *context,
                                     IWICImagingFactory *wicFactory,
                                     IDWriteFactory *dwriteFactory,
                                     const ConfigManager *config) {
  if (!config)
    return;
  m_dwriteFactory = dwriteFactory;

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistTitleFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistTitleFontSize(), L"ja-jp",
      &m_playlistTitleTextFormat);

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistArtistFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistArtistFontSize(), L"ja-jp",
      &m_playlistArtistTextFormat);

  dwriteFactory->CreateTextFormat(
      config->GetPlaylistTimeFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistTimeFontSize(), L"en-us",
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
      config->GetPlaylistTitleFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetPlaylistToolbarTextFontSize(),
      L"ja-jp", &m_toolbarTextFormat);
  if (m_toolbarTextFormat) {
    m_toolbarTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_toolbarTextFormat->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      L"Segoe UI Emoji", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetPlaylistToolbarIconSize(), L"ja-jp", &m_toolbarIconFormat);
  if (m_toolbarIconFormat) {
    m_toolbarIconFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_toolbarIconFormat->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  dwriteFactory->CreateTextFormat(
      L"Segoe UI Emoji", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetPinSubIconFontSize(), L"ja-jp", &m_pinSubIconFormat);
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
      ParseHexColor(config->GetPlaylistArtistColor()), &m_playlistArtistBrush);
  context->CreateSolidColorBrush(ParseHexColor(config->GetPlaylistTimeColor()),
                                 &m_playlistTimeBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_playlistBgBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
                                 &m_playlistHighlightBrush);
  context->CreateSolidColorBrush(
      ParseHexColor(config->GetPlaylistGripLineColor()),
      &m_playlistGripLineBrush);
  context->CreateSolidColorBrush(
      ParseHexColor(config->GetPlaylistGripArrowColor()),
      &m_playlistGripArrowBrush);

  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
  context->GetFactory(&d2dFactory);

  if (d2dFactory) {
    d2dFactory->CreatePathGeometry(&m_playlistGripArrowGeometry);
    if (m_playlistGripArrowGeometry) {
      Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
      if (SUCCEEDED(m_playlistGripArrowGeometry->Open(&sink))) {
        float width = config->GetPlaylistGripArrowWidth();
        float height = config->GetPlaylistGripArrowHeight();
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
        float width = config->GetPlaylistGripArrowWidth();
        float height = config->GetPlaylistGripArrowHeight();
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
  D2D1_COLOR_F playingColor = ParseHexColor(config->GetPlayingItemColor());
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
  m_textBrush.Reset();
  m_shadowBrush.Reset();
  m_radarGradientStops.Reset();
  m_radarGradientBrush.Reset();
  m_dwriteFactory.Reset();
}

void PlaylistWidget::UpdateAnimation(const WidgetContext &ctx) {
  if (!ctx.config)
    return;

  float configPlaylistWidth =
      static_cast<float>(ctx.config->GetPlaylistWidth());
  if (m_playlistSlideX > configPlaylistWidth * 2.0f)
    m_playlistSlideX = configPlaylistWidth;

  bool isExpanded = ctx.isPlaylistHovered || ctx.config->GetIsPlaylistPinned();
  
  if (isExpanded) {
      m_playlistLeaveTimer = ctx.config->GetPlaylistLeaveDelay();
  } else {
      if (m_playlistLeaveTimer > 0.0f) {
          m_playlistLeaveTimer -= ctx.deltaTime;
          isExpanded = true;
      }
  }

  float targetSlideX = isExpanded ? 0.0f : configPlaylistWidth;
  m_playlistSlideX += (targetSlideX - m_playlistSlideX) * 0.2f;

  if (ctx.outIsPlaylistExpanded) {
      *ctx.outIsPlaylistExpanded = isExpanded || (m_playlistSlideX < configPlaylistWidth - 0.5f);
  }

  if (!isExpanded) {
    m_playlistManualScrollY = 0.0f;
  } else {
    float logicWidth = ctx.logicalWidth;
    float logicHeight = ctx.logicalHeight;
    if (logicWidth == 0.0f || logicHeight == 0.0f) {
        D2D1_SIZE_F renderTargetSize =
            D2D1::SizeF(ctx.config->GetWindowWidth() * ctx.dpiScale,
                        ctx.config->GetWindowHeight() * ctx.dpiScale);
        logicWidth = renderTargetSize.width / ctx.dpiScale;
        logicHeight = renderTargetSize.height / ctx.dpiScale;
    }

    size_t activeIndex = ctx.currentTrackIndex;
    size_t activeTotal = ctx.totalTracks;

    if (ctx.isPlaylistListViewMode) {
      if (ctx.availablePlaylistsCache) {
        activeTotal = ctx.availablePlaylistsCache->size();
        std::wstring currentPlaylist = ctx.config->GetDefaultPlaylistPath();
        activeIndex = 0;
        for (size_t i = 0; i < ctx.availablePlaylistsCache->size(); ++i) {
          if ((*ctx.availablePlaylistsCache)[i].filepath == currentPlaylist) {
            activeIndex = i;
            break;
          }
        }
      } else {
        activeTotal = 0;
        activeIndex = 0;
      }
    }

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicWidth, logicHeight, ctx.config, m_playlistSlideX,
        m_playlistManualScrollY, activeIndex, activeTotal);
    m_playlistManualScrollY = layout.newManualScrollY;
  }

  if (m_lastTrackIndex != static_cast<size_t>(-1) && m_lastTrackIndex != ctx.currentTrackIndex && !ctx.isPlaylistListViewMode) {
      m_isScanlineActive = true;
      m_scanlineProgress = 0.0f;
  }
  m_lastTrackIndex = ctx.currentTrackIndex;

  if (m_isScanlineActive) {
      m_scanlineProgress += ctx.deltaTime * 2.0f;
      if (m_scanlineProgress >= 1.0f) {
          m_isScanlineActive = false;
          m_scanlineProgress = 1.0f;
      }
  }

  float fadeOutSpeed = ctx.config->GetHoverFadeOutSpeed() * ctx.deltaTime;
  float fadeInSpeed = 10.0f * ctx.deltaTime;

  for (auto it = m_hoverAlpha.begin(); it != m_hoverAlpha.end(); ) {
      if (it->first == ctx.playlistHoveredItemIndex) {
          it->second += fadeInSpeed;
          if (it->second > 1.0f) it->second = 1.0f;
          ++it;
      } else {
          it->second -= fadeOutSpeed;
          if (it->second <= 0.0f) {
              it = m_hoverAlpha.erase(it);
          } else {
              ++it;
          }
      }
  }

  if (ctx.playlistHoveredItemIndex >= 0 && m_hoverAlpha.find(ctx.playlistHoveredItemIndex) == m_hoverAlpha.end()) {
      m_hoverAlpha[ctx.playlistHoveredItemIndex] = fadeInSpeed > 1.0f ? 1.0f : fadeInSpeed;
  }
}

void PlaylistWidget::UpdateLayout(const WidgetContext &ctx,
                                  const ConfigManager *config) {}

void PlaylistWidget::AddScroll(float delta) {
  m_playlistManualScrollY += delta;
}

float PlaylistWidget::GetScrollY() const { return m_playlistManualScrollY; }

void PlaylistWidget::Draw(ID2D1DeviceContext *context, const WidgetContext &ctx,
                          const ConfigManager *config) {
  if (!config || !m_textBrush)
    return;

  D2D1_SIZE_F renderTargetSize = context->GetSize();
  float logicWidth = renderTargetSize.width / ctx.dpiScale;
  float logicHeight = renderTargetSize.height / ctx.dpiScale;

  size_t activeIndex = ctx.currentTrackIndex;
  size_t activeTotal = ctx.totalTracks;

  if (ctx.isPlaylistListViewMode && config) {
    if (ctx.availablePlaylistsCache) {
      activeTotal = ctx.availablePlaylistsCache->size();
      std::wstring currentPlaylist = config->GetDefaultPlaylistPath();
      activeIndex = 0;
      for (size_t i = 0; i < ctx.availablePlaylistsCache->size(); ++i) {
        if ((*ctx.availablePlaylistsCache)[i].filepath == currentPlaylist) {
          activeIndex = i;
          break;
        }
      }
    } else {
      activeTotal = 0;
      activeIndex = 0;
    }
  }

  PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
      logicWidth, logicHeight, config, m_playlistSlideX,
      m_playlistManualScrollY, activeIndex, activeTotal);

  float viewHeight = layout.clipRect.bottom - layout.clipRect.top;
  if (m_lastViewHeight != 0.0f && m_lastViewHeight != viewHeight) {
      float diff = (viewHeight - m_lastViewHeight) / 2.0f;
      m_playlistManualScrollY -= diff;
      layout = LayoutCalculator::CalculatePlaylistLayout(
          logicWidth, logicHeight, config, m_playlistSlideX,
          m_playlistManualScrollY, activeIndex, activeTotal);
  }
  m_lastViewHeight = viewHeight;

  ID2D1PathGeometry *arrowGeometry =
      config->GetPlaylistPosition() == 0
          ? m_playlistGripArrowRightGeometry.Get()
          : m_playlistGripArrowGeometry.Get();
  if (m_playlistGripLineBrush && m_playlistGripArrowBrush && arrowGeometry) {
    if (m_shadowBrush && config->GetPlaylistGripShadowOpacity() > 0.0f) {
      m_shadowBrush->SetOpacity(config->GetPlaylistGripShadowOpacity());

      context->DrawLine(D2D1::Point2F(layout.gripShadowX, layout.playlistY),
                        D2D1::Point2F(layout.gripShadowX,
                                      layout.playlistY + layout.playlistHeight),
                        m_shadowBrush.Get(), layout.gripLineWidth);

      if (!config->GetIsPlaylistPinned()) {
        D2D1_MATRIX_3X2_F shadowTransform = D2D1::Matrix3x2F::Translation(
            layout.gripShadowX,
            layout.gripShadowY + layout.playlistHeight / 2.0f);
        context->SetTransform(shadowTransform * D2D1::Matrix3x2F::Scale(
                                                    ctx.dpiScale, ctx.dpiScale));
        context->FillGeometry(arrowGeometry, m_shadowBrush.Get());
        context->SetTransform(
            D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
      }
    }

    context->DrawLine(
        D2D1::Point2F(layout.gripX, layout.playlistY),
        D2D1::Point2F(layout.gripX, layout.playlistY + layout.playlistHeight),
        m_playlistGripLineBrush.Get(), layout.gripLineWidth);

    if (!config->GetIsPlaylistPinned()) {
      D2D1_MATRIX_3X2_F arrowTransform = D2D1::Matrix3x2F::Translation(
          layout.gripX, layout.playlistY + layout.playlistHeight / 2.0f);
      context->SetTransform(arrowTransform *
                            D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
      context->FillGeometry(arrowGeometry, m_playlistGripArrowBrush.Get());
      context->SetTransform(D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
    }
  }

  if (m_playlistSlideX < layout.playlistWidth - 0.5f) {
    if (m_playlistBgBrush) {
      m_playlistBgBrush->SetOpacity(config->GetPlaylistBgOpacity());
      context->FillRectangle(&layout.bgRect, m_playlistBgBrush.Get());
    }

    // Draw Toolbar
    std::wstring hoverText = L"";
    std::wstring icons[3];
    if (ctx.isPlaylistListViewMode) {
      icons[0] = L""; // (非表示)
      icons[1] = L"➕";
      icons[2] = L"🗑️";
    } else {
      icons[0] = L"📁";
      icons[1] = L"➖";
      icons[2] = L"🗑️";
    }

    if (ctx.playlistToolbarHoveredIndex >= 0 &&
        ctx.playlistToolbarHoveredIndex <= 2) {
      int idx = ctx.playlistToolbarHoveredIndex;
      if (ctx.isPlaylistListViewMode) {
        if (idx == 0)
          hoverText = L""; // (非表示)
        if (idx == 1)
          hoverText = L"プレイリストを新規作成する";
        if (idx == 2)
          hoverText = L"プレイリストを削除する";
      } else {
        if (idx == 0)
          hoverText = L"プレイリスト一覧を開く";
        if (idx == 1)
          hoverText = L"再生中の曲をプレイリストから削除する";
        if (idx == 2)
          hoverText = L"プレイリストの全曲を削除する";
      }
    }
    
    if (ctx.isPlaylistPinnedHovered) {
      hoverText = config->GetIsPlaylistPinned() ? L"表示モード: 画面固定" : L"表示モード: 自動格納";
    }

    std::wstring toolbarCenterText = hoverText;
    if (toolbarCenterText.empty() && !ctx.isPlaylistListViewMode) {
      std::wstring currentPlaylist = config->GetDefaultPlaylistPath();
      if (!currentPlaylist.empty()) {
        try {
          toolbarCenterText = std::filesystem::path(currentPlaylist).stem().wstring();
        } catch (...) {
          toolbarCenterText = L"Playlist";
        }
      }
    }

    if (!toolbarCenterText.empty() && m_toolbarTextFormat && m_textBrush) {
      if (m_shadowBrush && config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(config->GetShadowOpacity());
        D2D1_RECT_F sRect = layout.toolbarLayout.textRect;
        sRect.left += 1.0f;
        sRect.top += 1.0f;
        sRect.right += 1.0f;
        sRect.bottom += 1.0f;
        context->DrawText(
            toolbarCenterText.c_str(), static_cast<UINT32>(toolbarCenterText.length()),
            m_toolbarTextFormat.Get(), &sRect, m_shadowBrush.Get());
      }
      context->DrawText(toolbarCenterText.c_str(),
                        static_cast<UINT32>(toolbarCenterText.length()),
                        m_toolbarTextFormat.Get(),
                        &layout.toolbarLayout.textRect, m_textBrush.Get());
    }

    for (int i = 0; i < 3; ++i) {
      if (ctx.isPlaylistListViewMode && i == 0)
        continue; // (非表示)

      if (ctx.playlistToolbarHoveredIndex == i && m_playlistHighlightBrush) {
        m_playlistHighlightBrush->SetOpacity(0.2f);
        context->FillRectangle(&layout.toolbarLayout.buttonHitRects[i],
                               m_playlistHighlightBrush.Get());
      }

      if (m_toolbarIconFormat && !icons[i].empty()) {
        context->DrawText(
            icons[i].c_str(), static_cast<UINT32>(icons[i].length()),
            m_toolbarIconFormat.Get(), &layout.toolbarLayout.buttonHitRects[i],
            m_textBrush.Get());
      }
    }

    // ピン留めボタンの描画
    if (ctx.isPlaylistPinnedHovered && m_playlistHighlightBrush) {
      m_playlistHighlightBrush->SetOpacity(0.2f);
      context->FillRectangle(&layout.toolbarLayout.pinButtonHitRect, m_playlistHighlightBrush.Get());
    }

    bool isPinned = config->GetIsPlaylistPinned();
    std::wstring pinBaseIcon = L"📌";
    std::wstring pinSubIcon = isPinned ? L"🔒" : L"🔓";

    if (m_toolbarIconFormat && m_textBrush) {
      m_textBrush->SetOpacity(isPinned ? 1.0f : 0.4f);
      context->DrawText(pinBaseIcon.c_str(), static_cast<UINT32>(pinBaseIcon.length()),
                        m_toolbarIconFormat.Get(), &layout.toolbarLayout.pinButtonHitRect,
                        m_textBrush.Get());
      m_textBrush->SetOpacity(1.0f);
    }

    if (m_pinSubIconFormat && m_textBrush && m_shadowBrush) {
      D2D1_RECT_F subRect = layout.toolbarLayout.pinButtonHitRect;
      subRect.left += config->GetPinSubIconOffsetX();
      subRect.right += config->GetPinSubIconOffsetX();
      subRect.top += config->GetPinSubIconOffsetY();
      subRect.bottom += config->GetPinSubIconOffsetY();

      m_shadowBrush->SetOpacity(1.0f);
      D2D1_RECT_F outlineRects[4] = {
          {subRect.left - 1, subRect.top, subRect.right - 1, subRect.bottom},
          {subRect.left + 1, subRect.top, subRect.right + 1, subRect.bottom},
          {subRect.left, subRect.top - 1, subRect.right, subRect.bottom - 1},
          {subRect.left, subRect.top + 1, subRect.right, subRect.bottom + 1}};
      for (int i = 0; i < 4; ++i) {
        context->DrawText(pinSubIcon.c_str(), static_cast<UINT32>(pinSubIcon.length()),
                          m_pinSubIconFormat.Get(), &outlineRects[i], m_shadowBrush.Get());
      }
      context->DrawText(pinSubIcon.c_str(), static_cast<UINT32>(pinSubIcon.length()),
                        m_pinSubIconFormat.Get(), &subRect, m_textBrush.Get());
    }

    context->PushAxisAlignedClip(&layout.clipRect,
                                 D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    float currentY = layout.startY;

    if (m_playlistHighlightBrush) {
      m_playlistHighlightBrush->SetOpacity(0.2f);
    }

    D2D1_COLOR_F originalTextColor = m_textBrush->GetColor();
    auto GetBlendedTextColor = [&](int index, bool isPlaying) {
        D2D1_COLOR_F baseColor = isPlaying ? ParseHexColor(config->GetPlayingItemColor()) : originalTextColor;
        float t = 0.0f;
        auto it = m_hoverAlpha.find(index);
        if (it != m_hoverAlpha.end()) t = it->second;

        if (t <= 0.0f) return baseColor;

        D2D1_COLOR_F hoverColor = ParseHexColor(config->GetHoverItemColor());
        return D2D1_COLOR_F{
            baseColor.r + (hoverColor.r - baseColor.r) * t,
            baseColor.g + (hoverColor.g - baseColor.g) * t,
            baseColor.b + (hoverColor.b - baseColor.b) * t,
            baseColor.a
        };
    };

    if (ctx.isPlaylistListViewMode) {
      if (ctx.availablePlaylistsCache) {
        std::wstring currentPlaylist = config->GetDefaultPlaylistPath();

        for (size_t i = 0; i < ctx.availablePlaylistsCache->size(); ++i) {
        if (currentY + layout.itemHeight > 0 &&
            currentY < layout.playlistHeight) {
          PlaylistItemLayout itemLayout =
              LayoutCalculator::CalculatePlaylistItemLayout(layout, config,
                                                            currentY);

          bool isPlaying = ((*ctx.availablePlaylistsCache)[i].filepath == currentPlaylist);
          bool isFocused = ctx.focusedPlaylistIndex && *ctx.focusedPlaylistIndex == i;
          if ((isPlaying || isFocused) && m_playlistHighlightBrush) {
            float opacity = isPlaying ? 0.2f : 0.1f;
            m_playlistHighlightBrush->SetOpacity(opacity);
            context->FillRectangle(&itemLayout.hlRect,
                                   m_playlistHighlightBrush.Get());
          }

          std::wstring title = (*ctx.availablePlaylistsCache)[i].displayName;
          std::wstring info = std::to_wstring((*ctx.availablePlaylistsCache)[i].trackCount) + L" tracks";
          std::wstring timeStr = (*ctx.availablePlaylistsCache)[i].totalTimeString;

          m_textBrush->SetColor(GetBlendedTextColor(static_cast<int>(i), isPlaying));

          context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                            m_playlistTitleTextFormat.Get(),
                            &itemLayout.titleRect, m_textBrush.Get());

          if (!info.empty()) {
            context->DrawText(
                info.c_str(), static_cast<UINT32>(info.length()),
                m_playlistArtistTextFormat.Get(), &itemLayout.artistRect,
                m_playlistArtistBrush ? m_playlistArtistBrush.Get()
                                      : m_textBrush.Get());
          }

          if (!timeStr.empty() && m_playlistTimeTextFormat) {
            D2D1_RECT_F timeRect =
                D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                            itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                            itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
            context->DrawText(timeStr.c_str(),
                              static_cast<UINT32>(timeStr.length()),
                              m_playlistTimeTextFormat.Get(), &timeRect,
                              m_playlistTimeBrush ? m_playlistTimeBrush.Get()
                                                  : m_textBrush.Get());
          }
        }
        currentY += layout.itemHeight;
      }
      }
    } else {
      for (size_t i = 0; i < ctx.totalTracks && ctx.shuffleMetadataList &&
                         i < ctx.shuffleMetadataList->size();
           ++i) {
        if (currentY + layout.itemHeight > 0 &&
            currentY < layout.playlistHeight) {
          PlaylistItemLayout itemLayout =
              LayoutCalculator::CalculatePlaylistItemLayout(layout, config,
                                                            currentY);

          bool isPlaying = (i == ctx.currentTrackIndex);
          bool isFocused = ctx.focusedPlaylistIndex && *ctx.focusedPlaylistIndex == i;
          if ((isPlaying || isFocused) && m_playlistHighlightBrush) {
            float opacity = isPlaying ? 0.2f : 0.1f;
            m_playlistHighlightBrush->SetOpacity(opacity);
            context->FillRectangle(&itemLayout.hlRect,
                                   m_playlistHighlightBrush.Get());
          }

          const TrackMetadata &meta = (*ctx.shuffleMetadataList)[i];
          std::wstring title;
          std::wstring artist;
          std::wstring timeStr;

          if (meta.isLoaded) {
            title = meta.title;
            artist = meta.artist;
            timeStr = meta.timeString;
          } else {
            try {
              title = std::filesystem::path(meta.filepath).filename().wstring();
            } catch (...) {
              title = L"Unknown";
            }
            artist = L"Unknown Artist";
          }

          m_textBrush->SetColor(GetBlendedTextColor(static_cast<int>(i), isPlaying && !m_isScanlineActive));

          context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                            m_playlistTitleTextFormat.Get(),
                            &itemLayout.titleRect, m_textBrush.Get());

          if (!artist.empty()) {
            context->DrawText(
                artist.c_str(), static_cast<UINT32>(artist.length()),
                m_playlistArtistTextFormat.Get(), &itemLayout.artistRect,
                m_playlistArtistBrush ? m_playlistArtistBrush.Get()
                                      : m_textBrush.Get());
          }

          if (!timeStr.empty() && m_playlistTimeTextFormat) {
            D2D1_RECT_F timeRect =
                D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                            itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                            itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
            context->DrawText(timeStr.c_str(),
                              static_cast<UINT32>(timeStr.length()),
                              m_playlistTimeTextFormat.Get(), &timeRect,
                              m_playlistTimeBrush ? m_playlistTimeBrush.Get()
                                                  : m_textBrush.Get());
          }

          if (isPlaying && m_isScanlineActive && m_radarGradientBrush) {
              float startX = itemLayout.hlRect.left;
              float endX = itemLayout.hlRect.right;
              float tailLength = 150.0f * ctx.dpiScale;
              
              float currentX = startX + (endX - startX + tailLength) * m_scanlineProgress;
              float tailStart = currentX - tailLength;
              
              context->PushAxisAlignedClip(&itemLayout.hlRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
              
              m_radarGradientBrush->SetStartPoint(D2D1::Point2F(tailStart, itemLayout.hlRect.top));
              m_radarGradientBrush->SetEndPoint(D2D1::Point2F(currentX, itemLayout.hlRect.top));
              D2D1_RECT_F tailRect = D2D1::RectF(tailStart, itemLayout.hlRect.top, currentX, itemLayout.hlRect.bottom);
              context->FillRectangle(&tailRect, m_radarGradientBrush.Get());
              
              D2D1_RECT_F textClipRect = D2D1::RectF(startX, itemLayout.hlRect.top, currentX, itemLayout.hlRect.bottom);
              context->PushAxisAlignedClip(&textClipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
              
              m_textBrush->SetColor(ParseHexColor(config->GetPlayingItemColor()));
              context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                                m_playlistTitleTextFormat.Get(), &itemLayout.titleRect, m_textBrush.Get());
              if (!artist.empty()) {
                  context->DrawText(artist.c_str(), static_cast<UINT32>(artist.length()),
                                    m_playlistArtistTextFormat.Get(), &itemLayout.artistRect, m_textBrush.Get());
              }
              if (!timeStr.empty() && m_playlistTimeTextFormat) {
                  D2D1_RECT_F timeRect = D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                                                     itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                                                     itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
                  context->DrawText(timeStr.c_str(), static_cast<UINT32>(timeStr.length()),
                                    m_playlistTimeTextFormat.Get(), &timeRect, m_textBrush.Get());
              }
              
              context->PopAxisAlignedClip(); // Pop text clip
              context->PopAxisAlignedClip(); // Pop hlRect clip
          }
        }
        currentY += layout.itemHeight;
      }
    }

    m_textBrush->SetColor(originalTextColor); // Restore

    context->PopAxisAlignedClip();
  }
}

void PlaylistWidget::ForceClearHoverDelay() {
    m_playlistLeaveTimer = 0.0f;
}
