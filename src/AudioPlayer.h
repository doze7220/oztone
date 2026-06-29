#pragma once
#include <string>

struct ma_engine;

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

private:
    ma_engine* m_engine;
    bool m_initialized;
};
