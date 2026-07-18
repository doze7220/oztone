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
    if (!m_config) return;

    int bgMode = m_config->GetBackgroundArtMode();
    ID2D1Bitmap* nowBitmap = nullptr;
    
    if (bgMode == 0) {
        nowBitmap = m_drumSlots[m_currentDrumSlotIndex].artBitmap.Get();
    } else if (bgMode == 2) {
        nowBitmap = m_placeholderArtBitmap.Get();
    }

    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;
    float bgOpacity = m_config->GetBgOpacity();

    ID2D1Bitmap* bmpNow = nowBitmap;
    if (!bmpNow && bgMode == 0) {
        bmpNow = m_placeholderArtBitmap.Get();
    }
    
    if (bmpNow) {
        BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, bmpNow->GetSize(), m_bgOffsetX, m_bgOffsetY, m_bgScale);
        m_d2dContext->DrawBitmap(
            bmpNow,
            &layout.destRect,
            bgOpacity,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &layout.srcRect
        );
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
