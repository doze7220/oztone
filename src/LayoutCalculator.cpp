#include "LayoutCalculator.h"
#include "Config/ConfigManager.h"
#include <algorithm>

static void ApplyPinningOffset(float& logicalWidth, float& offsetX, const ConfigManager* config) {
    if (!config || !config->GetLayoutPlaylist().IsPlaylistPinned) return;
    float playlistWidth = static_cast<float>(config->GetLayoutPlaylist().PlaylistWidth);
    float minRequiredWidth = 495.0f + playlistWidth;
    if (logicalWidth >= minRequiredWidth) {
        logicalWidth -= playlistWidth;
        if (config->GetLayoutPlaylist().PlaylistPosition == 0) { // Left
            offsetX = playlistWidth;
        }
    }
}

BackgroundLayout LayoutCalculator::CalculateBackgroundLayout(float logicalWidth, float logicalHeight, D2D1_SIZE_F bitmapSize, float offsetX, float offsetY, float artScale) {
    BackgroundLayout layout;
    layout.overlayRect = D2D1::RectF(0.0f, 0.0f, logicalWidth, logicalHeight);

    if (bitmapSize.width > 0 && bitmapSize.height > 0) {
        float scaleX = logicalWidth / bitmapSize.width;
        float scaleY = logicalHeight / bitmapSize.height;
        float baseScale = (std::max)(scaleX, scaleY);
        float finalScale = baseScale * artScale;
        
        float drawWidth = bitmapSize.width * finalScale;
        float drawHeight = bitmapSize.height * finalScale;
        
        float limitX = (std::max)(0.0f, (drawWidth - logicalWidth) / 2.0f);
        float limitY = (std::max)(0.0f, (drawHeight - logicalHeight) / 2.0f);
        
        float clampedOffsetX = std::clamp(offsetX, -limitX, limitX);
        float clampedOffsetY = std::clamp(offsetY, -limitY, limitY);
        
        float x = (logicalWidth - drawWidth) / 2.0f + clampedOffsetX;
        float y = (logicalHeight - drawHeight) / 2.0f + clampedOffsetY;
        
        layout.srcRect = D2D1::RectF(0.0f, 0.0f, bitmapSize.width, bitmapSize.height);
        layout.destRect = D2D1::RectF(x, y, x + drawWidth, y + drawHeight);
    } else {
        layout.srcRect = D2D1::RectF(0.0f, 0.0f, 0.0f, 0.0f);
        layout.destRect = D2D1::RectF(0.0f, 0.0f, logicalWidth, logicalHeight);
    }

    return layout;
}

void LayoutCalculator::CalculateArtFramingBounds(float logicalWidth, float logicalHeight, D2D1_SIZE_F bitmapSize, float scale, float& outMaxOffsetX, float& outMaxOffsetY) {
    outMaxOffsetX = 0.0f;
    outMaxOffsetY = 0.0f;
    if (bitmapSize.width <= 0 || bitmapSize.height <= 0) return;

    float scaleX = logicalWidth / bitmapSize.width;
    float scaleY = logicalHeight / bitmapSize.height;
    float baseScale = (std::max)(scaleX, scaleY);
    float finalScale = baseScale * scale;

    float scaledImageWidth = bitmapSize.width * finalScale;
    float scaledImageHeight = bitmapSize.height * finalScale;

    outMaxOffsetX = (std::max)(0.0f, (scaledImageWidth - logicalWidth) / 2.0f);
    outMaxOffsetY = (std::max)(0.0f, (scaledImageHeight - logicalHeight) / 2.0f);
}

VisualizerLayout LayoutCalculator::CalculateVisualizerLayout(float logicalWidth, float logicalHeight) {
    VisualizerLayout layout;
    layout.drawRect = D2D1::RectF(0.0f, 0.0f, logicalWidth, logicalHeight);
    return layout;
}



