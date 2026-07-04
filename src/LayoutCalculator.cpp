#include "LayoutCalculator.h"
#include "ConfigManager.h"
#include <algorithm>

BackgroundLayout LayoutCalculator::CalculateBackgroundLayout(float logicalWidth, float logicalHeight, D2D1_SIZE_F bitmapSize) {
    BackgroundLayout layout;
    layout.destRect = D2D1::RectF(0.0f, 0.0f, logicalWidth, logicalHeight);
    layout.overlayRect = D2D1::RectF(0.0f, 0.0f, logicalWidth, logicalHeight);

    if (bitmapSize.width > 0 && bitmapSize.height > 0) {
        float scaleX = logicalWidth / bitmapSize.width;
        float scaleY = logicalHeight / bitmapSize.height;
        float scale = (std::max)(scaleX, scaleY);
        
        float newWidth = logicalWidth / scale;
        float newHeight = logicalHeight / scale;
        float srcX = (bitmapSize.width - newWidth) / 2.0f;
        float srcY = (bitmapSize.height - newHeight) / 2.0f;
        
        layout.srcRect = D2D1::RectF(srcX, srcY, srcX + newWidth, srcY + newHeight);
    } else {
        layout.srcRect = D2D1::RectF(0.0f, 0.0f, 0.0f, 0.0f);
    }

    return layout;
}

VisualizerLayout LayoutCalculator::CalculateVisualizerLayout(float logicalWidth, float logicalHeight) {
    VisualizerLayout layout;
    layout.drawRect = D2D1::RectF(0.0f, 0.0f, logicalWidth, logicalHeight);
    return layout;
}



AppLogoLayout LayoutCalculator::CalculateAppLogoLayout(const ConfigManager* config) {
    AppLogoLayout layout = {};
    if (!config) return layout;

    float x = static_cast<float>(config->GetLogoX());
    float y = static_cast<float>(config->GetLogoY());
    float w = static_cast<float>(config->GetLogoWidth());
    float h = static_cast<float>(config->GetLogoHeight());
    
    layout.destRect = D2D1::RectF(x, y, x + w, y + h);
    layout.shadowOffset = D2D1::Point2F(x + config->GetShadowOffsetX(), y + config->GetShadowOffsetY());
    
    return layout;
}

TrackInfoLayout LayoutCalculator::CalculateTrackInfoLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, D2D1_SIZE_F bitmapSize) {
    TrackInfoLayout layout = {};
    if (!config) return layout;

    float size = static_cast<float>(config->GetArtSize());
    float x = static_cast<float>(config->GetBaseX() + config->GetArtOffsetX());
    float y = logicalHeight - static_cast<float>(config->GetBaseBottomOffset()) + static_cast<float>(config->GetArtOffsetY());

    // Album Art
    layout.fallbackArtRect = D2D1::RectF(x, y, x + size, y + size);

    if (bitmapSize.width > 0 && bitmapSize.height > 0) {
        float scaleX = size / bitmapSize.width;
        float scaleY = size / bitmapSize.height;
        float scale = (std::min)(scaleX, scaleY);
        
        float drawWidth = bitmapSize.width * scale;
        float drawHeight = bitmapSize.height * scale;
        
        float drawX = x + (size - drawWidth) / 2.0f;
        float drawY = y + (size - drawHeight) / 2.0f;
        
        layout.artDestRect = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);
        layout.artShadowRect = D2D1::RectF(
            drawX + config->GetShadowOffsetX(),
            drawY + config->GetShadowOffsetY(),
            drawX + drawWidth + config->GetShadowOffsetX(),
            drawY + drawHeight + config->GetShadowOffsetY()
        );
    }

    // Texts
    float baseX = static_cast<float>(config->GetBaseX());
    float baseY = logicalHeight - static_cast<float>(config->GetBaseBottomOffset());
    float rightMargin = 30.0f;

    // Title
    float titleX = baseX + static_cast<float>(config->GetTitleOffsetX());
    float titleY = baseY + static_cast<float>(config->GetTitleOffsetY());
    float titleRight = logicalWidth - rightMargin;
    if (titleRight < titleX) titleRight = titleX + 1.0f;

    layout.titleRect = D2D1::RectF(titleX, titleY, titleRight, titleY + 100.0f);
    layout.titleShadowRect = D2D1::RectF(
        titleX + config->GetShadowOffsetX(),
        titleY + config->GetShadowOffsetY(),
        titleRight + config->GetShadowOffsetX(),
        titleY + config->GetShadowOffsetY() + 100.0f
    );

    // Artist
    float artistX = baseX + static_cast<float>(config->GetArtistOffsetX());
    float artistY = baseY + static_cast<float>(config->GetArtistOffsetY());
    float artistRight = logicalWidth - rightMargin;
    if (artistRight < artistX) artistRight = artistX + 1.0f;

    layout.artistRect = D2D1::RectF(artistX, artistY, artistRight, artistY + 50.0f);
    layout.artistShadowRect = D2D1::RectF(
        artistX + config->GetShadowOffsetX(),
        artistY + config->GetShadowOffsetY(),
        artistRight + config->GetShadowOffsetX(),
        artistY + config->GetShadowOffsetY() + 50.0f
    );

    // TrackCount
    float trackCountX = baseX + static_cast<float>(config->GetTrackCountOffsetX());
    float trackCountY = baseY + static_cast<float>(config->GetTrackCountOffsetY());
    layout.trackCountOrigin = D2D1::Point2F(trackCountX, trackCountY);
    layout.trackCountShadowOrigin = D2D1::Point2F(trackCountX + config->GetTrackCountShadowOffsetX(), trackCountY + config->GetTrackCountShadowOffsetY());
    layout.trackCountMaxWidth = 200.0f;
    layout.trackCountMaxHeight = 50.0f;

    return layout;
}

