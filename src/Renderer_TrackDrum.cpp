#include "Renderer_TrackDrum.h"
#include "ConfigManager.h"
#include <cmath>
#include <algorithm>

TrackDrum::TrackDrum() {
}

TrackDrum::~TrackDrum() {
}

void TrackDrum::StartDrumAnimation(int relativeDistance, 
                                  std::function<TrackMetadata(int relativeIndex)> dataProvider,
                                  std::function<void()> onComplete) {
    if (relativeDistance == 0) {
        if (dataProvider) {
            TrackMetadata meta = dataProvider(0);
            m_drumSlots[m_currentDrumSlotIndex].artBitmap = nullptr;
            m_drumSlots[m_currentDrumSlotIndex].trackTitle = meta.title;
            m_drumSlots[m_currentDrumSlotIndex].trackArtist = meta.artist;
            m_drumSlots[m_currentDrumSlotIndex].trackNumber = meta.timeString;
        }
        if (onComplete) {
            onComplete();
        }
        return;
    }

    m_drumDataProvider = dataProvider;
    m_drumOnComplete = onComplete;
    
    m_drumRelativePosition += static_cast<float>(relativeDistance);
    
    m_animatingTargetIndex += relativeDistance;
    m_animatingOldIndexOffset += relativeDistance;
}

void TrackDrum::Update(float deltaTime, const ConfigManager* config) {
    if (!config) return;
    
    if (!config->GetEnableTrackDrum()) {
        m_drumRelativePosition = 0.0f;
        m_animatingTargetIndex = 0;
    } else {
        if (m_drumRelativePosition != 0.0f) {
            float dampingFactor = static_cast<float>(config->GetTrackDrumMaxSpeed()) * deltaTime;
            if (dampingFactor > 1.0f) dampingFactor = 1.0f;
            
            m_drumRelativePosition += (0.0f - m_drumRelativePosition) * dampingFactor;
            
            int currentPosRound = static_cast<int>(std::round(m_drumRelativePosition));
            if (currentPosRound != m_animatingTargetIndex) {
                m_animatingOldIndexOffset = m_animatingTargetIndex;
                m_animatingTargetIndex = currentPosRound;
                OnSlotAnimationCompleted();
            }

            if (std::abs(m_drumRelativePosition) < 0.001f) {
                m_drumRelativePosition = 0.0f;
                m_animatingTargetIndex = 0;
                OnSlotAnimationCompleted();
            }
        }
    }
}

void TrackDrum::SetAlbumArt(ID2D1Bitmap* bitmap) {
    m_drumSlots[m_currentDrumSlotIndex].artBitmap = bitmap;
}

void TrackDrum::UpdateCurrentDrumSlot(const TrackMetadata& meta) {
    if (!IsDrumAnimating()) {
        m_drumSlots[m_currentDrumSlotIndex].trackTitle = meta.title;
        m_drumSlots[m_currentDrumSlotIndex].trackArtist = meta.artist;
    }
}

void TrackDrum::OnSlotAnimationCompleted() {
    if (m_drumRelativePosition == 0.0f) {
        if (m_drumOnComplete) {
            auto cb = m_drumOnComplete;
            m_drumOnComplete = nullptr;
            m_drumDataProvider = nullptr;
            cb();
        }
    } else {
        if (m_drumDataProvider) {
            TrackMetadata meta = m_drumDataProvider(m_animatingTargetIndex);
            m_currentDrumSlotIndex = 1 - m_currentDrumSlotIndex;
            m_drumSlots[m_currentDrumSlotIndex].artBitmap = nullptr;
            m_drumSlots[m_currentDrumSlotIndex].trackTitle = meta.title;
            m_drumSlots[m_currentDrumSlotIndex].trackArtist = meta.artist;
            m_drumSlots[m_currentDrumSlotIndex].trackNumber = meta.timeString;
        }
    }
}
