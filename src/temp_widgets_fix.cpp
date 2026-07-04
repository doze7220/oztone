
// ================= PlaylistWidget =================

PlaylistWidget::PlaylistWidget() : m_playlistSlideX(9999.0f), m_playlistManualScrollY(0.0f), m_lastTotalTracks(0) {}

void PlaylistWidget::CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) {
    if (!config) return;
    m_dwriteFactory = dwriteFactory;

    // テキストフォーマットの作成
    dwriteFactory->CreateTextFormat(
        config->GetTrackCountFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        config->GetTrackCountFontSize(),
        L"en-us",
        &m_trackCountTextFormat
    );

    dwriteFactory->CreateTextFormat(
        config->GetPlaylistTitleFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        config->GetPlaylistTitleFontSize(),
        L"ja-jp",
        &m_playlistTitleTextFormat
    );

    dwriteFactory->CreateTextFormat(
        config->GetPlaylistArtistFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        config->GetPlaylistArtistFontSize(),
        L"ja-jp",
        &m_playlistArtistTextFormat
    );

    dwriteFactory->CreateTextFormat(
        config->GetPlaylistTimeFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        config->GetPlaylistTimeFontSize(),
        L"en-us",
        &m_playlistTimeTextFormat
    );

    auto ApplyTrimming = [&](Microsoft::WRL::ComPtr<IDWriteTextFormat>& format) {
        if (!format) return;
        DWRITE_TRIMMING trimmingOptions = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
        Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
        if (SUCCEEDED(dwriteFactory->CreateEllipsisTrimmingSign(format.Get(), &ellipsis))) {
            format->SetTrimming(&trimmingOptions, ellipsis.Get());
            format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        }
    };
    ApplyTrimming(m_trackCountTextFormat);
    ApplyTrimming(m_playlistTitleTextFormat);
    ApplyTrimming(m_playlistArtistTextFormat);
    ApplyTrimming(m_playlistTimeTextFormat);

    if (m_trackCountTextFormat) m_trackCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (m_playlistTimeTextFormat) m_playlistTimeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

    auto ParseHexColor = [](const std::wstring& hexColor) -> D2D1_COLOR_F {
        if (hexColor.empty() || hexColor[0] != L'#') return D2D1::ColorF(D2D1::ColorF::White);
        try {
            unsigned int hexValue = std::stoul(hexColor.substr(1), nullptr, 16);
            if (hexColor.length() == 7) {
                return D2D1::ColorF(
                    ((hexValue >> 16) & 0xFF) / 255.0f,
                    ((hexValue >> 8) & 0xFF) / 255.0f,
                    (hexValue & 0xFF) / 255.0f
                );
            }
        } catch (...) {}
        return D2D1::ColorF(D2D1::ColorF::White);
    };

    context->CreateSolidColorBrush(ParseHexColor(config->GetPlaylistArtistColor()), &m_playlistArtistBrush);
    context->CreateSolidColorBrush(ParseHexColor(config->GetPlaylistTimeColor()), &m_playlistTimeBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_textBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_shadowBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_playlistBgBrush);
    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_playlistHighlightBrush);
    context->CreateSolidColorBrush(ParseHexColor(config->GetPlaylistGripLineColor()), &m_playlistGripLineBrush);
    context->CreateSolidColorBrush(ParseHexColor(config->GetPlaylistGripArrowColor()), &m_playlistGripArrowBrush);

    Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
    context->GetFactory(&d2dFactory);

    if (d2dFactory) {
        d2dFactory->CreatePathGeometry(&m_playlistGripArrowGeometry);
        if (m_playlistGripArrowGeometry) {
            Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
            if (SUCCEEDED(m_playlistGripArrowGeometry->Open(&sink))) {
                float width = config->GetPlaylistGripArrowWidth();
                float height = config->GetPlaylistGripArrowHeight();
                sink->BeginFigure(D2D1::Point2F(0.0f, -height / 2.0f), D2D1_FIGURE_BEGIN_FILLED);
                sink->AddLine(D2D1::Point2F(-width, 0.0f));
                sink->AddLine(D2D1::Point2F(0.0f, height / 2.0f));
                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                sink->Close();
            }
        }
    }
}