NextTrackLayout LayoutCalculator::CalculateNextTrackLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, D2D1_SIZE_F bitmapSize) {
    NextTrackLayout layout = {};
    if (!config) return layout;

    float baseX = logicalWidth - static_cast<float>(config->GetNextBaseRightOffset());
    float baseY = logicalHeight - static_cast<float>(config->GetNextBaseBottomOffset());

    float artSize = static_cast<float>(config->GetNextArtSize());
    float artX = baseX + static_cast<float>(config->GetNextArtOffsetX());
    float artY = baseY + static_cast<float>(config->GetNextArtOffsetY());

    // Label
    float labelX = baseX + static_cast<float>(config->GetNextLabelOffsetX());
    float labelY = baseY + static_cast<float>(config->GetNextLabelOffsetY());
    
    layout.labelRect = D2D1::RectF(labelX, labelY, labelX + 200.0f, labelY + 30.0f);
    layout.labelShadowRect = D2D1::RectF(
        labelX + config->GetShadowOffsetX(),
        labelY + config->GetShadowOffsetY(),
        labelX + config->GetShadowOffsetX() + 200.0f,
        labelY + config->GetShadowOffsetY() + 30.0f
    );

    // Album Art
    layout.fallbackArtRect = D2D1::RectF(artX, artY, artX + artSize, artY + artSize);

    if (bitmapSize.width > 0 && bitmapSize.height > 0) {
        float scaleX = artSize / bitmapSize.width;
        float scaleY = artSize / bitmapSize.height;
        float scale = (std::min)(scaleX, scaleY);
        
        float drawWidth = bitmapSize.width * scale;
        float drawHeight = bitmapSize.height * scale;
        
        float drawX = artX + (artSize - drawWidth) / 2.0f;
        float drawY = artY + (artSize - drawHeight) / 2.0f;
        
        layout.artDestRect = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);
        layout.artShadowRect = D2D1::RectF(
            drawX + config->GetShadowOffsetX(),
            drawY + config->GetShadowOffsetY(),
            drawX + drawWidth + config->GetShadowOffsetX(),
            drawY + drawHeight + config->GetShadowOffsetY()
        );
    }

    // Title
    float titleX = baseX + static_cast<float>(config->GetNextTitleOffsetX());
    float titleY = baseY + static_cast<float>(config->GetNextTitleOffsetY());
    
    layout.titleRect = D2D1::RectF(titleX, titleY, titleX + 400.0f, titleY + 50.0f);
    layout.titleShadowRect = D2D1::RectF(
        titleX + config->GetShadowOffsetX(),
        titleY + config->GetShadowOffsetY(),
        titleX + config->GetShadowOffsetX() + 400.0f,
        titleY + config->GetShadowOffsetY() + 50.0f
    );

    // Artist
    float artistX = baseX + static_cast<float>(config->GetNextArtistOffsetX());
    float artistY = baseY + static_cast<float>(config->GetNextArtistOffsetY());
    
    layout.artistRect = D2D1::RectF(artistX, artistY, artistX + 400.0f, artistY + 50.0f);
    layout.artistShadowRect = D2D1::RectF(
        artistX + config->GetShadowOffsetX(),
        artistY + config->GetShadowOffsetY(),
        artistX + config->GetShadowOffsetX() + 400.0f,
        artistY + config->GetShadowOffsetY() + 50.0f
    );

    return layout;
}