AppLogoLayout LayoutCalculator::CalculateAppLogoLayout(float logicalWidth, const ConfigManager* config) {
    AppLogoLayout layout = {};
    if (!config) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);

    float x = offsetX + static_cast<float>(config->GetLayoutAppLogo().X);
    float y = static_cast<float>(config->GetLayoutAppLogo().Y);
    float w = static_cast<float>(config->GetLayoutAppLogo().Width);
    float h = static_cast<float>(config->GetLayoutAppLogo().Height);
    
    layout.destRect = D2D1::RectF(x, y, x + w, y + h);
    layout.shadowOffset = D2D1::Point2F(x + config->GetUICommonParm().ShadowOffsetX, y + config->GetUICommonParm().ShadowOffsetY);
    
    return layout;
}

LogoMenuLayout LayoutCalculator::CalculateLogoMenuLayout(float logicalWidth, const ConfigManager* config, float progress, size_t itemCount) {
    LogoMenuLayout layout;
    if (!config || itemCount == 0) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);

    float logoX = offsetX + static_cast<float>(config->GetLayoutAppLogo().X);
    float logoY = static_cast<float>(config->GetLayoutAppLogo().Y);
    float logoW = static_cast<float>(config->GetLayoutAppLogo().Width);
    float logoH = static_cast<float>(config->GetLayoutAppLogo().Height);
    
    // ロゴアイコン右上 (Top-Right)
    float topRightX = logoX + logoW;
    float topRightY = logoY;

    float iconOffsetY = static_cast<float>(config->GetLayoutLogoMenu().MenuIconOffsetY);
    float centerY = topRightY + iconOffsetY;
    
    float iconOffsetX = static_cast<float>(config->GetLayoutLogoMenu().MenuIconOffsetX);
    float startX = topRightX + iconOffsetX;
    float iconSpacing = static_cast<float>(config->GetLayoutLogoMenu().MenuIconSpacing);
    float iconSize = config->GetLayoutLogoMenu().MenuIconSize;

    layout.fullRegionRect = D2D1::RectF(logoX, logoY, logoX + logoW, logoY + logoH);
    
    for (size_t i = 0; i < itemCount; ++i) {
        float targetX = startX + iconSpacing * (i + 1);
        float currentX = startX + (targetX - startX) * progress;
        
        LogoMenuItemLayout itemLayout;
        itemLayout.position = D2D1::Point2F(currentX, centerY);
        float hitMargin = iconSpacing / 2.0f;
        itemLayout.hitRect = D2D1::RectF(currentX - hitMargin, centerY - logoH/2.0f, currentX + hitMargin, centerY + logoH/2.0f);
        
        layout.items.push_back(itemLayout);
        
        layout.fullRegionRect.right = (std::max)(layout.fullRegionRect.right, currentX + iconSpacing);
    }
    
    float textStartX = topRightX + static_cast<float>(config->GetLayoutLogoMenu().MenuTextOffsetX);
    float textStartY = topRightY + static_cast<float>(config->GetLayoutLogoMenu().MenuTextOffsetY);
    layout.typingTextRect = D2D1::RectF(textStartX, textStartY, textStartX + 800.0f, textStartY + 50.0f);
    layout.fullRegionRect.bottom = (std::max)(layout.fullRegionRect.bottom, textStartY + 50.0f);
    
    return layout;
}

