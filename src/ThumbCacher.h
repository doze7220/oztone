#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

class ThumbnailDatabase;

class ThumbCacher
{
public:
    ThumbCacher(ThumbnailDatabase* db);
    ~ThumbCacher();

    void Initialize();
    void Uninitialize();

    void EnqueueTrack(const std::wstring& filepath);

private:
    void WorkerLoop();

    ThumbnailDatabase* m_db;
    std::thread m_workerThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::wstring> m_taskQueue;
    std::atomic<bool> m_stopFlag;
};
