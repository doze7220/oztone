#pragma once
#include <string>
#include <mutex>
#include <unordered_map>
#include <list>
#include <vector>
#include <unordered_set>
#include <atomic>
#include <d2d1_1.h>
#include <wrl/client.h>
#include "ConfigManager.h"

struct IWICImagingFactory;

struct SectorInfo {
    uint64_t offset;
    size_t size;
};

class ThumbnailDatabase {
public:
    static constexpr uint32_t NO_ART_THUMB_ID = 0;

    ThumbnailDatabase(ConfigManager* config);

    ~ThumbnailDatabase();

    void Initialize();

    uint32_t GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew);
    bool HasCookedData(uint32_t thumbId);
    ConfigManager* GetConfig() const { return m_config; }
    Microsoft::WRL::ComPtr<ID2D1Bitmap> GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory);
    void DrawThumbnail(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity);

    ID2D1Bitmap* GetCachedThumbnailBitmap(uint32_t thumbId);
    void RequestThumbnailLoad(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory);

    bool StoreCookedData(uint32_t thumbId, const std::wstring& filepath, const std::vector<BYTE>& data);
    void RollbackThumbId(const std::wstring& filepath);

private:
    ConfigManager* m_config;
    std::wstring m_idxPath;
    std::wstring m_imgPath;
    std::mutex m_mutex;
    std::mutex m_ioMutex;

    uint32_t m_nextId = 1;
    std::unordered_map<std::wstring, uint32_t> m_pathToId;
    std::list<uint32_t> m_lruList;
    std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID2D1Bitmap>> m_cache;
    std::unordered_map<uint32_t, SectorInfo> m_sectorMap;
    std::unordered_set<uint32_t> m_loadingSet;
    std::atomic<bool> m_isShuttingDown{false};
    std::atomic<int> m_activeLoadTasks{0};
};