void PlaylistWidget::ReleaseResources() {
    m_playlistTitleTextFormat.Reset();
    m_playlistArtistTextFormat.Reset();
    m_playlistTimeTextFormat.Reset();
    m_trackCountTextFormat.Reset();
    m_trackCountTextLayout.Reset();
    m_playlistArtistBrush.Reset();
    m_playlistTimeBrush.Reset();
    m_playlistGripLineBrush.Reset();
    m_playlistGripArrowBrush.Reset();
    m_playlistGripArrowGeometry.Reset();
    m_playlistBgBrush.Reset();
    m_playlistHighlightBrush.Reset();
    m_textBrush.Reset();
    m_shadowBrush.Reset();
    m_dwriteFactory.Reset();
}

void PlaylistWidget::UpdateAnimation(const WidgetContext& ctx) {}

void PlaylistWidget::UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) {
    if (!config) return;

    float configPlaylistWidth = static_cast<float>(config->GetPlaylistWidth());
    if (m_playlistSlideX > configPlaylistWidth * 2.0f) m_playlistSlideX = configPlaylistWidth;

    float targetSlideX = ctx.isPlaylistHovered ? 0.0f : configPlaylistWidth;
    m_playlistSlideX += (targetSlideX - m_playlistSlideX) * 0.2f;

    if (!ctx.isPlaylistHovered) {
        m_playlistManualScrollY = 0.0f;
    } else {
        D2D1_SIZE_F renderTargetSize = D2D1::SizeF(
            config->GetWindowWidth() * ctx.dpiScale, 
            config->GetWindowHeight() * ctx.dpiScale
        );
        float logicWidth = renderTargetSize.width / ctx.dpiScale;
        float logicHeight = renderTargetSize.height / ctx.dpiScale;
        
        PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
            logicWidth, logicHeight, config, m_playlistSlideX, m_playlistManualScrollY, ctx.currentTrackIndex, ctx.totalTracks);
        m_playlistManualScrollY = layout.newManualScrollY;
    }

    if (m_dwriteFactory && m_trackCountTextFormat && m_lastTotalTracks != ctx.totalTracks) {
        m_lastTotalTracks = ctx.totalTracks;
        m_trackCountTextLayout.Reset();
        
        wchar_t trackCountBuf[64];
        if (ctx.totalTracks == 0) {
            swprintf_s(trackCountBuf, L"TRACK ---/---");
        } else {
            swprintf_s(trackCountBuf, L"TRACK %zu/%zu", ctx.currentTrackIndex + 1, ctx.totalTracks);
        }
        std::wstring trackCountStr(trackCountBuf);
        
        m_dwriteFactory->CreateTextLayout(
            trackCountStr.c_str(),
            static_cast<UINT32>(trackCountStr.length()),
            m_trackCountTextFormat.Get(),
            1000.0f,
            100.0f,
            &m_trackCountTextLayout
        );
    }
}

void PlaylistWidget::AddScroll(float delta) {
    m_playlistManualScrollY += delta;
}

float PlaylistWidget::GetScrollY() const {
    return m_playlistManualScrollY;
}

