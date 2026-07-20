#include "ThumbnailManager.h"
#include "ThumbnailDatabase.h"
#include "ThumbCacher.h"

ThumbnailManager::ThumbnailManager(ConfigManager* config)
{
    m_database = std::make_unique<ThumbnailDatabase>(config);
    m_cacher = std::make_unique<ThumbCacher>(m_database.get());
}

ThumbnailManager::~ThumbnailManager()
{
}

void ThumbnailManager::Initialize()
{
    m_database->Initialize();
    m_cacher->Initialize();
}

void ThumbnailManager::Uninitialize()
{
    m_cacher->Uninitialize();
}

uint32_t ThumbnailManager::GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew)
{
    uint32_t id = m_database->GetOrGenerateThumbId(filepath, out_isNew);
    if (out_isNew)
    {
        m_cacher->EnqueueTrack(id, filepath);
    }
    return id;
}

bool ThumbnailManager::HasCookedData(uint32_t thumbId)
{
    return m_database->HasCookedData(thumbId);
}

Microsoft::WRL::ComPtr<ID2D1Bitmap> ThumbnailManager::GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory)
{
    return m_database->GetThumbnailBitmap(thumbId, renderTarget, wicFactory);
}

void ThumbnailManager::DrawThumbnail(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity)
{
    m_database->DrawThumbnail(context, wicFactory, thumbId, destRect, opacity);
}

ID2D1Bitmap* ThumbnailManager::GetCachedThumbnailBitmap(uint32_t thumbId)
{
    return m_database->GetCachedThumbnailBitmap(thumbId);
}

void ThumbnailManager::RequestThumbnailLoad(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory)
{
    m_database->RequestThumbnailLoad(thumbId, renderTarget, wicFactory);
}

void ThumbnailManager::EnqueueTrack(uint32_t thumbId, const std::wstring& filepath)
{
    m_cacher->EnqueueTrack(thumbId, filepath);
}
