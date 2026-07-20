#include "AudioAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <windows.h>
#include "miniaudio.h"

const double AudioAnalyzer::PI = 3.14159265358979323846;

namespace {
    size_t ReverseBits(size_t n, int bits) {
        size_t reversed = 0;
        for (int i = 0; i < bits; ++i) {
            if (n & (1ULL << i)) {
                reversed |= (1ULL << ((bits - 1) - i));
            }
        }
        return reversed;
    }
}

void AudioAnalyzer::PerformFFT(std::vector<std::complex<float>>& x) {
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

bool AudioAnalyzer::ScanAudioData(const std::wstring& filepath, float noiseThreshold, float& outPeakAmplitude, float& outMaxFrequency) {
    ma_decoder decoder;
    ma_result result = ma_decoder_init_file_w(filepath.c_str(), NULL, &decoder);
    if (result != MA_SUCCESS) {
        OutputDebugStringW((L"AudioAnalyzer::ScanAudioData failed to open file: " + filepath + L"\n").c_str());
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
                    if (mag > peakAmplitude) {
                        peakAmplitude = mag;
                    }
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
