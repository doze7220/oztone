#pragma warning(push)
#pragma warning(disable: 4244)
#define MINIAUDIO_IMPLEMENTATION
#include "AudioPlayer.h"
#pragma warning(pop)
#include "AudioAnalyzer.h"
#include <complex>
#include <cmath>
#include <algorithm>

namespace {
    void AudioPlayerOnProcess(void* pUserData, float* pFramesOut, ma_uint64 frameCount) {
        if (pUserData) {
            AudioPlayer* player = static_cast<AudioPlayer*>(pUserData);
            player->ProcessAudioFrames(pFramesOut, frameCount, 0);
        }
    }
}
AudioPlayer::AudioPlayer() : m_engine(nullptr), m_initialized(false), m_isSoundLoaded(false), m_cachedLengthSeconds(0.0f), m_learningPeakAmplitude(0.0f), m_learningMaxFrequency(0.0f), m_isLearningValid(false) {}

AudioPlayer::~AudioPlayer() {
    Uninitialize();
}

bool AudioPlayer::Initialize() {
    if (m_initialized) {
        return true;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.onProcess = AudioPlayerOnProcess;
    engineConfig.pProcessUserData = this;

    m_engine = new ma_engine;
    ma_result result = ma_engine_init(&engineConfig, m_engine);
    if (result != MA_SUCCESS) {
        delete m_engine;
        m_engine = nullptr;
        return false;
    }

    m_spectrumData.resize(AudioAnalyzer::FFT_SIZE / 2, 0.0f);
    m_initialized = true;
    return true;
}

void AudioPlayer::Uninitialize() {
    if (m_isSoundLoaded) {
        ma_sound_uninit(&m_sound);
        m_isSoundLoaded = false;
    }

    if (m_initialized && m_engine) {
        ma_engine_uninit(m_engine);
        delete m_engine;
        m_engine = nullptr;
        m_initialized = false;
    }
}

bool AudioPlayer::HasValidOutputDevice() {
    ma_context context;
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        return false;
    }

    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    ma_device_info* pCaptureDeviceInfos;
    ma_uint32 captureDeviceCount;

    ma_result result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        return false;
    }

    ma_context_uninit(&context);
    return playbackDeviceCount > 0;
}

void AudioPlayer::TogglePlayPause() {
    if (!m_isSoundLoaded) return;
    if (ma_sound_is_playing(&m_sound) == MA_TRUE) {
        ma_sound_stop(&m_sound);
    } else {
        ma_sound_start(&m_sound);
    }
}

void AudioPlayer::Stop() {
    if (!m_isSoundLoaded) return;
    ma_sound_stop(&m_sound);
    ma_sound_seek_to_pcm_frame(&m_sound, 0);
}

void AudioPlayer::Seek(float targetSeconds) {
    if (!m_isSoundLoaded) return;
    if (targetSeconds < 0.0f) targetSeconds = 0.0f;
    if (targetSeconds > m_cachedLengthSeconds) targetSeconds = m_cachedLengthSeconds;
    
    m_isLearningValid = false;
    
    ma_uint32 sampleRate = 0;
    ma_sound_get_data_format(&m_sound, nullptr, nullptr, &sampleRate, nullptr, 0);
    if (sampleRate == 0) sampleRate = 44100;
    
    ma_uint64 targetFrame = static_cast<ma_uint64>(targetSeconds * sampleRate);
    ma_sound_seek_to_pcm_frame(&m_sound, targetFrame);
}

bool AudioPlayer::Play(const std::wstring& filepath) {
    if (!m_initialized || !m_engine) {
        return false;
    }

    if (m_isSoundLoaded) {
        ma_sound_uninit(&m_sound);
        m_isSoundLoaded = false;
    }

    ma_result result = ma_sound_init_from_file_w(m_engine, filepath.c_str(), 0, NULL, NULL, &m_sound);
    if (result != MA_SUCCESS) {
        return false;
    }

    m_isSoundLoaded = true;
    m_learningPeakAmplitude = 0.0f;
    m_learningMaxFrequency = 0.0f;
    m_isLearningValid = true;
    
    float length = 0.0f;
    ma_sound_get_length_in_seconds(&m_sound, &length);
    m_cachedLengthSeconds = length;
    
    result = ma_sound_start(&m_sound);
    return (result == MA_SUCCESS);
}

