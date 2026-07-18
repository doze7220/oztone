#include "Widget_TrackInfo.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <cwchar>

// ==========================================
// TrackInfoWidget
// ==========================================
void TrackInfoWidget::CreateResources(ID2D1DeviceContext *context,
                                      IWICImagingFactory *wicFactory,
                                      IDWriteFactory *dwriteFactory,
                                      const ConfigManager *config) {
  dwriteFactory->CreateTextFormat(
      config->GetBaseFontFamily().c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetTitleFontSize(), L"ja-jp", &m_titleTextFormat);
  dwriteFactory->CreateTextFormat(
      config->GetBaseFontFamily().c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      config->GetArtistFontSize(), L"ja-jp", &m_artistTextFormat);

  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_titleTextFormat.Get());
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_artistTextFormat.Get());

  m_dwriteFactory = dwriteFactory;
  dwriteFactory->CreateTextFormat(
      config->GetMonoFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetTrackCountFontSize(), L"en-us",
      &m_trackCountTextFormat);
  WidgetCommon::ApplyTextTrimming(dwriteFactory, m_trackCountTextFormat.Get());

  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_shadowBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                 &m_textBrush);
  context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                 &m_fallbackBlackBrush);

  D2D1_COLOR_F baseColor = WidgetCommon::HexToColorF(config->GetTrackCountBoxBaseColor());
  baseColor.a = config->GetTrackCountBoxBaseOpacity();
  context->CreateSolidColorBrush(baseColor, &m_trackCountBoxBaseBrush);

  D2D1_COLOR_F fontColor = WidgetCommon::HexToColorF(config->GetTrackCountBoxFontColor());
  context->CreateSolidColorBrush(fontColor, &m_trackCountTextBrush);
  context->CreateSolidColorBrush(baseColor, &m_trackCountBoxUnderLineBrush);
}

void TrackInfoWidget::ReleaseResources() {
  m_titleTextFormat.Reset();
  m_artistTextFormat.Reset();
  m_titleTextLayout.Reset();
  m_artistTextLayout.Reset();
  m_shadowBrush.Reset();
  m_textBrush.Reset();
  m_fallbackBlackBrush.Reset();
  m_trackCountTextFormat.Reset();
  m_trackCountTextLayout.Reset();
  m_dwriteFactory.Reset();
  m_trackCountBoxBaseBrush.Reset();
  m_trackCountBoxUnderLineBrush.Reset();
  m_trackCountTextBrush.Reset();
}

void TrackInfoWidget::UpdateAnimation(const WidgetContext &ctx) {
  if (!ctx.isDrumAnimating) {
    if (!m_lastArtBitmap && ctx.currentArtBitmap) {
      m_artCrossfadeProgress = 0.0f;
    } else if (m_artCrossfadeProgress < 1.0f) {
      m_artCrossfadeProgress += ctx.deltaTime * 3.0f; // 300ms fade
      if (m_artCrossfadeProgress > 1.0f) m_artCrossfadeProgress = 1.0f;
    }
  } else {
    m_artCrossfadeProgress = 1.0f;
  }
  m_lastArtBitmap = ctx.currentArtBitmap;
  m_wasDrumAnimating = ctx.isDrumAnimating;
}

