#pragma once
#include <string>
#include "miniaudio.h"

/**
 * @brief 音声再生（バックグラウンドMP3再生）を管理するクラス
 */
class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

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
     * @brief 指定されたMP3ファイルの再生
     * @param filepath 再生するファイルのパス
     * @return 成功ならtrue
     */
    bool Play(const std::string& filepath);

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

private:
    ma_engine* m_engine;
    bool m_initialized;
    ma_sound m_sound;
    bool m_isSoundLoaded;
    float m_cachedLengthSeconds;
};