void AudioPlayer::SetVolume(float volume) {
    if (m_initialized && m_engine) {
        volume = std::clamp(volume, 0.0002f, 1.0f);
        ma_engine_set_volume(m_engine, volume);
    }
}

float AudioPlayer::GetVolume() const {
    if (m_initialized && m_engine) {
        return ma_engine_get_volume(m_engine);
    }
    return 1.0f;
}

float AudioPlayer::GetPositionSeconds() {
    if (!m_isSoundLoaded) {
        return 0.0f;
    }
    float cursor = 0.0f;
    ma_sound_get_cursor_in_seconds(&m_sound, &cursor);
    return cursor;

}

float AudioPlayer::GetLengthSeconds() {
    if (!m_isSoundLoaded) {
        return 0.0f;
    }
    return m_cachedLengthSeconds;
}

bool AudioPlayer::IsPlaying() {
    if (!m_isSoundLoaded) {
        return false;
    }
    return ma_sound_is_playing(&m_sound) == MA_TRUE;
}

bool AudioPlayer::IsAtEnd() {
    if (!m_isSoundLoaded) {
        return false;
    }
    return ma_sound_at_end(&m_sound) == MA_TRUE;
}

void AudioPlayer::ProcessAudioFrames(const float* pFrames, ma_uint64 frameCount, ma_uint32 channels) {
    if (!m_initialized || !m_engine) return;
    
    if (channels == 0) {
        channels = ma_engine_get_channels(m_engine);
    }
    if (channels == 0) channels = 2;

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    
    float currentVol = GetVolume();
    float volFactor = (currentVol > 0.0001f) ? (1.0f / currentVol) : 0.0f;
    
    for (ma_uint64 i = 0; i < frameCount; ++i) {
        float monoSample = 0.0f;
        for (ma_uint32 c = 0; c < channels; ++c) {
            monoSample += pFrames[i * channels + c];
        }
        monoSample /= static_cast<float>(channels);
        

        if (volFactor > 0.0f) {
            monoSample *= volFactor;
        } else {
            monoSample = 0.0f;
        }
        m_audioBuffer.push_back(monoSample);
    }
    
    while (m_audioBuffer.size() >= AudioAnalyzer::FFT_SIZE) {
        std::vector<std::complex<float>> fftData(AudioAnalyzer::FFT_SIZE);
        for (size_t i = 0; i < AudioAnalyzer::FFT_SIZE; ++i) {
            float window = 0.5f * (1.0f - std::cos(static_cast<float>(2.0 * AudioAnalyzer::PI * i / (AudioAnalyzer::FFT_SIZE - 1))));
            fftData[i] = std::complex<float>(m_audioBuffer[i] * window, 0.0f);
        }
        
        m_audioBuffer.erase(m_audioBuffer.begin(), m_audioBuffer.begin() + AudioAnalyzer::FFT_SIZE / 2);
        
        AudioAnalyzer::PerformFFT(fftData);
        
        std::vector<float> spectrum(AudioAnalyzer::FFT_SIZE / 2);
        float localMaxFreqIdx = 0.0f;
        for (size_t i = 0; i < AudioAnalyzer::FFT_SIZE / 2; ++i) {
            spectrum[i] = std::abs(fftData[i]);
            if (m_isLearningValid) {
                if (spectrum[i] > m_learningPeakAmplitude) {
                    m_learningPeakAmplitude = spectrum[i];
                }
                if (spectrum[i] > 0.001f) {
                    localMaxFreqIdx = static_cast<float>(i);
                }
            }
        }
        
        if (m_isLearningValid && localMaxFreqIdx > m_learningMaxFrequency) {
            m_learningMaxFrequency = localMaxFreqIdx;
        }
        
        {
            std::lock_guard<std::mutex> specLock(m_spectrumMutex);
            m_spectrumData = std::move(spectrum);
        }
    }
}

void AudioPlayer::GetSpectrumData(std::vector<float>& outSpectrum) {
    std::lock_guard<std::mutex> lock(m_spectrumMutex);
    outSpectrum = m_spectrumData;
}

bool AudioPlayer::ScanAudioData(const std::wstring& filepath, float noiseThreshold, float& outPeakAmplitude, float& outMaxFrequency) {
    return AudioAnalyzer::ScanAudioData(filepath, noiseThreshold, outPeakAmplitude, outMaxFrequency);
}

