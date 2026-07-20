#pragma once

#include <string>
#include <vector>
#include <memory>
#include <d2d1_1.h>
#include <wrl/client.h>

class ConfigManager;
class ThumbnailDatabase;
class ThumbCacher;
struct IWICImagingFactory;

class ThumbnailManager
{
public:
    ThumbnailManager(ConfigManager* config);
    ~ThumbnailManager();

    void Initialize();
    void Uninitialize();

    // Database facade
    uint32_t GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew);
    bool HasCookedData(uint32_t thumbId);
    Microsoft::WRL::ComPtr<ID2D1Bitmap> GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory);
    void DrawThumbnail(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity);
    ID2D1Bitmap* GetCachedThumbnailBitmap(uint32_t thumbId);
    void RequestThumbnailLoad(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory);

    // Cacher facade
    void EnqueueTrack(uint32_t thumbId, const std::wstring& filepath);

private:
    std::unique_ptr<ThumbnailDatabase> m_database;
    std::unique_ptr<ThumbCacher> m_cacher;
};
