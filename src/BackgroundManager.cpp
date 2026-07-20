#include "BackgroundManager.h"
#include <objbase.h>

BackgroundManager::BackgroundManager()
    : m_isRunning(false)
{
}

BackgroundManager::~BackgroundManager()
{
    Uninitialize();
}

void BackgroundManager::Initialize()
{
    if (m_isRunning) {
        return;
    }

    m_isRunning = true;
    m_workerThread = std::thread(&BackgroundManager::WorkerLoop, this);
}

void BackgroundManager::Uninitialize()
{
    if (!m_isRunning) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning = false;
    }
    // ワーカー・スレッドを起床させて終了を促す
    m_cv.notify_one();

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void BackgroundManager::WorkerLoop()
{
    // WICデコード等のため、バックグラウンドスレッドでもCOMを初期化する
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    
    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        m_cv.wait(lock, [this]() {
            // 終了フラグが立つか、新規ロードタスクが積まれるまで待機する
            // TODO: タスク2でキューの有無のチェック条件を追加する
            return !m_isRunning; 
        });

        if (!m_isRunning) {
            break;
        }

        // TODO: タスク2でFileManagerを用いた非同期画像抽出とWICデコード処理を実装する
    }

    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
}
