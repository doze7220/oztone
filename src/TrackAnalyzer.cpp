#include "TrackAnalyzer.h"
#include "PlaylistManager.h"
#include "ConfigManager.h"
#include "AudioManager.h"
#include "FileManager.h"
#include <windows.h>
#include <filesystem>

TrackAnalyzer::TrackAnalyzer() {}

TrackAnalyzer::~TrackAnalyzer() {
    Uninitialize();
}

void TrackAnalyzer::Initialize(TrackDatabase* trackDatabase, ConfigManager* configManager) {
    m_trackDatabase = trackDatabase;
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

        if (!m_trackDatabase || !m_configManager) continue;

        TrackMetadata currentMeta;
        bool hasMeta = m_trackDatabase->GetMetadata(targetPath, currentMeta);
        if (!hasMeta) {
            currentMeta.filepath = targetPath;
        }
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
                AudioMetadata fmMeta = FileManager::ExtractTextMetadata(targetPath);
                if (!fmMeta.title.empty() || !fmMeta.artist.empty()) {
                    title = fmMeta.title;
                    artist = fmMeta.artist;
                    wchar_t buf[32];
                    swprintf_s(buf, 32, L"%02d:%02d", fmMeta.durationSeconds / 60, fmMeta.durationSeconds % 60);
                    timeString = buf;
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
            m_trackDatabase->UpdateMetadata(targetPath, currentMeta);
            updated = true;
        }

        if (!isFFTLoaded || needsScan) {
            if (m_configManager->GetEnablePreScan()) {
                float peakAmplitude = 0.0f;
                float maxFrequency = 0.0f;
                float noiseThreshold = m_configManager->GetHighFreqNoiseThreshold();
                if (AudioManager::ScanAudioData(targetPath, noiseThreshold, peakAmplitude, maxFrequency)) {
                    currentMeta.peakAmplitude = peakAmplitude;
                    currentMeta.maxFrequency = maxFrequency;
                } else {
                    currentMeta.peakAmplitude = 1.0f;
                    currentMeta.maxFrequency = static_cast<float>(2048 - 1);
                }
                currentMeta.isFFTLoaded = true;
                m_trackDatabase->UpdateMetadata(targetPath, currentMeta);
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
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(NULL, exePath, MAX_PATH);
            std::wstring dbPath = std::filesystem::path(exePath).parent_path().wstring() + L"\\oztone_track.odb";
            m_trackDatabase->SaveToFile(dbPath);
        }
    }

    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
}
