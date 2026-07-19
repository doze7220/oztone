#pragma once
#include <string>
#include <mutex>
#include <unordered_map>
#include <list>
#include <d2d1_1.h>
#include <wrl/client.h>
#include "ConfigManager.h"

class ThumbnailDatabase {
public:
    ThumbnailDatabase(ConfigManager* config);
    ~ThumbnailDatabase();

    void Initialize();

    uint32_t GetThumbnailId(const std::wstring& filepath);
    void DrawThumbnail(ID2D1DeviceContext* context, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity);

private:
    ConfigManager* m_config;
    std::wstring m_idxPath;
    std::wstring m_imgPath;
    std::mutex m_mutex;

    uint32_t m_nextId = 1;
    std::unordered_map<std::wstring, uint32_t> m_pathToId;
    std::list<uint32_t> m_lruList;
    std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID2D1Bitmap>> m_cache;
};
