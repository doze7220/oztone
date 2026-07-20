#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "miniaudio.h"

/**
 * @brief 音声再生（バックグラウンドMP3再生）を管理するクラス
 */
class AudioPlaybackEngine {
public:
    AudioPlaybackEngine();
    ~AudioPlaybackEngine();

    /**
     * @brief miniaudioエンジンの初期化
     * @return 成功ならtrue
     */
    bool Initialize();

    /**
     * @brief エンジンの解放
     */
    void Uninitialize();

    /**
     * @brief 再生/一時停止を切り替える
     */
    void TogglePlayPause();

    /**
     * @brief 再生を停止し、先頭に巻き戻す
     */
    void Stop();

    /**
     * @brief 指定秒数へシークする
     */
    void Seek(float targetSeconds);

    /**
     * @brief 指定されたMP3ファイルの再生
     * @param filepath 再生するファイルのパス
     * @return 成功ならtrue
     */
    bool Play(const std::wstring& filepath);

    /**
     * @brief 音声ファイルを高速でスキャンし、最大振幅と最高周波数を算出する
     * @deprecated タスク4にてAudioManagerへ移行予定
     */
    static bool ScanAudioData(const std::wstring& filepath, float noiseThreshold, float& outPeakAmplitude, float& outMaxFrequency);


    /**
     * @brief 音量を設定する (0.0f - 1.0f)
     */
    void SetVolume(float volume);

    /**
     * @brief 現在有効な出力デバイスが存在するかを確認する
     * @return 出力デバイスが1つ以上あればtrue
     */
    bool HasValidOutputDevice();


    /**
     * @brief 現在の音量を取得する
     */
    float GetVolume() const;

    /**
     * @brief 現在の再生位置（秒）を取得
     * @return 再生位置（秒）
     */
    float GetPositionSeconds();

    /**
     * @brief 曲の総時間（秒）を取得
     * @return 総時間（秒）
     */
    float GetLengthSeconds();

    /**
     * @brief 現在再生中かどうかを取得
     * @return 再生中ならtrue
     */
    bool IsPlaying();

    /**
     * @brief 現在の音声が最後まで再生されたか検知する
     * @return 終了していればtrue
     */
    bool IsAtEnd();

    bool IsLearningValid() const { return m_isLearningValid; }
    float GetLearningPeakAmplitude() const { return m_learningPeakAmplitude; }
    float GetLearningMaxFrequency() const { return m_learningMaxFrequency; }

    /**
     * @brief スペクトルデータ（FFT結果）を取得する
     * @param outSpectrum 取得したスペクトルデータを格納するベクター
     */
    void GetSpectrumData(std::vector<float>& outSpectrum);

    /**
     * @brief 内部用：音声フレームを処理し、バッファリングとFFT解析を行う
     */
    void ProcessAudioFrames(const float* pFrames, ma_uint64 frameCount, ma_uint32 channels);

private:
    ma_engine* m_engine;
    bool m_initialized;
    ma_sound m_sound;
    bool m_isSoundLoaded;
    float m_cachedLengthSeconds;

    std::vector<float> m_audioBuffer;
    std::mutex m_bufferMutex;
    std::vector<float> m_spectrumData;
    std::mutex m_spectrumMutex;

    float m_learningPeakAmplitude;
    float m_learningMaxFrequency;
    bool m_isLearningValid;
};

using AudioPlayer = AudioPlaybackEngine;
