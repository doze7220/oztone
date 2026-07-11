#include "TrackDatabase.h"
#include <fstream>
#include <windows.h>
#include <sstream>

TrackDatabase::TrackDatabase() {}

TrackDatabase::~TrackDatabase() {}

void TrackDatabase::LoadFromFile(const std::wstring& dbPath) {
    std::ifstream ifs(dbPath, std::ios::binary);
    if (!ifs) return;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.length(), NULL, 0);
        if (size_needed > 0) {
            std::wstring wline(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.length(), &wline[0], size_needed);

            try {
                std::wstringstream wss(wline);
                std::wstring token;
                std::vector<std::wstring> tokens;
                while (std::getline(wss, token, L'\t')) {
                    tokens.push_back(token);
                }

                if (!tokens.empty()) {
                    std::wstring filepath = tokens[0];
                    TrackMetadata meta;
                    meta.filepath = filepath;

                    if (tokens.size() >= 4) {
                        meta.title = tokens[1];
                        meta.artist = tokens[2];
                        meta.timeString = tokens[3];
                        meta.isMetaLoaded = true;
                    }
                    if (tokens.size() >= 9) {
                        try {
                            meta.peakAmplitude = std::stof(tokens[7]);
                            meta.maxFrequency = std::stof(tokens[8]);
                            if (meta.peakAmplitude > 0.0f) {
                                meta.isFFTLoaded = true;
                            }
                        } catch (...) {
                            meta.peakAmplitude = 0.0f;
                            meta.maxFrequency = 0.0f;
                        }
                    } else if (tokens.size() >= 6) {
                        try {
                            meta.peakAmplitude = std::stof(tokens[4]);
                            meta.maxFrequency = std::stof(tokens[5]);
                            if (meta.peakAmplitude > 0.0f) {
                                meta.isFFTLoaded = true;
                            }
                        } catch (...) {
                            meta.peakAmplitude = 0.0f;
                            meta.maxFrequency = 0.0f;
                        }
                    }

                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_database[filepath] = meta;
                }
            } catch (...) {
                // 個別の行のパースエラーは無視
            }
        }
    }
}

void TrackDatabase::SaveToFile(const std::wstring& dbPath) const {
    std::unordered_map<std::wstring, TrackMetadata> dbCopy;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        dbCopy = m_database;
    }

    std::ofstream ofs(dbPath, std::ios::binary);
    if (!ofs) return;

    for (const auto& pair : dbCopy) {
        const auto& item = pair.second;
        if (item.filepath.empty()) continue;

        std::wstring wline;
        if (item.isMetaLoaded) {
            wline = item.filepath + L"\t" + item.title + L"\t" + item.artist + L"\t" + item.timeString + L"\t" +
                    std::to_wstring(item.peakAmplitude) + L"\t" + std::to_wstring(item.maxFrequency);
        } else {
            wline = item.filepath;
        }

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), (int)wline.length(), NULL, 0, NULL, NULL);
        if (size_needed > 0) {
            std::string utf8Str(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), (int)wline.length(), &utf8Str[0], size_needed, NULL, NULL);
            ofs << utf8Str << "\n";
        }
    }
}

bool TrackDatabase::GetMetadata(const std::wstring& filepath, TrackMetadata& outMeta) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_database.find(filepath);
    if (it != m_database.end()) {
        outMeta = it->second;
        return true;
    }
    return false;
}

void TrackDatabase::SetMetadata(const std::wstring& filepath, const TrackMetadata& meta) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_database[filepath] = meta;
}

void TrackDatabase::UpdateMetadata(const std::wstring& filepath, const TrackMetadata& newData) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_database.find(filepath);
    if (it != m_database.end()) {
        if (newData.isMetaLoaded) {
            it->second.title = newData.title;
            it->second.artist = newData.artist;
            it->second.timeString = newData.timeString;
            it->second.isMetaLoaded = true;
        }
        if (newData.isFFTLoaded) {
            it->second.peakAmplitude = newData.peakAmplitude;
            it->second.maxFrequency = newData.maxFrequency;
            it->second.isFFTLoaded = true;
        }
    } else {
        m_database[filepath] = newData;
    }
}
