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
                            float maxDuration, float maxSpeed,
                            std::function<TrackMetadata(int relativeIndex, DrumSlot* slot)> dataProvider,
                            std::function<void()> onComplete);
                            
    void Update(float deltaTime, const ConfigManager* config);

    void SetAlbumArt(ID2D1Bitmap* bitmap);
    void UpdateCurrentDrumSlot(const TrackMetadata& meta);
    
    bool IsDrumAnimating() const { return m_drumAbsolutePosition != static_cast<float>(m_drumTargetPosition); }

    const std::array<DrumSlot, 3>& GetDrumSlots() const { return m_drumSlots; }
    std::array<DrumSlot, 3>& GetDrumSlotsWritable() { return m_drumSlots; }
    int GetCurrentDrumSlotIndex() const { return m_currentDrumSlotIndex; }
    float GetDrumRelativePosition() const { return m_drumAbsolutePosition; }
    int GetAnimatingTargetIndex() const { return m_animatingTargetIndex; }
    int GetAnimatingOldIndexOffset() const { return m_animatingOldIndexOffset; }

private:
    void OnSlotAnimationCompleted();
    void PrefillAbsolute(int absolutePos);

    float m_indexStep = 1.0f;
    float m_drumAbsolutePosition = 0.0f;
    std::array<DrumSlot, 3> m_drumSlots;
    int m_currentDrumSlotIndex = 0;
    int m_animatingTargetIndex = 0;
    int m_animatingOldIndexOffset = 0;
    
    int m_drumTargetPosition = 0;
    int m_lastIntegerPosition = 0;
    
    std::function<TrackMetadata(int, DrumSlot*)> m_drumDataProvider;
    std::function<void()> m_drumOnComplete;
};
