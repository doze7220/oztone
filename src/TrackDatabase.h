#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

struct TrackMetadata {
    std::wstring filepath;
    std::wstring title;
    std::wstring artist;
    std::wstring timeString; // 例: "03:45"
    bool isMetaLoaded = false; // 解析済みかどうか（メタデータ）
    bool isFFTLoaded = false;  // 解析済みかどうか（FFT波形）
    float peakAmplitude = 0.0f; // 解析: ピーク振幅
    float maxFrequency = 0.0f;  // 解析: 最大周波数
};

class TrackDatabase {
public:
    TrackDatabase();
    ~TrackDatabase();

    void LoadFromFile(const std::wstring& dbPath);
    void SaveToFile(const std::wstring& dbPath) const;

    bool GetMetadata(const std::wstring& filepath, TrackMetadata& outMeta) const;
    void SetMetadata(const std::wstring& filepath, const TrackMetadata& meta);
    void UpdateMetadata(const std::wstring& filepath, const TrackMetadata& newData);

private:
    std::unordered_map<std::wstring, TrackMetadata> m_database;
    mutable std::mutex m_mutex;
};