SeekBarLayout LayoutCalculator::CalculateSeekBarLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, float progress) {
    SeekBarLayout layout = {};
    if (!config) return layout;

    float margin = config->GetSeekBarMargin();
    float totalWidth = logicalWidth - (margin * 2.0f);
    float startX = margin;
    float barAreaWidth = totalWidth - static_cast<float>(config->GetSeekBarTimeAreaWidth());
    float y = logicalHeight - static_cast<float>(config->GetSeekBarBottomOffset());
    float h = static_cast<float>(config->GetSeekBarHeight());

    layout.bgRect = D2D1::RectF(startX, y, startX + barAreaWidth, y + h);
    layout.fgRect = D2D1::RectF(startX, y, startX + barAreaWidth * progress, y + h);
    
    layout.textMaxWidth = totalWidth - barAreaWidth;
    layout.textMaxHeight = h;
    layout.textOrigin = D2D1::Point2F(startX + barAreaWidth, y);

    return layout;
}

PlaybackControlsLayout LayoutCalculator::CalculatePlaybackControlsLayout(float logicalWidth, float logicalHeight, const ConfigManager* config) {
    PlaybackControlsLayout layout = {};
    if (!config) return layout;

    layout.centerX = (logicalWidth / 2.0f) + config->GetPlaybackCenterOffsetX();
    layout.centerY = logicalHeight - config->GetPlaybackBaseBottomOffset();
    layout.size = static_cast<float>(config->GetPlaybackButtonSize());
    layout.spacing = static_cast<float>(config->GetPlaybackButtonSpacing());
    layout.half = layout.size / 2.0f;

    return layout;
}

VolumeControlLayout LayoutCalculator::CalculateVolumeControlLayout(float logicalWidth, float logicalHeight, const ConfigManager* config) {
    VolumeControlLayout layout = {};
    if (!config) return layout;

    layout.volX = static_cast<float>(config->GetVolumeBaseLeftOffset());
    layout.volY = logicalHeight - static_cast<float>(config->GetVolumeBaseBottomOffset());
    layout.volSize = static_cast<float>(config->GetVolumeIconSize());

    layout.spkW = layout.volSize * 0.35f;
    layout.spkH = layout.volSize * 0.35f;
    layout.spkConeW = layout.volSize * 0.45f;
    layout.spkConeH = layout.volSize * 0.8f;

    layout.shadowX = config->GetVolumeShadowOffsetX();
    layout.shadowY = config->GetVolumeShadowOffsetY();

    layout.textX = layout.volX + static_cast<float>(config->GetVolumeTextOffsetX());
    layout.textY = layout.volY + static_cast<float>(config->GetVolumeTextOffsetY());
    layout.textMaxWidth = 100.0f;
    layout.textMaxHeight = layout.volSize * 2.0f;

    return layout;
}

