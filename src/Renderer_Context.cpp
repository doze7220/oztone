#include "Renderer.h"
#include "ConfigManager.h"
#include "WidgetContext.h"
#include "LayoutCalculator.h"
#include "ThumbnailManager.h"

WidgetContext Renderer::BuildAnimationContext(float deltaTime, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isTrackInfoHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode, int playbackHoveredIndex, int playlistHoveredItemIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isLogoClicked, int clickedLogoMenuIndex, int clickedPlaybackIndex, bool* outIsPlaylistExpanded, bool* outIsLogoMenuExpanded, bool isJogDialing) const {
    WidgetContext ctx = {};
    ctx.deltaTime = deltaTime;
    ctx.isControlHovered = isControlHovered;
    ctx.isVolumeHovered = isVolumeHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
    ctx.isTrackInfoHovered = isTrackInfoHovered;
    ctx.isLogoMenuHovered = isLogoMenuHovered;
    ctx.logoMenuHoveredIndex = logoMenuHoveredIndex;
    ctx.isLogoClicked = isLogoClicked;
    ctx.clickedLogoMenuIndex = clickedLogoMenuIndex;
    ctx.clickedPlaybackIndex = clickedPlaybackIndex;
    ctx.isPlaylistListViewMode = isPlaylistListViewMode;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.config = m_config;
    ctx.dpiScale = m_dpiScale;
    if (m_d2dContext) {
        D2D1_SIZE_F sz = m_d2dContext->GetSize();
        ctx.logicalWidth = sz.width / m_dpiScale;
        ctx.logicalHeight = sz.height / m_dpiScale;
    }
    ctx.playbackHoveredIndex = playbackHoveredIndex;
    ctx.playlistHoveredItemIndex = playlistHoveredItemIndex;
    ctx.logoMenuItems = logoMenuItems;
    ctx.outIsPlaylistExpanded = outIsPlaylistExpanded;
    ctx.outIsLogoMenuExpanded = outIsLogoMenuExpanded;
    ctx.shuffleIndices = m_shuffleIndices;
    
    ctx.drumRelativePosition = m_trackDrum.GetDrumRelativePosition();
    ctx.drumSlots = m_trackDrum.GetDrumSlots();
    ctx.currentDrumSlotIndex = m_trackDrum.GetCurrentDrumSlotIndex();
    ctx.animatingOldIndexOffset = m_trackDrum.GetAnimatingOldIndexOffset();
    ctx.isJogDialing = isJogDialing;

    return ctx;
}

WidgetContext Renderer::BuildLayoutContext(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>* shuffleMetadataList) const {
    WidgetContext ctx = {};
    ctx.timeString = timeString;
    ctx.volume = volume;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.dpiScale = m_dpiScale;
    if (m_d2dContext) {
        D2D1_SIZE_F sz = m_d2dContext->GetSize();
        ctx.logicalWidth = sz.width / m_dpiScale;
        ctx.logicalHeight = sz.height / m_dpiScale;
    }
    ctx.shuffleIndices = m_shuffleIndices;
    ctx.shuffleMetadataList = shuffleMetadataList;

    ctx.drumRelativePosition = m_trackDrum.GetDrumRelativePosition();
    ctx.drumSlots = m_trackDrum.GetDrumSlots();
    ctx.currentDrumSlotIndex = m_trackDrum.GetCurrentDrumSlotIndex();
    ctx.animatingOldIndexOffset = m_trackDrum.GetAnimatingOldIndexOffset();

    return ctx;
}