void PlaylistWidget::Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) {
    if (!config || !m_trackCountTextFormat || !m_textBrush) return;

    D2D1_SIZE_F renderTargetSize = context->GetSize();
    float logicWidth = renderTargetSize.width / ctx.dpiScale;
    float logicHeight = renderTargetSize.height / ctx.dpiScale;

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicWidth, logicHeight, config, m_playlistSlideX, m_playlistManualScrollY, ctx.currentTrackIndex, ctx.totalTracks);

    if (m_trackCountTextLayout) {
        if (m_shadowBrush && config->GetTrackCountShadowOpacity() > 0.0f) {
            m_shadowBrush->SetOpacity(config->GetTrackCountShadowOpacity());
            context->DrawTextLayout(layout.trackCountShadowOrigin, m_trackCountTextLayout.Get(), m_shadowBrush.Get());
        }
        context->DrawTextLayout(layout.trackCountOrigin, m_trackCountTextLayout.Get(), m_textBrush.Get());
    }

    if (m_playlistGripLineBrush && m_playlistGripArrowBrush && m_playlistGripArrowGeometry) {
        if (m_shadowBrush && config->GetPlaylistGripShadowOpacity() > 0.0f) {
            m_shadowBrush->SetOpacity(config->GetPlaylistGripShadowOpacity());
            
            context->DrawLine(D2D1::Point2F(layout.gripShadowX, layout.playlistY), D2D1::Point2F(layout.gripShadowX, layout.playlistY + layout.playlistHeight), m_shadowBrush.Get(), layout.gripLineWidth);
            
            D2D1_MATRIX_3X2_F shadowTransform = D2D1::Matrix3x2F::Translation(layout.gripShadowX, layout.gripShadowY + layout.playlistHeight / 2.0f);
            context->SetTransform(shadowTransform * D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
            context->FillGeometry(m_playlistGripArrowGeometry.Get(), m_shadowBrush.Get());
            context->SetTransform(D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
        }

        context->DrawLine(D2D1::Point2F(layout.gripX, layout.playlistY), D2D1::Point2F(layout.gripX, layout.playlistY + layout.playlistHeight), m_playlistGripLineBrush.Get(), layout.gripLineWidth);

        D2D1_MATRIX_3X2_F arrowTransform = D2D1::Matrix3x2F::Translation(layout.gripX, layout.playlistY + layout.playlistHeight / 2.0f);
        context->SetTransform(arrowTransform * D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
        context->FillGeometry(m_playlistGripArrowGeometry.Get(), m_playlistGripArrowBrush.Get());
        context->SetTransform(D2D1::Matrix3x2F::Scale(ctx.dpiScale, ctx.dpiScale));
    }

    if (m_playlistSlideX < layout.playlistWidth - 0.5f) {
        if (m_playlistBgBrush) {
            m_playlistBgBrush->SetOpacity(config->GetPlaylistBgOpacity());
            context->FillRectangle(&layout.bgRect, m_playlistBgBrush.Get());
        }

        context->PushAxisAlignedClip(&layout.clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        float currentY = layout.startY;
        
        if (m_playlistHighlightBrush) {
            m_playlistHighlightBrush->SetOpacity(0.2f);
        }

        for (size_t i = 0; i < ctx.totalTracks && ctx.shuffleList && i < ctx.shuffleList->size(); ++i) {
            if (currentY + layout.itemHeight > 0 && currentY < layout.playlistHeight) {
                PlaylistItemLayout itemLayout = LayoutCalculator::CalculatePlaylistItemLayout(layout, config, currentY);

                if (i == ctx.currentTrackIndex && m_playlistHighlightBrush) {
                    context->FillRectangle(&itemLayout.hlRect, m_playlistHighlightBrush.Get());
                }

                std::wstring path = (*ctx.shuffleList)[i];
                std::wstring title;
                try { title = std::filesystem::path(path).filename().wstring(); } catch(...) { title = L"Unknown"; }
                std::wstring artist = L"Unknown Artist";

                context->DrawText(title.c_str(), static_cast<UINT32>(title.length()), m_playlistTitleTextFormat.Get(), &itemLayout.titleRect, m_textBrush.Get());

                context->DrawText(artist.c_str(), static_cast<UINT32>(artist.length()), m_playlistArtistTextFormat.Get(), &itemLayout.artistRect, m_playlistArtistBrush ? m_playlistArtistBrush.Get() : m_textBrush.Get());
            }
            currentY += layout.itemHeight;
        }

        context->PopAxisAlignedClip();
    }
}

// ================= ResizeGripWidget =================

void ResizeGripWidget::CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) {
    if (!config) return;

    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_resizeGripBrush);

    Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
    context->GetFactory(&d2dFactory);

    if (d2dFactory) {
        d2dFactory->CreatePathGeometry(&m_resizeGripGeometry);
        if (m_resizeGripGeometry) {
            Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
            if (SUCCEEDED(m_resizeGripGeometry->Open(&sink))) {
                sink->SetFillMode(D2D1_FILL_MODE_WINDING);
                sink->BeginFigure(D2D1::Point2F(0.0f, -15.0f), D2D1_FIGURE_BEGIN_FILLED);
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

void ResizeGripWidget::Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) {
    if (config && config->GetEnableResize() && m_resizeGripBrush && m_resizeGripGeometry) {
        D2D1_SIZE_F renderTargetSize = context->GetSize();
        float logicWidth = renderTargetSize.width / ctx.dpiScale;
        float logicHeight = renderTargetSize.height / ctx.dpiScale;

        m_resizeGripBrush->SetOpacity(0.5f);
        D2D1_MATRIX_3X2_F oldTransform;
        context->GetTransform(&oldTransform);
        context->SetTransform(D2D1::Matrix3x2F::Translation(logicWidth, logicHeight) * oldTransform);
        context->FillGeometry(m_resizeGripGeometry.Get(), m_resizeGripBrush.Get());
        context->SetTransform(oldTransform);
    }
}
