#include "Renderer_TrackDrum.h"
#include "Config/ConfigManager.h"
#include <cmath>
#include <algorithm>

TrackDrum::TrackDrum() {
}

TrackDrum::~TrackDrum() {
}

void TrackDrum::StartDrumAnimation(int relativeDistance, 
                                  float maxDuration, float maxSpeed,
                                  std::function<TrackMetadata(int relativeIndex, DrumSlot* slot)> dataProvider,
                                  std::function<void()> onComplete) {
    if (!m_isDrumInitialized) {
        m_drumTargetPosition = 0;
        m_drumAbsolutePosition = 0.0f;
        if (dataProvider) {
            TrackMetadata meta0 = dataProvider(0, &m_drumSlots[0]);
            m_drumSlots[0].artBitmap = nullptr;
            m_drumSlots[0].trackTitle = meta0.title;
            m_drumSlots[0].trackArtist = meta0.artist;
            m_drumSlots[0].trackNumber = meta0.timeString;

            TrackMetadata meta1 = dataProvider(1, &m_drumSlots[1]);
            m_drumSlots[1].artBitmap = nullptr;
            m_drumSlots[1].trackTitle = meta1.title;
            m_drumSlots[1].trackArtist = meta1.artist;
            m_drumSlots[1].trackNumber = meta1.timeString;

            TrackMetadata meta2 = dataProvider(-1, &m_drumSlots[2]);
            m_drumSlots[2].artBitmap = nullptr;
            m_drumSlots[2].trackTitle = meta2.title;
            m_drumSlots[2].trackArtist = meta2.artist;
            m_drumSlots[2].trackNumber = meta2.timeString;
        }
        m_isDrumInitialized = true;
        if (onComplete) {
            onComplete();
        }
        return;
    }

    if (relativeDistance == 0) {
        if (dataProvider) {
            int slotIdx = (m_drumTargetPosition % 3 + 3) % 3;
            TrackMetadata meta = dataProvider(0, &m_drumSlots[slotIdx]);
            m_drumSlots[slotIdx].artBitmap = nullptr;
            m_drumSlots[slotIdx].trackTitle = meta.title;
            m_drumSlots[slotIdx].trackArtist = meta.artist;
            m_drumSlots[slotIdx].trackNumber = meta.timeString;
        } else {
            m_drumSlots[(m_drumTargetPosition % 3 + 3) % 3].thumbId = 0;
        }
        if (onComplete) {
            onComplete();
        }
        return;
    }

    m_drumDataProvider = dataProvider;
    m_drumOnComplete = onComplete;
    
    int startPos = static_cast<int>(std::round(m_drumAbsolutePosition));
    int direction = (relativeDistance > 0) ? 1 : -1;

    int maxFlipCount = (std::max)(1, static_cast<int>(maxDuration * maxSpeed * 60.0f));

    int physicalDistance = (std::min)(std::abs(relativeDistance), maxFlipCount) * direction;

    if (physicalDistance != 0) {
        m_indexStep = static_cast<float>(relativeDistance) / physicalDistance;
    } else {
        m_indexStep = 1.0f;
    }

    m_drumTargetPosition += physicalDistance;
    m_currentDrumSlotIndex = (m_drumTargetPosition % 3 + 3) % 3;
    
    // 開始位置(Start)を基準とし、進行方向に向かって3スロット分を正しく連番で事前注入(Pre-fill)
    PrefillAbsolute(startPos);
    PrefillAbsolute(startPos + direction);
    PrefillAbsolute(startPos + direction * 2);
    
    m_animatingTargetIndex = m_drumTargetPosition;
    m_animatingOldIndexOffset = 999999;
    
    m_lastIntegerPosition = startPos;
}

void TrackDrum::Update(float deltaTime, const ConfigManager* config) {
    if (!config) return;
    
    if (!config->GetTrackDrum().EnableTrackDrum) {
        m_drumAbsolutePosition = static_cast<float>(m_drumTargetPosition);
    } else {
        if (m_drumAbsolutePosition != static_cast<float>(m_drumTargetPosition)) {
            float dampingFactor = static_cast<float>(config->GetTrackDrum().MaxSpeed) * deltaTime;
            if (dampingFactor > 1.0f) dampingFactor = 1.0f;
            
            m_drumAbsolutePosition += (static_cast<float>(m_drumTargetPosition) - m_drumAbsolutePosition) * dampingFactor;
            
            int currentPosRound = static_cast<int>(std::round(m_drumAbsolutePosition));
            if (currentPosRound != m_lastIntegerPosition) {
                // 現在の絶対位置(中央) + 進行方向 を正しく計算して注入する完全修復ロジック
                int direction = currentPosRound - m_lastIntegerPosition;
                PrefillAbsolute(currentPosRound + direction);
                m_lastIntegerPosition = currentPosRound;
            }

            if (std::abs(m_drumAbsolutePosition - static_cast<float>(m_drumTargetPosition)) < 0.001f) {
                m_drumAbsolutePosition = static_cast<float>(m_drumTargetPosition);
                m_lastIntegerPosition = m_drumTargetPosition;
                
                PrefillAbsolute(m_drumTargetPosition - 1);
                PrefillAbsolute(m_drumTargetPosition + 1);
                
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

void TrackDrum::PrefillAbsolute(int absolutePos) {
    if (!m_drumDataProvider) return;
    int slotIdx = (absolutePos % 3 + 3) % 3;
    // 物理的な絶対座標(absolutePos)とターゲット座標(m_drumTargetPosition)の差分を求める。
    // UI側のスクロール方向（NEXTでドラムの座標が減少）とプレイリストの進行（NEXTでインデックス増加）が
    // 逆ベクトルであるため、m_drumTargetPosition - absolutePos と符号を反転させることで
    // 物理座標と論理インデックスを正しくマッピングする。スケーリングを加味して算出する。
    int relativeIndex = static_cast<int>(std::round((m_drumTargetPosition - absolutePos) * m_indexStep));
    TrackMetadata meta = m_drumDataProvider(relativeIndex, &m_drumSlots[slotIdx]);
    m_drumSlots[slotIdx].artBitmap = nullptr;
    m_drumSlots[slotIdx].trackTitle = meta.title;
    m_drumSlots[slotIdx].trackArtist = meta.artist;
    m_drumSlots[slotIdx].trackNumber = meta.timeString;
}

void TrackDrum::OnSlotAnimationCompleted() {
    if (m_drumOnComplete) {
        auto cb = m_drumOnComplete;
        m_drumOnComplete = nullptr;
        m_drumDataProvider = nullptr;
        cb();
    }
}
