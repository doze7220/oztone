#pragma once

#include <windows.h>
#include <string>
#include <vector>
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

    void EnqueueTrack(uint32_t thumbId, const std::wstring& filepath);

    std::vector<BYTE> CookThumbnailImage(const std::vector<BYTE>& rawBinary, UINT targetSize, float jpegQuality);

private:
    void WorkerLoop();

    ThumbnailDatabase* m_db;
    std::thread m_workerThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::pair<uint32_t, std::wstring>> m_taskQueue;
    std::atomic<bool> m_stopFlag;
};
