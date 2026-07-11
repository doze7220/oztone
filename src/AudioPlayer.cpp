#define MINIAUDIO_IMPLEMENTATION
#include "AudioPlayer.h"
#include <complex>
#include <cmath>
#include <algorithm>

namespace {
    const size_t FFT_SIZE = 4096;
    const double PI = 3.14159265358979323846;

    size_t ReverseBits(size_t n, int bits) {
        size_t reversed = 0;
        for (int i = 0; i < bits; ++i) {
            if (n & (1ULL << i)) {
                reversed |= (1ULL << ((bits - 1) - i));
            }
        }
        return reversed;
    }

    void PerformFFT(std::vector<std::complex<float>>& x) {
        const size_t N = x.size();
        if (N <= 1) return;
        
        int bits = 0;
        while ((1ULL << bits) < N) bits++;
        
        for (size_t i = 0; i < N; ++i) {
            size_t j = ReverseBits(i, bits);
            if (j > i) {
                std::swap(x[i], x[j]);
            }
        }
        
        for (size_t len = 2; len <= N; len <<= 1) {
            float angle = -2.0f * static_cast<float>(PI) / len;
            std::complex<float> wlen(std::cos(angle), std::sin(angle));
            for (size_t i = 0; i < N; i += len) {
                std::complex<float> w(1.0f, 0.0f);
                for (size_t j = 0; j < len / 2; ++j) {
                    std::complex<float> u = x[i + j];
                    std::complex<float> v = x[i + j + len / 2] * w;
                    x[i + j] = u + v;
                    x[i + j + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }
    }

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

    m_spectrumData.resize(FFT_SIZE / 2, 0.0f);
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
        
        if (m_isLearningValid) {
            float absSample = std::abs(monoSample);
            if (absSample > m_learningPeakAmplitude) {
                m_learningPeakAmplitude = absSample;
            }
        }

        if (volFactor > 0.0f) {
            monoSample *= volFactor;
        } else {
            monoSample = 0.0f;
        }
        m_audioBuffer.push_back(monoSample);
    }
    
    while (m_audioBuffer.size() >= FFT_SIZE) {
        std::vector<std::complex<float>> fftData(FFT_SIZE);
        for (size_t i = 0; i < FFT_SIZE; ++i) {
            float window = 0.5f * (1.0f - std::cos(static_cast<float>(2.0 * PI * i / (FFT_SIZE - 1))));
            fftData[i] = std::complex<float>(m_audioBuffer[i] * window, 0.0f);
        }
        
        m_audioBuffer.erase(m_audioBuffer.begin(), m_audioBuffer.begin() + FFT_SIZE / 2);
        
        PerformFFT(fftData);
        
        std::vector<float> spectrum(FFT_SIZE / 2);
        float localMaxFreqIdx = 0.0f;
        for (size_t i = 0; i < FFT_SIZE / 2; ++i) {
            spectrum[i] = std::abs(fftData[i]);
            if (m_isLearningValid && spectrum[i] > 0.001f) {
                localMaxFreqIdx = static_cast<float>(i);
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
    ma_decoder decoder;
    ma_result result = ma_decoder_init_file_w(filepath.c_str(), NULL, &decoder);
    if (result != MA_SUCCESS) {
        OutputDebugStringW((L"AudioPlayer::ScanAudioData failed to open file: " + filepath + L"\n").c_str());
        return false;
    }

    float peakAmplitude = 0.0f;
    ma_uint32 channels = decoder.outputChannels;
    if (channels == 0) channels = 2;

    const size_t CHUNK_SIZE = 4096;
    std::vector<float> pcmBuffer(CHUNK_SIZE * channels);
    std::vector<float> maxSpectrum(FFT_SIZE / 2, 0.0f);
    
    std::vector<float> audioWindow(FFT_SIZE);
    size_t windowIdx = 0;

    while (true) {
        ma_uint64 framesRead = 0;
        result = ma_decoder_read_pcm_frames(&decoder, pcmBuffer.data(), CHUNK_SIZE, &framesRead);
        if (framesRead == 0) {
            break;
        }

        for (ma_uint64 i = 0; i < framesRead; ++i) {
            float monoSample = 0.0f;
            for (ma_uint32 c = 0; c < channels; ++c) {
                monoSample += pcmBuffer[i * channels + c];
            }
            monoSample /= static_cast<float>(channels);
            
            float absSample = std::abs(monoSample);
            if (absSample > peakAmplitude) {
                peakAmplitude = absSample;
            }

            audioWindow[windowIdx++] = monoSample;
            
            if (windowIdx >= FFT_SIZE) {
                std::vector<std::complex<float>> fftData(FFT_SIZE);
                for (size_t j = 0; j < FFT_SIZE; ++j) {
                    float window = 0.5f * (1.0f - std::cos(static_cast<float>(2.0 * PI * j / (FFT_SIZE - 1))));
                    fftData[j] = std::complex<float>(audioWindow[j] * window, 0.0f);
                }
                
                PerformFFT(fftData);
                
                for (size_t j = 0; j < FFT_SIZE / 2; ++j) {
                    float mag = std::abs(fftData[j]);
                    if (mag > maxSpectrum[j]) {
                        maxSpectrum[j] = mag;
                    }
                }
                
                windowIdx = 0;
            }
        }
    }

    ma_decoder_uninit(&decoder);

    outPeakAmplitude = peakAmplitude;

    outMaxFrequency = static_cast<float>(FFT_SIZE / 2 - 1);
    for (int i = static_cast<int>(FFT_SIZE / 2) - 1; i >= 0; --i) {
        if (maxSpectrum[i] > noiseThreshold) {
            outMaxFrequency = static_cast<float>(i);
            break;
        }
    }

    return true;
}