void TrackInfoWidget::UpdateLayout(const WidgetContext &ctx,
                                   const ConfigManager *config) {
  if (!config)
    return;

  if (m_dwriteFactory) {
    if (m_titleTextFormat &&
        (!m_titleTextLayout || m_lastTitle != ctx.trackTitle)) {
      m_lastTitle = ctx.trackTitle;
      m_titleTextLayout.Reset();
      m_dwriteFactory->CreateTextLayout(
          ctx.trackTitle.c_str(), static_cast<UINT32>(ctx.trackTitle.length()),
          m_titleTextFormat.Get(), 4000.0f, 1000.0f, &m_titleTextLayout);
    }

    if (m_artistTextFormat &&
        (!m_artistTextLayout || m_lastArtist != ctx.trackArtist)) {
      m_lastArtist = ctx.trackArtist;
      m_artistTextLayout.Reset();
      m_dwriteFactory->CreateTextLayout(
          ctx.trackArtist.c_str(),
          static_cast<UINT32>(ctx.trackArtist.length()),
          m_artistTextFormat.Get(), 4000.0f, 1000.0f, &m_artistTextLayout);
    }
    
    if (m_titleTextFormat &&
        (!m_oldTitleTextLayout || m_lastOldTitle != ctx.oldTrackTitle)) {
      m_lastOldTitle = ctx.oldTrackTitle;
      m_oldTitleTextLayout.Reset();
      m_dwriteFactory->CreateTextLayout(
          ctx.oldTrackTitle.c_str(), static_cast<UINT32>(ctx.oldTrackTitle.length()),
          m_titleTextFormat.Get(), 4000.0f, 1000.0f, &m_oldTitleTextLayout);
    }

    if (m_artistTextFormat &&
        (!m_oldArtistTextLayout || m_lastOldArtist != ctx.oldTrackArtist)) {
      m_lastOldArtist = ctx.oldTrackArtist;
      m_oldArtistTextLayout.Reset();
      m_dwriteFactory->CreateTextLayout(
          ctx.oldTrackArtist.c_str(),
          static_cast<UINT32>(ctx.oldTrackArtist.length()),
          m_artistTextFormat.Get(), 4000.0f, 1000.0f, &m_oldArtistTextLayout);
    }
  }

  if (m_dwriteFactory && m_trackCountTextFormat &&
      (!m_trackCountTextLayout || m_lastTotalTracks != ctx.totalTracks ||
       m_lastCurrentTrackIndex != ctx.currentTrackIndex)) {
    m_lastTotalTracks = ctx.totalTracks;
    m_lastCurrentTrackIndex = ctx.currentTrackIndex;
    m_trackCountTextLayout.Reset();

    wchar_t trackCountBuf[64];
    if (ctx.totalTracks == 0) {
      swprintf_s(trackCountBuf, L"---");
    } else {
      size_t displayNo = ctx.currentTrackIndex + 1; // フォールバック
      if (!ctx.shuffleIndices.empty() && ctx.currentTrackIndex < ctx.shuffleIndices.size()) {
          displayNo = ctx.shuffleIndices[ctx.currentTrackIndex] + 1;
      }
      swprintf_s(trackCountBuf, L"%zu", displayNo);
    }
    std::wstring trackCountStr(trackCountBuf);

    if (m_trackCountTextFormat) {
      m_trackCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    m_dwriteFactory->CreateTextLayout(
        trackCountStr.c_str(), static_cast<UINT32>(trackCountStr.length()),
        m_trackCountTextFormat.Get(), static_cast<float>(config->GetArtSize()), config->GetTrackCountBoxWidth(), &m_trackCountTextLayout);

    if (m_trackCountTextLayout) {
      Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
      if (SUCCEEDED(m_trackCountTextLayout.As(&textLayout1))) {
        DWRITE_TEXT_RANGE textRange = {
            0, static_cast<UINT32>(trackCountStr.length())};
        textLayout1->SetCharacterSpacing(
            0.0f, config->GetTrackCountLetterSpacing(), 0.0f, textRange);
      }
    }
  }

  if (m_dwriteFactory && m_trackCountTextFormat &&
      (!m_oldTrackCountTextLayout || m_lastTotalTracks != ctx.totalTracks ||
       m_lastOldTrackIndex != ctx.oldTrackIndex)) {
    m_lastTotalTracks = ctx.totalTracks;
    m_lastOldTrackIndex = ctx.oldTrackIndex;
    m_oldTrackCountTextLayout.Reset();

    wchar_t trackCountBuf[64];
    if (ctx.totalTracks == 0 || ctx.oldTrackIndex == static_cast<size_t>(-1) || ctx.oldTrackIndex == static_cast<size_t>(-2)) {
      swprintf_s(trackCountBuf, L"---");
    } else {
      size_t displayNo = ctx.oldTrackIndex + 1;
      if (!ctx.shuffleIndices.empty() && ctx.oldTrackIndex < ctx.shuffleIndices.size()) {
          displayNo = ctx.shuffleIndices[ctx.oldTrackIndex] + 1;
      }
      swprintf_s(trackCountBuf, L"%zu", displayNo);
    }
    std::wstring trackCountStr(trackCountBuf);

    m_dwriteFactory->CreateTextLayout(
        trackCountStr.c_str(), static_cast<UINT32>(trackCountStr.length()),
        m_trackCountTextFormat.Get(), static_cast<float>(config->GetArtSize()), config->GetTrackCountBoxWidth(), &m_oldTrackCountTextLayout);

    if (m_oldTrackCountTextLayout) {
      Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
      if (SUCCEEDED(m_oldTrackCountTextLayout.As(&textLayout1))) {
        DWRITE_TEXT_RANGE textRange = {
            0, static_cast<UINT32>(trackCountStr.length())};
        textLayout1->SetCharacterSpacing(
            0.0f, config->GetTrackCountLetterSpacing(), 0.0f, textRange);
      }
    }
  }
}

void TrackInfoWidget::Draw(ID2D1DeviceContext *context,
                           const WidgetContext &ctx,
                           const ConfigManager *config) {
  if (config && config->GetShowNowPlaying()) {
    D2D1_SIZE_F rtSize = context->GetSize();
    float logicWidth = rtSize.width / ctx.dpiScale;
    float logicHeight = rtSize.height / ctx.dpiScale;

    D2D1_SIZE_F bitmapSize = ctx.currentArtBitmap
                                 ? ctx.currentArtBitmap->GetSize()
                                 : D2D1::SizeF(0.0f, 0.0f);
    TrackInfoLayout layout = LayoutCalculator::CalculateTrackInfoLayout(
        logicWidth, logicHeight, config, bitmapSize);

    float slotHeight = config->GetArtSize() + config->GetShadowOffsetY() + 1.0f;

    D2D1_RECT_F drumClipRect = layout.clipRect;
    drumClipRect.top = layout.fallbackArtRect.top;
    drumClipRect.bottom = layout.fallbackArtRect.top + slotHeight;
    context->PushAxisAlignedClip(drumClipRect, D2D1_ANTIALIAS_MODE_ALIASED);

    auto drawDrumItem = [&](size_t trackIndex, double diffOffset) {
      if (std::abs(diffOffset) > 2.0) return;

      float offsetY = static_cast<float>(diffOffset) * slotHeight;
      D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(0.0f, offsetY);
      D2D1::Matrix3x2F originalTransform;
      context->GetTransform(&originalTransform);
      context->SetTransform(transform * originalTransform);

      ID2D1Bitmap* art = nullptr;
      IDWriteTextLayout* titleLayout = nullptr;
      IDWriteTextLayout* artistLayout = nullptr;
      IDWriteTextLayout* trackCountLayout = nullptr;

      Microsoft::WRL::ComPtr<IDWriteTextLayout> tempTrackCountLayout;
      Microsoft::WRL::ComPtr<IDWriteTextLayout> tempTitleLayout;
      Microsoft::WRL::ComPtr<IDWriteTextLayout> tempArtistLayout;

      if (trackIndex == ctx.drumStartIndex) {
        art = ctx.oldArtBitmap;
        titleLayout = m_oldTitleTextLayout.Get();
        artistLayout = m_oldArtistTextLayout.Get();
        trackCountLayout = m_oldTrackCountTextLayout.Get();
      } else if (trackIndex == ctx.drumTargetIndex) {
        art = ctx.currentArtBitmap;
        titleLayout = m_titleTextLayout.Get();
        artistLayout = m_artistTextLayout.Get();
        trackCountLayout = m_trackCountTextLayout.Get();
      } else {
        // 中間スロット：メタデータからタイトル・アーティスト名と、CD帯(トラックナンバー)のテキストを生成
        if (ctx.totalTracks > 0) {
            std::wstring midTitle;
            std::wstring midArtist;
            if (ctx.shuffleMetadataList && trackIndex < ctx.shuffleMetadataList->size()) {
                const auto& meta = (*ctx.shuffleMetadataList)[trackIndex];
                midTitle = meta.title;
                midArtist = meta.artist;
            }

            if (!midTitle.empty() && m_dwriteFactory && m_titleTextFormat) {
                m_dwriteFactory->CreateTextLayout(
                    midTitle.c_str(), static_cast<UINT32>(midTitle.length()),
                    m_titleTextFormat.Get(), 4000.0f, 1000.0f, &tempTitleLayout);
                titleLayout = tempTitleLayout.Get();
            }
            if (!midArtist.empty() && m_dwriteFactory && m_artistTextFormat) {
                m_dwriteFactory->CreateTextLayout(
                    midArtist.c_str(), static_cast<UINT32>(midArtist.length()),
                    m_artistTextFormat.Get(), 4000.0f, 1000.0f, &tempArtistLayout);
                artistLayout = tempArtistLayout.Get();
            }

            wchar_t trackCountBuf[64];
            size_t displayNo = trackIndex + 1;
            if (!ctx.shuffleIndices.empty() && trackIndex < ctx.shuffleIndices.size()) {
                displayNo = ctx.shuffleIndices[trackIndex] + 1;
            }
            swprintf_s(trackCountBuf, L"%zu", displayNo);
            std::wstring trackCountStr(trackCountBuf);

            m_dwriteFactory->CreateTextLayout(
                trackCountStr.c_str(), static_cast<UINT32>(trackCountStr.length()),
                m_trackCountTextFormat.Get(), static_cast<float>(config->GetArtSize()), config->GetTrackCountBoxWidth(), &tempTrackCountLayout);
            
            Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
            if (SUCCEEDED(tempTrackCountLayout.As(&textLayout1))) {
                DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(trackCountStr.length())};
                textLayout1->SetCharacterSpacing(0.0f, config->GetTrackCountLetterSpacing(), 0.0f, textRange);
            }
            trackCountLayout = tempTrackCountLayout.Get();
        }
      }

      bool drawGlass = (art == nullptr);
      float artOpacity = 1.0f;
      float glassAlphaMultiplier = 1.0f;

      if (drawGlass) {
          artOpacity = 0.0f;
      } else if (trackIndex == ctx.drumTargetIndex && m_artCrossfadeProgress < 1.0f) {
          drawGlass = true;
          artOpacity = m_artCrossfadeProgress;
          glassAlphaMultiplier = 1.0f - m_artCrossfadeProgress;
      }

      if (drawGlass && m_fallbackBlackBrush) {
        m_fallbackBlackBrush->SetOpacity(config->GetFallbackArtOpacity() * glassAlphaMultiplier);
        context->FillRectangle(&layout.fallbackArtRect, m_fallbackBlackBrush.Get());
      }

      if (artOpacity > 0.0f && art) {
        D2D1_RECT_F itemArtDestRect = layout.fallbackArtRect;
        D2D1_RECT_F itemArtShadowRect = layout.fallbackArtRect;
        D2D1_SIZE_F bitmapSize = art->GetSize();
        if (bitmapSize.width > 0 && bitmapSize.height > 0) {
            float size = static_cast<float>(config->GetArtSize());
            float scaleX = size / bitmapSize.width;
            float scaleY = size / bitmapSize.height;
            float scale = (scaleX < scaleY) ? scaleX : scaleY;
            
            float drawWidth = bitmapSize.width * scale;
            float drawHeight = bitmapSize.height * scale;
            
            float drawX = layout.fallbackArtRect.left + (size - drawWidth) / 2.0f;
            float drawY = layout.fallbackArtRect.top + (size - drawHeight) / 2.0f;
            
            itemArtDestRect = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);
            itemArtShadowRect = D2D1::RectF(
                drawX + config->GetShadowOffsetX(),
                drawY + config->GetShadowOffsetY(),
                drawX + drawWidth + config->GetShadowOffsetX(),
                drawY + drawHeight + config->GetShadowOffsetY()
            );
        }

        if (m_shadowBrush && config->GetEnableShadow()) {
          m_shadowBrush->SetOpacity(config->GetShadowOpacity() * artOpacity);
          context->FillRectangle(&itemArtShadowRect, m_shadowBrush.Get());
        }
        context->DrawBitmap(art, &itemArtDestRect, artOpacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
      }

      if (m_textBrush && titleLayout && artistLayout) {
        titleLayout->SetMaxWidth(layout.titleRect.right - layout.titleRect.left);
        titleLayout->SetMaxHeight(layout.titleRect.bottom - layout.titleRect.top);
        artistLayout->SetMaxWidth(layout.artistRect.right - layout.artistRect.left);
        artistLayout->SetMaxHeight(layout.artistRect.bottom - layout.artistRect.top);

        float shadowOpacity = config->GetEnableShadow() ? config->GetShadowOpacity() : 0.0f;
        WidgetCommon::DrawShadowedTextLayout(
            context, titleLayout, m_textBrush.Get(), m_shadowBrush.Get(),
            D2D1::Point2F(layout.titleRect.left, layout.titleRect.top),
            D2D1::Point2F(layout.titleShadowRect.left, layout.titleShadowRect.top),
            shadowOpacity);

        WidgetCommon::DrawShadowedTextLayout(
            context, artistLayout, m_textBrush.Get(), m_shadowBrush.Get(),
            D2D1::Point2F(layout.artistRect.left, layout.artistRect.top),
            D2D1::Point2F(layout.artistShadowRect.left, layout.artistShadowRect.top),
            shadowOpacity);
      }

      if (trackCountLayout && m_trackCountTextBrush && m_trackCountBoxBaseBrush) {
        D2D1::Matrix3x2F currentTransform;
        context->GetTransform(&currentTransform);

        D2D1::Matrix3x2F rotation = D2D1::Matrix3x2F::Rotation(-90.0f, layout.trackCountOrigin);
        context->SetTransform(rotation * currentTransform);

        D2D1_RECT_F boxRect = D2D1::RectF(
            layout.trackCountOrigin.x,
            layout.trackCountOrigin.y,
            layout.trackCountOrigin.x + layout.trackCountMaxWidth,
            layout.trackCountOrigin.y + config->GetTrackCountBoxWidth()
        );
        context->FillRectangle(&boxRect, m_trackCountBoxBaseBrush.Get());

        D2D1_RECT_F underlineRect = D2D1::RectF(
            layout.trackCountOrigin.x,
            layout.trackCountOrigin.y + config->GetTrackCountUnderLineX(),
            layout.trackCountOrigin.x + layout.trackCountMaxWidth,
            layout.trackCountOrigin.y + config->GetTrackCountUnderLineX() + config->GetTrackCountUnderLineWidth()
        );
        context->FillRectangle(&underlineRect, m_trackCountBoxUnderLineBrush.Get());

        context->DrawTextLayout(
            layout.trackCountOrigin, trackCountLayout, m_trackCountTextBrush.Get());

        context->SetTransform(currentTransform);
      }

      context->SetTransform(originalTransform);
    };

    int startSlot = static_cast<int>(std::floor(ctx.drumPosition - 1.5));
    int endSlot   = static_cast<int>(std::ceil(ctx.drumPosition + 1.5));

    for (int i = startSlot; i <= endSlot; ++i) {
        if (i < 0 || (ctx.totalTracks > 0 && i >= static_cast<int>(ctx.totalTracks))) continue;
        double diffOffset = static_cast<double>(i) - ctx.drumPosition;
        drawDrumItem(static_cast<size_t>(i), diffOffset);
    }

    context->PopAxisAlignedClip();
  }
}
