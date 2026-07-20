#pragma once
#include <string>
#include <vector>
#include <memory>

class AudioPlaybackEngine;

/**
 * @brief 音声に関することをすべて統括する司令塔
 * 
 * 外部（Application等）に対する音声操作の窓口を単一化し、
 * 内部で AudioPlaybackEngine および AudioAnalyzer をカプセル化・統括する。
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool Initialize();
    void Uninitialize();

    void TogglePlayPause();
    void Stop();
    void Seek(float targetSeconds);
    bool Play(const std::wstring& filepath);

    void SetVolume(float volume);
    float GetVolume() const;

    bool HasValidOutputDevice();
    float GetPositionSeconds();
    float GetLengthSeconds();
    bool IsPlaying();
    bool IsAtEnd();

    bool IsLearningValid() const;
    float GetLearningPeakAmplitude() const;
    float GetLearningMaxFrequency() const;

    void GetSpectrumData(std::vector<float>& outSpectrum);

    static bool ScanAudioData(const std::wstring& filepath, float noiseThreshold, float& outPeakAmplitude, float& outMaxFrequency);

private:
    std::unique_ptr<AudioPlaybackEngine> m_playbackEngine;
};