TrackInfoLayout LayoutCalculator::CalculateTrackInfoLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, D2D1_SIZE_F bitmapSize) {
    TrackInfoLayout layout = {};
    if (!config) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);
    layout.clipRect = D2D1::RectF(offsetX, 0.0f, offsetX + logicalWidth, logicalHeight);

    float size = static_cast<float>(config->GetLayoutTrackInfo().ArtSize);
    float x = offsetX + static_cast<float>(config->GetLayoutTrackInfo().BaseX + config->GetLayoutTrackInfo().ArtOffsetX);
    float y = logicalHeight - static_cast<float>(config->GetLayoutTrackInfo().BaseBottomOffset) + static_cast<float>(config->GetLayoutTrackInfo().ArtOffsetY);

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
            drawX + config->GetUICommonParm().ShadowOffsetX,
            drawY + config->GetUICommonParm().ShadowOffsetY,
            drawX + drawWidth + config->GetUICommonParm().ShadowOffsetX,
            drawY + drawHeight + config->GetUICommonParm().ShadowOffsetY
        );
    }

    // Texts
    float baseX = offsetX + static_cast<float>(config->GetLayoutTrackInfo().BaseX);
    float baseY = logicalHeight - static_cast<float>(config->GetLayoutTrackInfo().BaseBottomOffset);
    float rightMargin = 30.0f;

    // Title
    float titleX = baseX + static_cast<float>(config->GetLayoutTrackInfo().TitleOffsetX);
    float titleY = baseY + static_cast<float>(config->GetLayoutTrackInfo().TitleOffsetY);
    float titleRight = offsetX + logicalWidth - rightMargin;
    if (titleRight < titleX) titleRight = titleX + 1.0f;

    layout.titleRect = D2D1::RectF(titleX, titleY, titleRight, titleY + 100.0f);
    layout.titleShadowRect = D2D1::RectF(
        titleX + config->GetUICommonParm().ShadowOffsetX,
        titleY + config->GetUICommonParm().ShadowOffsetY,
        titleRight + config->GetUICommonParm().ShadowOffsetX,
        titleY + config->GetUICommonParm().ShadowOffsetY + 100.0f
    );

    // Artist
    float artistX = baseX + static_cast<float>(config->GetLayoutTrackInfo().ArtistOffsetX);
    float artistY = baseY + static_cast<float>(config->GetLayoutTrackInfo().ArtistOffsetY);
    float artistRight = offsetX + logicalWidth - rightMargin;
    if (artistRight < artistX) artistRight = artistX + 1.0f;

    layout.artistRect = D2D1::RectF(artistX, artistY, artistRight, artistY + 50.0f);
    layout.artistShadowRect = D2D1::RectF(
        artistX + config->GetUICommonParm().ShadowOffsetX,
        artistY + config->GetUICommonParm().ShadowOffsetY,
        artistRight + config->GetUICommonParm().ShadowOffsetX,
        artistY + config->GetUICommonParm().ShadowOffsetY + 50.0f
    );



    return layout;
}


SeekBarLayout LayoutCalculator::CalculateSeekBarLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, float progress) {
    SeekBarLayout layout = {};
    if (!config) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);

    float margin = config->GetLayoutSeekBar().SeekBarMargin;
    float totalWidth = logicalWidth - (margin * 2.0f);
    float startX = offsetX + margin;
    float barAreaWidth = totalWidth;
    float y = logicalHeight - static_cast<float>(config->GetLayoutSeekBar().BottomOffset);
    float h = static_cast<float>(config->GetLayoutSeekBar().Height);

    layout.bgRect = D2D1::RectF(startX, y, startX + barAreaWidth, y + h);
    layout.fgRect = D2D1::RectF(startX, y, startX + barAreaWidth * progress, y + h);
    
    layout.textMaxWidth = totalWidth - config->GetLayoutSeekBar().TimeMarginRight;
    layout.textMaxHeight = h;
    layout.textOrigin = D2D1::Point2F(startX, y);

    return layout;
}

PlaybackControlsLayout LayoutCalculator::CalculatePlaybackControlsLayout(float logicalWidth, float logicalHeight, const ConfigManager* config) {
    PlaybackControlsLayout layout = {};
    if (!config) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);

    layout.centerX = offsetX + (logicalWidth / 2.0f) + config->GetLayoutPlaybackControls().CenterOffsetX;
    layout.centerY = logicalHeight - config->GetLayoutPlaybackControls().BaseBottomOffset;
    layout.size = static_cast<float>(config->GetLayoutPlaybackControls().ButtonSize);
    layout.spacing = static_cast<float>(config->GetLayoutPlaybackControls().ButtonSpacing);
    layout.half = layout.size / 2.0f;

    return layout;
}

