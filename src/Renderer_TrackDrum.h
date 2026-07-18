#pragma once

#include "WidgetContext.h"
#include <array>
#include <functional>

class TrackDrum {
public:
    TrackDrum();
    ~TrackDrum();

private:
    float m_drumRelativePosition = 0.0f;
    std::array<DrumSlot, 2> m_drumSlots;
    int m_currentDrumSlotIndex = 0;
    int m_animatingTargetIndex = 0;
    int m_animatingOldIndexOffset = 0;
    
    std::function<TrackMetadata(int)> m_drumDataProvider;
    std::function<void()> m_drumOnComplete;
};
