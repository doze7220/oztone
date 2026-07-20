#include "Renderer.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "WidgetContext.h"
#include "BackgroundManager.h"

void Renderer::Render(bool isHovered, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isPlaylistListViewMode, bool isPlaying, float progress, const std::vector<float>& spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>& shuffleMetadataList, int playlistToolbarHoveredIndex, const std::vector<PlaylistSummary>* availablePlaylistsCache) {
    if (!m_d2dContext) return;

    m_d2dContext->BeginDraw();
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
    
    if (m_config && m_config->GetEnableResize()) {
        m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));
    } else {
        m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
    }
    
    DrawBackground();

    const TrackMetadata* currentMeta = nullptr;
    if (currentTrackIndex < shuffleMetadataList.size()) {
        currentMeta = &shuffleMetadataList[currentTrackIndex];
    }
    DrawVisualizer(spectrum, currentMeta);

    WidgetContext ctx = BuildRenderContext(isHovered, isControlHovered, isVolumeHovered, isPlaylistHovered, isLogoMenuHovered, logoMenuHoveredIndex, logoMenuItems, isPlaylistListViewMode, isPlaying, progress, &spectrum, volume, currentTrackIndex, totalTracks, &shuffleMetadataList, playlistToolbarHoveredIndex, availablePlaylistsCache);

    for (auto& widget : m_widgets) {
        widget->Draw(m_d2dContext.Get(), ctx, m_config);
    }

    HRESULT hr = m_d2dContext->EndDraw();
    if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) {
        m_swapChain->Present(1, 0);
    }
}

void Renderer::DrawBackground() {
    if (!m_config) return;

    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    if (m_backgroundManager) {
        auto currentWic = m_backgroundManager->GetCurrentWicImage();
        if (currentWic != m_lastCurrentWicImage || !m_currentBgBitmap) {
            m_lastCurrentWicImage = currentWic;
            m_currentBgBitmap.Reset();
            if (currentWic) {
                m_d2dContext->CreateBitmapFromWicBitmap(currentWic.Get(), &m_currentBgBitmap);
            }
            if (!m_currentBgBitmap && m_placeholderArtBitmap) {
                m_currentBgBitmap = m_placeholderArtBitmap;
            }
        }

        auto oldWic = m_backgroundManager->GetOldWicImage();
        if (oldWic != m_lastOldWicImage || !m_oldBgBitmap) {
            m_lastOldWicImage = oldWic;
            m_oldBgBitmap.Reset();
            if (oldWic) {
                m_d2dContext->CreateBitmapFromWicBitmap(oldWic.Get(), &m_oldBgBitmap);
            }
            if (!m_oldBgBitmap && m_placeholderArtBitmap) {
                m_oldBgBitmap = m_placeholderArtBitmap;
            }
        }
    }

    int bgMode = m_config->GetBackgroundArtMode();

    if (bgMode == 0) {
        if (m_oldBgBitmap) {
            D2D1_SIZE_F size = m_oldBgBitmap->GetSize();
            BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, size, m_bgOffsetX, m_bgOffsetY, m_bgScale);
            m_d2dContext->DrawBitmap(m_oldBgBitmap.Get(), &layout.destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &layout.srcRect);
        }

        if (m_currentBgBitmap) {
            D2D1_SIZE_F size = m_currentBgBitmap->GetSize();
            BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, size, m_bgOffsetX, m_bgOffsetY, m_bgScale);
            float opacity = m_backgroundManager ? m_backgroundManager->GetFadeProgress() : 1.0f;
            m_d2dContext->DrawBitmap(m_currentBgBitmap.Get(), &layout.destRect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &layout.srcRect);
        }
    } else if (bgMode == 2) {
        if (m_placeholderArtBitmap) {
            D2D1_SIZE_F size = m_placeholderArtBitmap->GetSize();
            BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, size, m_bgOffsetX, m_bgOffsetY, m_bgScale);
            m_d2dContext->DrawBitmap(m_placeholderArtBitmap.Get(), &layout.destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &layout.srcRect);
        }
    }

    if (m_config->GetBgDarkenOpacity() > 0.0f && m_bgDarkenBrush) {
        m_bgDarkenBrush->SetOpacity(m_config->GetBgDarkenOpacity());
        BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, D2D1::SizeF(0.0f, 0.0f));
        m_d2dContext->FillRectangle(&layout.overlayRect, m_bgDarkenBrush.Get());
    }
}

void Renderer::DrawVisualizer(const std::vector<float>& spectrum, const TrackMetadata* currentMeta) {
    if (m_config && m_config->GetVisualizerMode() != 0 && !spectrum.empty()) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        float logicWidth = renderTargetSize.width / m_dpiScale;
        float logicHeight = renderTargetSize.height / m_dpiScale;
        
        float peakAmplitude = currentMeta ? currentMeta->peakAmplitude : 0.0f;
        float maxFrequency = currentMeta ? currentMeta->maxFrequency : 0.0f;

        VisualizerLayout layout = LayoutCalculator::CalculateVisualizerLayout(logicWidth, logicHeight);
        std::wstring title = currentMeta ? currentMeta->title : L"Unknown";
        std::wstring artist = currentMeta ? currentMeta->artist : L"Unknown";
        m_visualizer.Draw(m_d2dContext.Get(), spectrum, layout.drawRect, title, artist, peakAmplitude, maxFrequency);
    }
}