VolumeControlLayout LayoutCalculator::CalculateVolumeControlLayout(float logicalWidth, float logicalHeight, const ConfigManager* config) {
    VolumeControlLayout layout = {};
    if (!config) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);

    layout.volX = offsetX + static_cast<float>(config->GetLayoutVolumeControl().BaseLeftOffset);
    layout.volY = logicalHeight - static_cast<float>(config->GetLayoutVolumeControl().BaseBottomOffset);
    layout.volSize = static_cast<float>(config->GetLayoutVolumeControl().IconSize);

    layout.spkW = layout.volSize * 0.35f;
    layout.spkH = layout.volSize * 0.35f;
    layout.spkConeW = layout.volSize * 0.45f;
    layout.spkConeH = layout.volSize * 0.8f;

    layout.shadowX = config->GetUICommonParm().ShadowOffsetX;
    layout.shadowY = config->GetUICommonParm().ShadowOffsetY;

    layout.textX = layout.volX + static_cast<float>(config->GetLayoutVolumeControl().TextOffsetX);
    layout.textY = layout.volY + static_cast<float>(config->GetLayoutVolumeControl().TextOffsetY);
    layout.textMaxWidth = 100.0f;
    layout.textMaxHeight = layout.volSize * 2.0f;

    // Tooltip Layout
    float tooltipW = config->GetLayoutTooltip().TooltipWidth;
    float tooltipH = config->GetLayoutTooltip().TooltipHeight;
    layout.tooltipBoxW = tooltipW;
    layout.tooltipBoxH = tooltipH;
    layout.tooltipBoxX = layout.volX + 16.0f - tooltipW / 2.0f; // center relative to speaker
    layout.tooltipBoxY = layout.volY - layout.volSize - tooltipH - config->GetLayoutVolumeControl().TooltipOffsetY;

    layout.tooltipRadius = 4.0f;

    // Triangle below box
    layout.tooltipTriangleX1 = layout.tooltipBoxX + tooltipW / 2.0f - 6.0f;
    layout.tooltipTriangleY1 = layout.tooltipBoxY + tooltipH;
    layout.tooltipTriangleX2 = layout.tooltipBoxX + tooltipW / 2.0f + 6.0f;
    layout.tooltipTriangleY2 = layout.tooltipBoxY + tooltipH;
    layout.tooltipTriangleX3 = layout.tooltipBoxX + tooltipW / 2.0f;
    layout.tooltipTriangleY3 = layout.tooltipBoxY + tooltipH + 6.0f;

    layout.tooltipTextX = layout.tooltipBoxX;
    layout.tooltipTextY = layout.tooltipBoxY;

    return layout;
}

