#include "TrackAnalyzer.h"
#include "PlaylistManager.h"
#include "ConfigManager.h"
#include "AudioPlayer.h"
#include "TagManager.h"
#include <windows.h>
#include <filesystem>

TrackAnalyzer::TrackAnalyzer() {}

TrackAnalyzer::~TrackAnalyzer() {
    Uninitialize();
}

void TrackAnalyzer::Initialize(PlaylistManager* playlistManager, ConfigManager* configManager) {
    m_playlistManager = playlistManager;
    m_configManager = configManager;
    
    m_parseThreadRunning.store(true);
    m_parseThread = std::thread(&TrackAnalyzer::ParseThreadFunc, this);
}

void TrackAnalyzer::Uninitialize() {
    m_parseThreadRunning.store(false);
    m_parseCV.notify_all();
    if (m_parseThread.joinable()) {
        m_parseThread.join();
    }
}

void TrackAnalyzer::AddTrackToQueue(const std::wstring& filepath) {
    {
        std::lock_guard<std::mutex> lock(m_parseMutex);
        m_parseQueue.push(filepath);
    }
    m_parseCV.notify_one();
}

void TrackAnalyzer::ClearQueue() {
    std::lock_guard<std::mutex> lock(m_parseMutex);
    std::queue<std::wstring> empty;
    std::swap(m_parseQueue, empty);
}

void TrackAnalyzer::ParseThreadFunc() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    TagManager localTagManager;

    while (m_parseThreadRunning.load()) {
        std::wstring targetPath;
        {
            std::unique_lock<std::mutex> lock(m_parseMutex);
            m_parseCV.wait(lock, [this]() {
                return !m_parseQueue.empty() || !m_parseThreadRunning.load();
            });

            if (!m_parseThreadRunning.load())
                break;

            targetPath = m_parseQueue.front();
            m_parseQueue.pop();
        }

        if (!m_playlistManager || !m_configManager) continue;

        TrackMetadata currentMeta;
        bool hasMeta = m_playlistManager->GetTrackMetadata(targetPath, currentMeta);
        bool isFFTLoaded = hasMeta && currentMeta.isFFTLoaded;
        bool isMetaLoaded = hasMeta && currentMeta.isMetaLoaded;
        bool needsScan = hasMeta && (currentMeta.peakAmplitude <= 1.0f) && m_configManager->GetEnablePreScan();

        if (isFFTLoaded && isMetaLoaded && !needsScan) {
            continue;
        }

        bool updated = false;

        if (!isMetaLoaded) {
            std::wstring title, artist, timeString;
            try {
                if (localTagManager.Load(targetPath)) {
                    title = localTagManager.GetTitle();
                    artist = localTagManager.GetArtist();
                    timeString = localTagManager.GetTimeString();
                    if (title.empty()) {
                        try { title = std::filesystem::path(targetPath).filename().wstring(); } catch (...) { title = L"UNKNOWN"; }
                    }
                    if (artist.empty()) artist = L"---";
                } else {
                    try { title = std::filesystem::path(targetPath).filename().wstring(); } catch (...) { title = L"UNKNOWN"; }
                    artist = L"---";
                }
            } catch (...) {
                try { title = std::filesystem::path(targetPath).filename().wstring(); } catch (...) { title = L"UNKNOWN"; }
                artist = L"---";
            }
            currentMeta.title = title;
            currentMeta.artist = artist;
            currentMeta.timeString = timeString;
            currentMeta.isMetaLoaded = true;
            m_playlistManager->UpdateMetadata(currentMeta);
            updated = true;
        }

        if (!isFFTLoaded || needsScan) {
            if (m_configManager->GetEnablePreScan()) {
                float peakAmplitude = 0.0f;
                float maxFrequency = 0.0f;
                float noiseThreshold = m_configManager->GetHighFreqNoiseThreshold();
                if (AudioPlayer::ScanAudioData(targetPath, noiseThreshold, peakAmplitude, maxFrequency)) {
                    m_playlistManager->UpdateScanData(targetPath, peakAmplitude, maxFrequency);
                } else {
                    m_playlistManager->UpdateScanData(targetPath, 1.0f, static_cast<float>(2048 - 1));
                }
                currentMeta.isFFTLoaded = true;
                updated = true;
            }
        }

        bool shouldSave = false;
        {
            std::lock_guard<std::mutex> lock(m_parseMutex);
            if (m_parseQueue.empty()) {
                shouldSave = true;
            }
        }
        if (shouldSave && updated) {
            m_playlistManager->SaveToFile(m_configManager->GetDefaultPlaylistPath());
        }
    }

    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
}