PlaylistLayout LayoutCalculator::CalculatePlaylistLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, float slideX, float manualScrollY, size_t currentTrackIndex, size_t totalTracks) {
    PlaylistLayout layout = {};
    if (!config) return layout;

    // Playlist Area
    layout.playlistWidth = static_cast<float>(config->GetPlaylistWidth());
    layout.playlistHeight = logicalHeight;
    layout.playlistY = 0.0f;

    float gripOffset = config->GetPlaylistGripOffset();
    layout.gripLineWidth = config->GetPlaylistGripLineWidth();

    if (config->GetPlaylistPosition() == 0) {
        layout.playlistX = -slideX;
        layout.clipRect = D2D1::RectF(0.0f, layout.playlistY, layout.playlistX + layout.playlistWidth, logicalHeight);
        layout.gripX = layout.playlistX + layout.playlistWidth + gripOffset;
    } else {
        layout.playlistX = logicalWidth - layout.playlistWidth + slideX;
        layout.clipRect = D2D1::RectF(layout.playlistX, layout.playlistY, logicalWidth, logicalHeight);
        layout.gripX = layout.playlistX - gripOffset;
    }

    layout.bgRect = D2D1::RectF(layout.playlistX, layout.playlistY, layout.playlistX + layout.playlistWidth, layout.playlistY + layout.playlistHeight);

    layout.gripShadowX = layout.gripX + config->GetPlaylistGripShadowOffsetX();
    layout.gripShadowY = layout.playlistY + config->GetPlaylistGripShadowOffsetY();

    // Scroll
    layout.itemHeight = static_cast<float>(config->GetPlaylistItemOffsetY());
    float baseScrollY = (layout.playlistHeight / 2.0f) - (currentTrackIndex * layout.itemHeight);
    float scrollY = baseScrollY + manualScrollY;
    float maxScroll = 0.0f;
    float minScroll = layout.playlistHeight - (totalTracks * layout.itemHeight);
    if (minScroll > 0) minScroll = 0;
    scrollY = std::clamp(scrollY, minScroll, maxScroll);
    
    layout.newManualScrollY = scrollY - baseScrollY;
    layout.startY = scrollY;

    return layout;
}

PlaylistItemLayout LayoutCalculator::CalculatePlaylistItemLayout(const PlaylistLayout& baseLayout, const ConfigManager* config, float currentY) {
    PlaylistItemLayout layout = {};
    if (!config) return layout;

    layout.hlRect = D2D1::RectF(baseLayout.playlistX, currentY, baseLayout.playlistX + baseLayout.playlistWidth, currentY + baseLayout.itemHeight);

    float textX = baseLayout.playlistX + static_cast<float>(config->GetPlaylistTitleOffsetX());
    float textY = currentY + static_cast<float>(config->GetPlaylistTitleOffsetY());
    layout.titleRect = D2D1::RectF(textX, textY, baseLayout.playlistX + baseLayout.playlistWidth - 10.0f, textY + 30.0f);

    float artistX = baseLayout.playlistX + static_cast<float>(config->GetPlaylistArtistOffsetX());
    float artistY = currentY + static_cast<float>(config->GetPlaylistArtistOffsetY());
    layout.artistRect = D2D1::RectF(artistX, artistY, baseLayout.playlistX + baseLayout.playlistWidth - 100.0f, artistY + 20.0f);

    float itemRightX = baseLayout.playlistX + baseLayout.playlistWidth;
    float timeX = itemRightX - 100.0f - static_cast<float>(config->GetPlaylistTimeOffsetX());
    float timeY = currentY + static_cast<float>(config->GetPlaylistTimeOffsetY());
    
    layout.timeOrigin = D2D1::Point2F(timeX, timeY);
    layout.timeMaxWidth = 100.0f;
    layout.timeMaxHeight = 20.0f;

    return layout;
}

ResizeGripLayout LayoutCalculator::CalculateResizeGripLayout(float logicalWidth, float logicalHeight) {
    ResizeGripLayout layout = {};
    float size = 15.0f;
    layout.pt1 = D2D1::Point2F(logicalWidth - size, logicalHeight);
    layout.pt2 = D2D1::Point2F(logicalWidth, logicalHeight);
    layout.pt3 = D2D1::Point2F(logicalWidth, logicalHeight - size);
    return layout;
}
