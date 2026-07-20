#pragma once

#include <Windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

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
};
