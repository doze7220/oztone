#pragma once

#include <Windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <wincodec.h>
#include <wrl/client.h>

/**
 * @brief 背景画像のロードおよびクロスフェードアニメーションを統括する司令塔
 */
class BackgroundManager {
public:
    BackgroundManager();
    ~BackgroundManager();

    BackgroundManager(const BackgroundManager&) = delete;
    BackgroundManager& operator=(const BackgroundManager&) = delete;

    /**
     * @brief 初期化処理。ワーカー・スレッドを起動する
     */
    void Initialize();

    /**
     * @brief 終了処理。ワーカー・スレッドを安全に停止・破棄する
     */
    void Uninitialize();

    /**
     * @brief 背景画像のロードを非同期で要求する
     * @param filePath 画像を抽出するファイルパス
     */
    void RequestLoad(const std::wstring& filePath);

    /**
     * @brief アニメーションの状態を更新する
     * @param deltaTime 前回フレームからの経過時間（秒）
     */
    void UpdateAnimation(float deltaTime);

    Microsoft::WRL::ComPtr<IWICFormatConverter> GetCurrentWicImage() const { return m_currentWicImage; }
    Microsoft::WRL::ComPtr<IWICFormatConverter> GetOldWicImage() const { return m_oldWicImage; }
    float GetFadeProgress() const { return m_fadeProgress; }

private:
    /**
     * @brief 非同期画像ロードを行うワーカー・スレッドのメインループ
     */
    void WorkerLoop();

private:
    std::thread m_workerThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_isRunning;

    std::queue<std::wstring> m_requestQueue;
    
    // ワーカーからの受け渡し用
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_nextWicImage;
    bool m_hasNewImage;

    // メインスレッド（描画・状態管理）用
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_currentWicImage;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_oldWicImage;
    float m_fadeProgress;
};
