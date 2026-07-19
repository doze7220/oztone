#pragma once

#include "WidgetContext.h"
#include <array>
#include <functional>
#include <d2d1.h>

class ConfigManager;

class TrackDrum {
public:
    TrackDrum();
    ~TrackDrum();

    void StartDrumAnimation(int relativeDistance, 
                            std::function<TrackMetadata(int relativeIndex, DrumSlot* slot)> dataProvider,
                            std::function<void()> onComplete);
                            
    void Update(float deltaTime, const ConfigManager* config);

    void SetAlbumArt(ID2D1Bitmap* bitmap);
    void UpdateCurrentDrumSlot(const TrackMetadata& meta);
    
    bool IsDrumAnimating() const { return m_drumRelativePosition != 0.0f; }

    const std::array<DrumSlot, 2>& GetDrumSlots() const { return m_drumSlots; }
    std::array<DrumSlot, 2>& GetDrumSlotsWritable() { return m_drumSlots; }
    int GetCurrentDrumSlotIndex() const { return m_currentDrumSlotIndex; }
    float GetDrumRelativePosition() const { return m_drumRelativePosition; }
    int GetAnimatingTargetIndex() const { return m_animatingTargetIndex; }
    int GetAnimatingOldIndexOffset() const { return m_animatingOldIndexOffset; }

private:
    void OnSlotAnimationCompleted();

    float m_drumRelativePosition = 0.0f;
    std::array<DrumSlot, 2> m_drumSlots;
    int m_currentDrumSlotIndex = 0;
    int m_animatingTargetIndex = 0;
    int m_animatingOldIndexOffset = 0;
    
    std::function<TrackMetadata(int, DrumSlot*)> m_drumDataProvider;
    std::function<void()> m_drumOnComplete;
};
