#include "ThumbCacher.h"
#include "ThumbnailDatabase.h"

ThumbCacher::ThumbCacher(ThumbnailDatabase* db)
    : m_db(db)
    , m_stopFlag(false)
{
}

ThumbCacher::~ThumbCacher()
{
    Uninitialize();
}

void ThumbCacher::Initialize()
{
    if (!m_workerThread.joinable())
    {
        m_stopFlag = false;
        m_workerThread = std::thread(&ThumbCacher::WorkerLoop, this);
    }
}

void ThumbCacher::Uninitialize()
{
    if (m_workerThread.joinable())
    {
        m_stopFlag = true;
        m_cv.notify_all();
        m_workerThread.join();
    }
}

void ThumbCacher::EnqueueTrack(const std::wstring& filepath)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskQueue.push(filepath);
    }
    m_cv.notify_one();
}

void ThumbCacher::WorkerLoop()
{
    while (true)
    {
        std::wstring filepath;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() { return m_stopFlag || !m_taskQueue.empty(); });

            // 終了フラグが立ったら直ちにループを抜ける（キューに残りがあっても破棄して終了）
            if (m_stopFlag)
            {
                break;
            }

            filepath = m_taskQueue.front();
            m_taskQueue.pop();
        }

        if (filepath.empty())
        {
            continue;
        }

        // TODO: WICでサムネイル生成しThumbnailDatabaseへ追記
    }
}