PlaylistLayout LayoutCalculator::CalculatePlaylistLayout(float logicalWidth, float logicalHeight, const ConfigManager* config, float slideX, float manualScrollY, size_t currentTrackIndex, size_t totalTracks) {
    PlaylistLayout layout = {};
    if (!config) return layout;

    // Playlist Area
    layout.playlistWidth = static_cast<float>(config->GetLayoutPlaylist().PlaylistWidth);
    layout.playlistHeight = logicalHeight;
    layout.playlistY = 0.0f;

    float gripOffset = config->GetLayoutPlaylist().PlaylistGripOffset;
    layout.gripLineWidth = config->GetLayoutPlaylist().PlaylistGripLineWidth;

    if (config->GetLayoutPlaylist().PlaylistPosition == 0) {
        layout.playlistX = -slideX;
        layout.gripX = layout.playlistX + layout.playlistWidth + gripOffset;
    } else {
        layout.playlistX = logicalWidth - layout.playlistWidth + slideX;
        layout.gripX = layout.playlistX - gripOffset;
    }

    float toolbarHeight = config->GetLayoutPlaylist().ToolbarHeight;
    layout.toolbarLayout.fullRect = D2D1::RectF(layout.playlistX, layout.playlistY, layout.playlistX + layout.playlistWidth, layout.playlistY + toolbarHeight);
    
    float btnSize = 30.0f;
    float iconSpacing = config->GetLayoutPlaylist().ToolbarIconSpacing;
    float totalWidth = 3 * btnSize + 2 * iconSpacing;
    float startX = layout.playlistX + 2.0f; // 左寄せ + 2.0fマージン
    float startY = layout.playlistY + 5.0f; // 上段に配置

    // 左から 0, 1, 2 の順に配置 (0:左, 1:中, 2:右)
    for (int i = 0; i < 3; ++i) {
        layout.toolbarLayout.buttonHitRects[i] = D2D1::RectF(startX, startY, startX + btnSize, startY + btnSize);
        startX += (btnSize + iconSpacing);
    }

    // ピン留めボタンの配置 (ツールバーの右上)
    float pinBtnMarginRight = 10.0f;
    float pinBtnX = layout.playlistX + layout.playlistWidth - btnSize - pinBtnMarginRight;
    layout.toolbarLayout.pinButtonHitRect = D2D1::RectF(pinBtnX, startY, pinBtnX + btnSize, startY + btnSize);

    
    float textOffsetY = config->GetLayoutPlaylist().ToolbarTextOffsetY;
    layout.toolbarLayout.textRect = D2D1::RectF(layout.playlistX, layout.playlistY + textOffsetY, layout.playlistX + layout.playlistWidth, layout.playlistY + toolbarHeight);

    if (config->GetLayoutPlaylist().PlaylistPosition == 0) {
        layout.clipRect = D2D1::RectF(0.0f, layout.playlistY + toolbarHeight, layout.playlistX + layout.playlistWidth, logicalHeight);
    } else {
        layout.clipRect = D2D1::RectF(layout.playlistX, layout.playlistY + toolbarHeight, logicalWidth, logicalHeight);
    }

    layout.bgRect = D2D1::RectF(layout.playlistX, layout.playlistY, layout.playlistX + layout.playlistWidth, layout.playlistY + layout.playlistHeight);

    layout.gripShadowX = layout.gripX + config->GetUICommonParm().ShadowOffsetX;
    layout.gripShadowY = layout.playlistY + config->GetUICommonParm().ShadowOffsetY;

    // Scroll
    layout.itemHeight = static_cast<float>(config->GetLayoutPlaylist().PlaylistItemOffsetY);
    float viewHeight = layout.clipRect.bottom - layout.clipRect.top;
    float totalHeight = totalTracks * layout.itemHeight;
    
    float baseScrollY = (viewHeight / 2.0f) - (currentTrackIndex * layout.itemHeight);
    float scrollY = baseScrollY + manualScrollY;
    
    float maxScroll = 0.0f;
    float minScroll = viewHeight - totalHeight;
    
    if (minScroll > maxScroll) {
        minScroll = maxScroll;
    }
    
    scrollY = std::clamp(scrollY, minScroll, maxScroll);
    
    layout.newManualScrollY = scrollY - baseScrollY;
    layout.startY = layout.playlistY + toolbarHeight + scrollY;

    return layout;
}

int LayoutCalculator::GetPlaylistItemIndexAt(float logicalY, const PlaylistLayout& layout, size_t totalItems) {
    float toolbarHeight = layout.toolbarLayout.fullRect.bottom - layout.toolbarLayout.fullRect.top;
    if (logicalY < layout.playlistY + toolbarHeight) {
        return -1;
    }
    
    float clickedY = logicalY - layout.startY;
    if (clickedY < 0.0f) {
        return -1;
    }
    
    int index = static_cast<int>(clickedY / layout.itemHeight);
    if (index >= 0 && index < static_cast<int>(totalItems)) {
        return index;
    }
    
    return -1;
}

