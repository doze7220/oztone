#include "Renderer.h"
#include "Config/ConfigManager.h"
#include "WidgetContext.h"
#include "ThumbnailManager.h"
#include <cmath>

void Renderer::UpdateAnimation(float deltaTime, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isTrackInfoHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode, int playbackHoveredIndex, int playlistHoveredItemIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isLogoClicked, int clickedLogoMenuIndex, int clickedPlaybackIndex, bool* outIsPlaylistExpanded, bool* outIsLogoMenuExpanded, bool isJogDialing) {
    if (isControlHovered) {
        m_controlLeaveTimer = m_config ? m_config->GetUICommonParm().BaseLeaveDelay : 3.0f;
        m_controlAlpha += 0.05f;
        if (m_controlAlpha > 1.0f) m_controlAlpha = 1.0f;
    } else {
        if (m_controlLeaveTimer > 0.0f) {
            m_controlLeaveTimer -= deltaTime;
        } else {
            m_controlAlpha -= 0.05f;
            if (m_controlAlpha < 0.0f) m_controlAlpha = 0.0f;
        }
    }

    if (m_config) {
        float fadeSpeed = m_config->GetLayoutOSD().OsdFadeSpeed;
        
        if (m_osdVolumeWaitTimer > 0.0f) {
            m_osdVolumeWaitTimer -= deltaTime;
        } else if (m_osdVolumeAlpha > 0.0f) {
            m_osdVolumeAlpha -= deltaTime * fadeSpeed;
            if (m_osdVolumeAlpha < 0.0f) m_osdVolumeAlpha = 0.0f;
        }

        if (m_flyTextWaitTimer > 0.0f) {
            m_flyTextWaitTimer -= deltaTime;
        } else if (m_flyTextAlpha > 0.0f) {
            m_flyTextAlpha -= deltaTime * fadeSpeed;
            if (m_flyTextAlpha < 0.0f) m_flyTextAlpha = 0.0f;
        }

    }

    m_trackDrum.Update(deltaTime, m_config);

    if (m_thumbnailManager) {
        auto& slots = m_trackDrum.GetDrumSlotsWritable();
        ULONGLONG now = GetTickCount64();
        bool canPoll = false;
        if (now - m_lastThumbPollTime >= 1000) {
            canPoll = true;
            m_lastThumbPollTime = now;
        }

        for (int i = 0; i < 3; ++i) {
            auto& slot = slots[i];
            if (slot.thumbId > 0 && !slot.artBitmap) {
                ID2D1Bitmap* bmp = m_thumbnailManager->GetCachedThumbnailBitmap(slot.thumbId);
                if (bmp) {
                    slot.artBitmap = bmp;
                } else if (canPoll) {
                    if (m_thumbnailManager->HasCookedData(slot.thumbId)) {
                        m_thumbnailManager->RequestThumbnailLoad(slot.thumbId, m_d2dContext.Get(), m_wicFactory.Get());
                    }
                }
            }
        }
    }

    WidgetContext ctx = BuildAnimationContext(deltaTime, isControlHovered, isVolumeHovered, isPlaylistHovered, isTrackInfoHovered, isLogoMenuHovered, logoMenuHoveredIndex, currentTrackIndex, totalTracks, isPlaylistListViewMode, playbackHoveredIndex, playlistHoveredItemIndex, logoMenuItems, isLogoClicked, clickedLogoMenuIndex, clickedPlaybackIndex, outIsPlaylistExpanded, outIsLogoMenuExpanded, isJogDialing);

    for (auto& widget : m_widgets) {
        widget->UpdateAnimation(ctx);
    }
}

void Renderer::UpdateTextLayouts(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>& shuffleMetadataList) {
    if (m_forceTextLayoutUpdate) {
        m_lastTimeString = L"";
        m_lastVolume = -2.0f;
        m_lastCurrentTrackIndex = static_cast<size_t>(-2);
        m_lastTotalTracks = static_cast<size_t>(-2);
        m_forceTextLayoutUpdate = false;
    }

    m_lastTimeString = timeString;
    m_lastVolume = volume;
    m_lastCurrentTrackIndex = currentTrackIndex;
    m_lastTotalTracks = totalTracks;
    
    WidgetContext ctx = BuildLayoutContext(timeString, volume, currentTrackIndex, totalTracks, &shuffleMetadataList);
    
    for (auto& widget : m_widgets) {
        widget->UpdateLayout(ctx, m_config);
    }
}
