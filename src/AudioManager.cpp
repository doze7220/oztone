#include "AudioManager.h"
#include "AudioPlayer.h" // AudioPlaybackEngine
#include "AudioAnalyzer.h"

AudioManager::AudioManager()
    : m_playbackEngine(std::make_unique<AudioPlaybackEngine>())
{
}

AudioManager::~AudioManager()
{
}

bool AudioManager::Initialize()
{
    return m_playbackEngine->Initialize();
}

void AudioManager::Uninitialize()
{
    m_playbackEngine->Uninitialize();
}

void AudioManager::TogglePlayPause()
{
    m_playbackEngine->TogglePlayPause();
}

void AudioManager::Stop()
{
    m_playbackEngine->Stop();
}

void AudioManager::Seek(float targetSeconds)
{
    m_playbackEngine->Seek(targetSeconds);
}

bool AudioManager::Play(const std::wstring& filepath)
{
    return m_playbackEngine->Play(filepath);
}

void AudioManager::SetVolume(float volume)
{
    m_playbackEngine->SetVolume(volume);
}

float AudioManager::GetVolume() const
{
    return m_playbackEngine->GetVolume();
}

bool AudioManager::HasValidOutputDevice()
{
    return m_playbackEngine->HasValidOutputDevice();
}

float AudioManager::GetPositionSeconds()
{
    return m_playbackEngine->GetPositionSeconds();
}

float AudioManager::GetLengthSeconds()
{
    return m_playbackEngine->GetLengthSeconds();
}

bool AudioManager::IsPlaying()
{
    return m_playbackEngine->IsPlaying();
}

bool AudioManager::IsAtEnd()
{
    return m_playbackEngine->IsAtEnd();
}

bool AudioManager::IsLearningValid() const
{
    return m_playbackEngine->IsLearningValid();
}

float AudioManager::GetLearningPeakAmplitude() const
{
    return m_playbackEngine->GetLearningPeakAmplitude();
}

float AudioManager::GetLearningMaxFrequency() const
{
    return m_playbackEngine->GetLearningMaxFrequency();
}

void AudioManager::GetSpectrumData(std::vector<float>& outSpectrum)
{
    m_playbackEngine->GetSpectrumData(outSpectrum);
}

bool AudioManager::ScanAudioData(const std::wstring& filepath, float noiseThreshold, float& outPeakAmplitude, float& outMaxFrequency)
{
    return AudioAnalyzer::ScanAudioData(filepath, noiseThreshold, outPeakAmplitude, outMaxFrequency);
}