PlaylistItemLayout LayoutCalculator::CalculatePlaylistItemLayout(const PlaylistLayout& baseLayout, const ConfigManager* config, float currentY) {
    PlaylistItemLayout layout = {};
    if (!config) return layout;

    layout.hlRect = D2D1::RectF(baseLayout.playlistX, currentY, baseLayout.playlistX + baseLayout.playlistWidth, currentY + baseLayout.itemHeight);

    float thumbSize = config->GetLayoutPlaylist().PlaylistThumbSize;
    float thumbOffsetX = config->GetLayoutPlaylist().PlaylistThumbOffsetX;
    float thumbOffsetY = config->GetLayoutPlaylist().PlaylistThumbOffsetY;

    float thumbX = baseLayout.playlistX + thumbOffsetX;
    float thumbY = currentY + thumbOffsetY;
    layout.thumbRect = D2D1::RectF(thumbX, thumbY, thumbX + thumbSize, thumbY + thumbSize);

    float textShiftX = thumbSize + thumbOffsetX;

    float textX = baseLayout.playlistX + static_cast<float>(config->GetLayoutPlaylist().PlaylistTitleOffsetX) + textShiftX;
    float textY = currentY + static_cast<float>(config->GetLayoutPlaylist().PlaylistTitleOffsetY);
    layout.titleRect = D2D1::RectF(textX, textY, baseLayout.playlistX + baseLayout.playlistWidth - 10.0f, textY + 30.0f);

    float artistX = baseLayout.playlistX + static_cast<float>(config->GetLayoutPlaylist().PlaylistArtistOffsetX) + textShiftX;
    float artistY = currentY + static_cast<float>(config->GetLayoutPlaylist().PlaylistArtistOffsetY);
    layout.artistRect = D2D1::RectF(artistX, artistY, baseLayout.playlistX + baseLayout.playlistWidth - 100.0f, artistY + 20.0f);

    float itemRightX = baseLayout.playlistX + baseLayout.playlistWidth;
    float timeX = itemRightX - 100.0f - static_cast<float>(config->GetLayoutPlaylist().PlaylistTimeOffsetX);
    float timeY = currentY + static_cast<float>(config->GetLayoutPlaylist().PlaylistTimeOffsetY);
    
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

GlobalHotkeysLayout LayoutCalculator::CalculateGlobalHotkeysLayout(float logicalWidth, const ConfigManager* config) {
    GlobalHotkeysLayout layout = {};
    if (!config) return layout;

    float topMargin = 20.0f;
    float keyOffset = config->GetLayoutGlobalHotkeys().KeyColumnOffset;
    float actionOffset = config->GetLayoutGlobalHotkeys().ActionColumnOffset;

    float keyX = logicalWidth - keyOffset;
    float actionX = logicalWidth - actionOffset;
    if (keyX < 0) keyX = 0;
    if (actionX < 0) actionX = 0;

    layout.keyColumnRect = D2D1::RectF(keyX, topMargin, logicalWidth, topMargin + 800.0f);
    layout.actionColumnRect = D2D1::RectF(actionX, topMargin, logicalWidth, topMargin + 800.0f);
    
    return layout;
}

OsdLayout LayoutCalculator::CalculateOsdLayout(float logicalWidth, float logicalHeight, float textWidth, float textHeight, const ConfigManager* config) {
    OsdLayout layout = {};
    if (!config) return layout;

    float offsetX = 0.0f;
    ApplyPinningOffset(logicalWidth, offsetX, config);

    float centerX = offsetX + logicalWidth / 2.0f;
    float centerY = logicalHeight / 2.0f;

    float left = centerX - textWidth / 2.0f;
    float top = centerY - textHeight / 2.0f;

    layout.textRect = D2D1::RectF(left, top, left + textWidth, top + textHeight);
    layout.shadowRect = D2D1::RectF(
        left + config->GetUICommonParm().ShadowOffsetX,
        top + config->GetUICommonParm().ShadowOffsetY,
        left + textWidth + config->GetUICommonParm().ShadowOffsetX,
        top + textHeight + config->GetUICommonParm().ShadowOffsetY
    );

    return layout;
}
