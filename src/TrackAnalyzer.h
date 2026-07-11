#pragma once
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class PlaylistManager;
class ConfigManager;

class TrackAnalyzer {
public:
    TrackAnalyzer();
    ~TrackAnalyzer();

    void Initialize(PlaylistManager* playlistManager, ConfigManager* configManager);
    void Uninitialize();

    void AddTrackToQueue(const std::wstring& filepath);
    void ClearQueue();

private:
    void ParseThreadFunc();

    PlaylistManager* m_playlistManager = nullptr;
    ConfigManager* m_configManager = nullptr;

    std::thread m_parseThread;
    std::mutex m_parseMutex;
    std::condition_variable m_parseCV;
    std::queue<std::wstring> m_parseQueue;
    std::atomic<bool> m_parseThreadRunning{false};
};
