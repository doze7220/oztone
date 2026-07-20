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
    if (!m_backgroundManager || !m_d2dContext) return;

    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    auto layers = m_backgroundManager->GetLayers();
    std::map<IWICFormatConverter*, Microsoft::WRL::ComPtr<ID2D1Bitmap>> nextCache;

    for (const auto& layer : layers) {
        if (layer.type == BackgroundLayerType::LayerGroupBegin) {
            D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters(
                D2D1::InfiniteRect(), nullptr,
                D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                D2D1::IdentityMatrix(),
                layer.opacity,
                nullptr,
                D2D1_LAYER_OPTIONS_NONE
            );
            m_d2dContext->PushLayer(layerParams, m_backgroundLayer.Get());
        } else if (layer.type == BackgroundLayerType::LayerGroupEnd) {
            m_d2dContext->PopLayer();
        } else if (layer.type == BackgroundLayerType::Image) {
            ID2D1Bitmap* bmp = nullptr;
            if (layer.image) {
                IWICFormatConverter* wic = layer.image.Get();
                if (m_bgBitmapCache.find(wic) != m_bgBitmapCache.end()) {
                    bmp = m_bgBitmapCache[wic].Get();
                    nextCache[wic] = m_bgBitmapCache[wic];
                } else {
                    Microsoft::WRL::ComPtr<ID2D1Bitmap> newBmp;
                    if (SUCCEEDED(m_d2dContext->CreateBitmapFromWicBitmap(wic, &newBmp))) {
                        bmp = newBmp.Get();
                        nextCache[wic] = newBmp;
                    }
                }
            } else if (m_placeholderArtBitmap) {
                bmp = m_placeholderArtBitmap.Get();
            }

            if (bmp) {
                D2D1_SIZE_F size = bmp->GetSize();
                BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, size, layer.x, layer.y, layer.scale);
                m_d2dContext->DrawBitmap(bmp, &layout.destRect, layer.opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &layout.srcRect);
            }
        } else if (layer.type == BackgroundLayerType::SolidColor) {
            if (m_fallbackBlackBrush) {
                m_fallbackBlackBrush->SetColor(layer.color);
                m_fallbackBlackBrush->SetOpacity(layer.opacity);
                BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, D2D1::SizeF(0.0f, 0.0f));
                m_d2dContext->FillRectangle(&layout.overlayRect, m_fallbackBlackBrush.Get());
            }
        }
    }
    
    m_bgBitmapCache = std::move(nextCache);
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
