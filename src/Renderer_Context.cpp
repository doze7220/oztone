#include "Renderer.h"
#include "ConfigManager.h"
#include "WidgetContext.h"

WidgetContext Renderer::BuildAnimationContext(float deltaTime, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode, int playbackHoveredIndex, int playlistHoveredItemIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isLogoClicked, int clickedLogoMenuIndex, int clickedPlaybackIndex, bool* outIsPlaylistExpanded, bool* outIsLogoMenuExpanded) const {
    WidgetContext ctx = {};
    ctx.deltaTime = deltaTime;
    ctx.isControlHovered = isControlHovered;
    ctx.isVolumeHovered = isVolumeHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
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
    
    ctx.oldTrackTitle = m_oldTrackTitle;
    ctx.oldTrackArtist = m_oldTrackArtist;
    ctx.oldTrackIndex = m_oldTrackIndex;
    ctx.oldTrackNoString = m_oldTrackNoString;
    ctx.oldArtBitmap = m_oldArtBitmap.Get();
    ctx.isDrumAnimating = m_isDrumAnimating;
    ctx.drumPosition = m_drumPosition;
    ctx.drumVelocity = m_drumVelocity;
    ctx.drumTargetIndex = m_drumTargetIndex;
    ctx.drumStartIndex = m_drumStartIndex;
    ctx.drumVirtualOldIndex = m_drumVirtualOldIndex;
    ctx.oldIsCrossPlaylist = m_oldIsCrossPlaylist;
    ctx.streamBreakDirection = m_streamBreakDirection;

    return ctx;
}

WidgetContext Renderer::BuildLayoutContext(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks) const {
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
    ctx.trackTitle = m_trackTitle;
    ctx.trackArtist = m_trackArtist;
    ctx.shuffleIndices = m_shuffleIndices;

    ctx.oldTrackTitle = m_oldTrackTitle;
    ctx.oldTrackArtist = m_oldTrackArtist;
    ctx.oldTrackIndex = m_oldTrackIndex;
    ctx.oldTrackNoString = m_oldTrackNoString;
    ctx.oldArtBitmap = m_oldArtBitmap.Get();
    ctx.isDrumAnimating = m_isDrumAnimating;
    ctx.drumPosition = m_drumPosition;
    ctx.drumVelocity = m_drumVelocity;
    ctx.drumTargetIndex = m_drumTargetIndex;
    ctx.drumStartIndex = m_drumStartIndex;
    ctx.drumVirtualOldIndex = m_drumVirtualOldIndex;
    ctx.oldIsCrossPlaylist = m_oldIsCrossPlaylist;
    ctx.streamBreakDirection = m_streamBreakDirection;

    return ctx;
}

WidgetContext Renderer::BuildRenderContext(bool isHovered, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isPlaylistListViewMode, bool isPlaying, float progress, const std::vector<float>* spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>* shuffleMetadataList, int playlistToolbarHoveredIndex, const std::vector<PlaylistSummary>* availablePlaylistsCache) const {
    WidgetContext ctx = {};
    ctx.logoMenuItems = logoMenuItems;
    ctx.isControlHovered = isControlHovered;
    ctx.isVolumeHovered = isVolumeHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
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
    ctx.trackTitle = m_trackTitle;
    ctx.trackArtist = m_trackArtist;
    ctx.currentArtBitmap = m_currentArtBitmap.Get();

    ctx.oldTrackTitle = m_oldTrackTitle;
    ctx.oldTrackArtist = m_oldTrackArtist;
    ctx.oldTrackIndex = m_oldTrackIndex;
    ctx.oldTrackNoString = m_oldTrackNoString;
    ctx.oldArtBitmap = m_oldArtBitmap.Get();
    ctx.isDrumAnimating = m_isDrumAnimating;
    ctx.drumPosition = m_drumPosition;
    ctx.drumVelocity = m_drumVelocity;
    ctx.drumTargetIndex = m_drumTargetIndex;
    ctx.drumVirtualOldIndex = m_drumVirtualOldIndex;
    ctx.oldIsCrossPlaylist = m_oldIsCrossPlaylist;
    ctx.streamBreakDirection = m_streamBreakDirection;

    ctx.config = m_config;
    ctx.focusedPlaylistIndex = m_focusedPlaylistIndex;
    ctx.availablePlaylistsCache = availablePlaylistsCache;
    ctx.osdVolumeAlpha = m_osdVolumeAlpha;
    ctx.flyTextAlpha = m_flyTextAlpha;
    ctx.flyTextString = m_flyTextString;
    return ctx;
}