WidgetContext Renderer::BuildRenderContext(bool isHovered, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isTrackInfoHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isPlaylistListViewMode, bool isPlaying, float progress, const std::vector<float>* spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>* shuffleMetadataList, int playlistToolbarHoveredIndex, const std::vector<PlaylistSummary>* availablePlaylistsCache, bool isJogDialing) const {
    WidgetContext ctx = {};
    ctx.logoMenuItems = logoMenuItems;
    ctx.isControlHovered = isControlHovered;
    ctx.isVolumeHovered = isVolumeHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
    ctx.isTrackInfoHovered = isTrackInfoHovered;
    ctx.isLogoMenuHovered = isLogoMenuHovered;
    ctx.logoMenuHoveredIndex = logoMenuHoveredIndex;
    ctx.playlistToolbarHoveredIndex = playlistToolbarHoveredIndex;
    ctx.isHovered = isHovered;
    ctx.isPlaylistListViewMode = isPlaylistListViewMode;
    ctx.isPlaying = isPlaying;
    ctx.progress = progress;
    ctx.spectrum = spectrum;
    ctx.volume = volume;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.shuffleMetadataList = shuffleMetadataList;
    ctx.shuffleIndices = m_shuffleIndices;
    ctx.dpiScale = m_dpiScale;
    if (m_d2dContext) {
        D2D1_SIZE_F sz = m_d2dContext->GetSize();
        ctx.logicalWidth = sz.width / m_dpiScale;
        ctx.logicalHeight = sz.height / m_dpiScale;
    }

    ctx.controlAlpha = m_controlAlpha;
    ctx.timeString = m_lastTimeString;

    ctx.drumRelativePosition = m_trackDrum.GetDrumRelativePosition();
    ctx.drumSlots = m_trackDrum.GetDrumSlots();
    ctx.currentDrumSlotIndex = m_trackDrum.GetCurrentDrumSlotIndex();
    ctx.animatingTargetIndex = m_trackDrum.GetAnimatingTargetIndex();
    ctx.animatingOldIndexOffset = m_trackDrum.GetAnimatingOldIndexOffset();

    ctx.config = m_config;
    ctx.focusedPlaylistIndex = m_focusedPlaylistIndex;
    ctx.availablePlaylistsCache = availablePlaylistsCache;
    ctx.osdVolumeAlpha = m_osdVolumeAlpha;
    ctx.flyTextAlpha = m_flyTextAlpha;
    ctx.flyTextString = m_flyTextString;

    if (m_thumbnailManager && m_config && !isPlaylistListViewMode && shuffleMetadataList) {
        float logicalWidth = ctx.logicalWidth;
        float logicalHeight = ctx.logicalHeight;
        float manualScrollY = GetPlaylistManualScrollY();
        PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(logicalWidth, logicalHeight, m_config, 0.0f, manualScrollY, currentTrackIndex, totalTracks);
        
        float currentY = layout.startY;
        for (size_t i = 0; i < totalTracks && i < shuffleMetadataList->size(); ++i) {
            if (currentY + layout.itemHeight > 0 && currentY < layout.playlistHeight) {
                const std::wstring& path = (*shuffleMetadataList)[i].filepath;
                bool isNew = false;
                uint32_t thumbId = m_thumbnailManager->GetOrGenerateThumbId(path, isNew);
                if (thumbId > 0) {
                    ID2D1Bitmap* bmp = m_thumbnailManager->GetCachedThumbnailBitmap(thumbId);
                    if (bmp) {
                        ctx.playlistThumbnails[i] = bmp;
                    } else if (m_d2dContext && m_wicFactory) {
                        m_thumbnailManager->RequestThumbnailLoad(thumbId, m_d2dContext.Get(), m_wicFactory.Get());
                    }
                }
            }
            currentY += layout.itemHeight;
            if (currentY >= layout.playlistHeight) {
                break; // Optimization: no need to check further if beyond visible area
            }
        }
    } else if (m_thumbnailManager && m_config && isPlaylistListViewMode && availablePlaylistsCache) {
        float logicalWidth = ctx.logicalWidth;
        float logicalHeight = ctx.logicalHeight;
        float manualScrollY = GetPlaylistManualScrollY();
        size_t totalPlaylists = availablePlaylistsCache->size();
        
        std::wstring currentPlaylist = m_config->GetDefaultPlaylistPath();
        int currentIndex = 0;
        for (size_t i = 0; i < totalPlaylists; ++i) {
            if ((*availablePlaylistsCache)[i].filepath == currentPlaylist) {
                currentIndex = static_cast<int>(i);
                break;
            }
        }

        PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(logicalWidth, logicalHeight, m_config, 0.0f, manualScrollY, currentIndex, static_cast<int>(totalPlaylists));
        
        float currentY = layout.startY;
        for (size_t i = 0; i < totalPlaylists; ++i) {
            if (currentY + layout.itemHeight > 0 && currentY < layout.playlistHeight) {
                uint32_t thumbId = (*availablePlaylistsCache)[i].firstTrackThumbId;
                if (thumbId > 0) {
                    ID2D1Bitmap* bmp = m_thumbnailManager->GetCachedThumbnailBitmap(thumbId);
                    if (bmp) {
                        ctx.playlistThumbnails[i] = bmp;
                    } else if (m_d2dContext && m_wicFactory) {
                        m_thumbnailManager->RequestThumbnailLoad(thumbId, m_d2dContext.Get(), m_wicFactory.Get());
                    }
                }
            }
            currentY += layout.itemHeight;
            if (currentY >= layout.playlistHeight) {
                break;
            }
        }
    }

    ctx.isJogDialing = isJogDialing;

    return ctx;
}
