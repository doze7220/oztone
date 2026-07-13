#include "Renderer.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "WidgetContext.h"

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
    int bgMode = m_config ? m_config->GetBackgroundArtMode() : 0;
    ID2D1Bitmap* artBitmap = nullptr;
    if (bgMode == 0) {
        artBitmap = m_currentArtBitmap ? m_currentArtBitmap.Get() : m_placeholderArtBitmap.Get();
    } else if (bgMode == 2) {
        artBitmap = m_placeholderArtBitmap.Get();
    }
    
    if (artBitmap && m_config) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        float logicWidth = renderTargetSize.width / m_dpiScale;
        float logicHeight = renderTargetSize.height / m_dpiScale;
        
        BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, artBitmap->GetSize(), m_bgOffsetX, m_bgOffsetY, m_bgScale);
        
        m_d2dContext->DrawBitmap(
            artBitmap,
            &layout.destRect,
            m_config->GetBgOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &layout.srcRect
        );
    }

    if (m_config && m_config->GetBgDarkenOpacity() > 0.0f && m_bgDarkenBrush) {
        m_bgDarkenBrush->SetOpacity(m_config->GetBgDarkenOpacity());
        D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
        float logicWidth = rtSize.width / m_dpiScale;
        float logicHeight = rtSize.height / m_dpiScale;
        
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
        m_visualizer.Draw(m_d2dContext.Get(), spectrum, layout.drawRect, m_trackTitle, m_trackArtist, peakAmplitude, maxFrequency);
    }
}
