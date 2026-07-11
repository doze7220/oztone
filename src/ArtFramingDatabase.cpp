#include "ArtFramingDatabase.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>

ArtFramingDatabase::ArtFramingDatabase() {}

ArtFramingDatabase::~ArtFramingDatabase() {}

bool ArtFramingDatabase::GetFraming(const std::wstring& filepath, float& outX, float& outY, float& outScale) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_database.find(filepath);
    if (it != m_database.end()) {
        outX = it->second.offsetX;
        outY = it->second.offsetY;
        outScale = it->second.scale;
        return true;
    }
    return false;
}

void ArtFramingDatabase::SetFraming(const std::wstring& filepath, float x, float y, float scale) {
    std::lock_guard<std::mutex> lock(m_mutex);
    ArtFramingData data;
    data.offsetX = x;
    data.offsetY = y;
    data.scale = scale;
    m_database[filepath] = data;
}

void ArtFramingDatabase::LoadFromFile(const std::wstring& dbPath) {
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

                if (tokens.size() >= 4) {
                    std::wstring filepath = tokens[0];
                    ArtFramingData data;
                    data.offsetX = std::stof(tokens[1]);
                    data.offsetY = std::stof(tokens[2]);
                    data.scale = std::stof(tokens[3]);

                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_database[filepath] = data;
                }
            } catch (...) {
                // 不正な行はスキップ
            }
        }
    }
}

void ArtFramingDatabase::SaveToFile(const std::wstring& dbPath) const {
    std::unordered_map<std::wstring, ArtFramingData> dbCopy;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        dbCopy = m_database;
    }

    std::ofstream ofs(dbPath, std::ios::binary);
    if (!ofs) return;

    for (const auto& pair : dbCopy) {
        if (pair.first.empty()) continue;

        std::wstring wline = pair.first + L"\t" +
                             std::to_wstring(pair.second.offsetX) + L"\t" +
                             std::to_wstring(pair.second.offsetY) + L"\t" +
                             std::to_wstring(pair.second.scale);

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), (int)wline.length(), NULL, 0, NULL, NULL);
        if (size_needed > 0) {
            std::string utf8Str(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), (int)wline.length(), &utf8Str[0], size_needed, NULL, NULL);
            ofs << utf8Str << "\n";
        }
    }
}
