#include "Renderer.h"
#include "ConfigManager.h"
#include "WidgetContext.h"
#include <cmath>

void Renderer::UpdateAnimation(float deltaTime, bool isControlHovered, bool isVolumeHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode, int playbackHoveredIndex, int playlistHoveredItemIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isLogoClicked, int clickedLogoMenuIndex, int clickedPlaybackIndex, bool* outIsPlaylistExpanded, bool* outIsLogoMenuExpanded) {
    if (isControlHovered) {
        m_controlLeaveTimer = m_config ? m_config->GetBaseLeaveDelay() : 3.0f;
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
        float fadeSpeed = m_config->GetOsdFadeSpeed();
        
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

        if (!m_config->GetEnableTrackDrum()) {
            m_isDrumAnimating = false;
            m_drumPosition = static_cast<double>(currentTrackIndex);
            m_drumVelocity = 0.0;
        } else {
            if (m_drumTargetIndex != currentTrackIndex) {
                m_drumStartIndex = m_drumTargetIndex;
                m_drumTargetIndex = currentTrackIndex;
                if (m_isDrumAnimating) {
                    double target = static_cast<double>(m_drumTargetIndex);
                    
                    if (m_drumStartIndex > 0 && m_drumTargetIndex == 0) {
                        m_drumPosition = target - 1.0;
                    } else if (m_drumStartIndex == 0 && m_drumTargetIndex == totalTracks - 1) {
                        m_drumPosition = target + 1.0;
                    }

                    double diff = target - m_drumPosition;
                    
                    double maxSpeed = m_config->GetTrackDrumMaxSpeed();
                    double maxDuration = m_config->GetTrackDrumMaxDuration();
                    double maxDist = maxSpeed * maxDuration;
                    
                    if (std::abs(diff) > maxDist) {
                        m_drumPosition = target - (diff > 0.0 ? maxDist : -maxDist);
                    }
                }
            } else {
                m_drumTargetIndex = currentTrackIndex;
            }

            if (m_isDrumAnimating) {
                double target = static_cast<double>(m_drumTargetIndex);
                double diff = target - m_drumPosition;
                
                if (std::abs(diff) < 0.001 && std::abs(m_drumVelocity) < 0.001) {
                    m_isDrumAnimating = false;
                    m_drumPosition = target;
                    m_drumVelocity = 0.0;
                } else {
                    double accel = m_config->GetTrackDrumAcceleration();
                    double decel = m_config->GetTrackDrumDeceleration();
                    double maxSpeed = m_config->GetTrackDrumMaxSpeed();
                    
                    double dir = (diff > 0.0) ? 1.0 : -1.0;
                    double currentSpeed = std::abs(m_drumVelocity);
                    double dist = std::abs(diff);
                    double stoppingDist = (currentSpeed * currentSpeed) / (2.0 * decel);
                    
                    if (dist <= stoppingDist) {
                        currentSpeed -= decel * deltaTime;
                        if (currentSpeed < 0.0) currentSpeed = 0.0;
                    } else {
                        currentSpeed += accel * deltaTime;
                        if (currentSpeed > maxSpeed) currentSpeed = maxSpeed;
                    }
                    
                    m_drumVelocity = currentSpeed * dir;
                    m_drumPosition += m_drumVelocity * deltaTime;
                    
                    if ((dir > 0.0 && m_drumPosition > target) || (dir < 0.0 && m_drumPosition < target)) {
                        m_drumPosition = target;
                        m_drumVelocity = 0.0;
                        m_isDrumAnimating = false;
                    }
                }
            } else {
                m_drumPosition = static_cast<double>(currentTrackIndex);
                m_drumVelocity = 0.0;
            }
        }
    }

    WidgetContext ctx = BuildAnimationContext(deltaTime, isControlHovered, isVolumeHovered, isPlaylistHovered, isLogoMenuHovered, logoMenuHoveredIndex, currentTrackIndex, totalTracks, isPlaylistListViewMode, playbackHoveredIndex, playlistHoveredItemIndex, logoMenuItems, isLogoClicked, clickedLogoMenuIndex, clickedPlaybackIndex, outIsPlaylistExpanded, outIsLogoMenuExpanded);
    
    for (auto& widget : m_widgets) {
        widget->UpdateAnimation(ctx);
    }
}

void Renderer::UpdateTextLayouts(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks) {
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
    
    WidgetContext ctx = BuildLayoutContext(timeString, volume, currentTrackIndex, totalTracks);
    
    for (auto& widget : m_widgets) {
        widget->UpdateLayout(ctx, m_config);
    }
}
