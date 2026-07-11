#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

struct ArtFramingData {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float scale = 1.0f;
};

class ArtFramingDatabase {
public:
    ArtFramingDatabase();
    ~ArtFramingDatabase();

    bool GetFraming(const std::wstring& filepath, float& outX, float& outY, float& outScale) const;
    void SetFraming(const std::wstring& filepath, float x, float y, float scale);
    void LoadFromFile(const std::wstring& dbPath);
    void SaveToFile(const std::wstring& dbPath) const;

private:
    std::unordered_map<std::wstring, ArtFramingData> m_database;
    mutable std::mutex m_mutex;
};
