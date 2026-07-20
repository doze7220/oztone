#pragma once
#include <string>
#include <vector>
#include <complex>

/**
 * @brief 音声データの解析（FFT等）を担当するコンポーネント
 */
class AudioAnalyzer {
public:
    static const size_t FFT_SIZE = 4096;
    static const double PI;

    /**
     * @brief 音声ファイルを高速でスキャンし、最大振幅と最高周波数を算出する
     * @param filepath スキャンする音声ファイルのパス
     * @param noiseThreshold 高音域のノイズ足切り閾値
     * @param outPeakAmplitude 出力: 最大振幅
     * @param outMaxFrequency 出力: 有効最高高音周波数（インデックス等）
     * @return スキャンに成功した場合はtrue
     */
    static bool ScanAudioData(const std::wstring& filepath, float noiseThreshold, float& outPeakAmplitude, float& outMaxFrequency);

    /**
     * @brief FFT解析を実行する
     */
    static void PerformFFT(std::vector<std::complex<float>>& x);
};
