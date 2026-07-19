#include "ThumbnailDatabase.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <wincodec.h>
#include <thread>

#define OZTHUMB_MAGIC "OZTHUMB_V1"

ThumbnailDatabase::ThumbnailDatabase(ConfigManager* config)
    : m_config(config)
{
    if (m_config) {
        std::wstring exeDir = m_config->GetExecutablePath();
        std::filesystem::path exePath(exeDir);
        m_idxPath = (exePath.parent_path() / L"oztone_track_thumb_idx.odb").wstring();
        m_imgPath = (exePath.parent_path() / L"oztone_track_thumb_img.odb").wstring();
    } else {
        m_idxPath = L"oztone_track_thumb_idx.odb";
        m_imgPath = L"oztone_track_thumb_img.odb";
    }
}

ThumbnailDatabase::~ThumbnailDatabase() {}

void ThumbnailDatabase::Initialize() {
    if (!m_config) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    float configSize = m_config->GetThumbnailSize();
    bool needReset = true;

    // ヘッダ検証 (idx)
    std::ifstream ifs(m_idxPath, std::ios::binary);
    if (ifs) {
        std::string line;
        if (std::getline(ifs, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // ヘッダフォーマット: マジックナンバー \t ThumbnailSize
            std::stringstream ss(line);
            std::string magic, sizeStr;
            if (std::getline(ss, magic, '\t') && std::getline(ss, sizeStr)) {
                if (magic == OZTHUMB_MAGIC) {
                    try {
                        float dbSize = std::stof(sizeStr);
                        if (std::abs(dbSize - configSize) < 0.1f) {
                            needReset = false;
                        }
                    } catch (...) {
                        // パース失敗時はリセット
                    }
                }
            }
        }
        
        if (!needReset) {
            uint32_t maxId = 0;
            while (std::getline(ifs, line)) {
                if (line.empty()) continue;
                if (line.back() == '\r') line.pop_back();
                
                std::stringstream ss(line);
                std::string idStr, offsetStr, sizeStr, filepathStr;
                if (std::getline(ss, idStr, '\t') && std::getline(ss, offsetStr, '\t') && std::getline(ss, sizeStr, '\t') && std::getline(ss, filepathStr)) {
                    try {
                        uint32_t id = std::stoul(idStr);
                        uint64_t offset = std::stoull(offsetStr);
                        size_t size = std::stoull(sizeStr);
                        m_sectorMap[id] = {offset, size};
                        std::wstring wfilepath = std::filesystem::path(filepathStr).wstring();
                        m_pathToId[wfilepath] = id;
                        if (id > maxId) maxId = id;
                    } catch (...) {
                        // パース失敗行は無視
                    }
                }
            }
            m_nextId = maxId + 1;
        }

        ifs.close();
    }

    if (needReset) {
        // 既存キャッシュの全破棄とヘッダの再構築
        std::ofstream ofsIdx(m_idxPath, std::ios::binary | std::ios::trunc);
        if (ofsIdx) {
            ofsIdx << OZTHUMB_MAGIC << "\t" << configSize << "\n";
            ofsIdx.close();
        }

        // imgファイルもサイズ0で初期化
        std::ofstream ofsImg(m_imgPath, std::ios::binary | std::ios::trunc);
        if (ofsImg) {
            ofsImg.close();
        }
    }
}

uint32_t ThumbnailDatabase::GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_pathToId.find(filepath);
    if (it != m_pathToId.end()) {
        out_isNew = false;
        return it->second;
    }
    
    uint32_t newId = m_nextId++;
    m_pathToId[filepath] = newId;
    out_isNew = true;
    return newId;
}

bool ThumbnailDatabase::HasCookedData(uint32_t thumbId) {
    std::lock_guard<std::mutex> lock(m_ioMutex);
    return m_sectorMap.find(thumbId) != m_sectorMap.end();
}

ID2D1Bitmap* ThumbnailDatabase::GetCachedThumbnailBitmap(uint32_t thumbId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cache.find(thumbId);
    if (it != m_cache.end()) {
        auto listIt = std::find(m_lruList.begin(), m_lruList.end(), thumbId);
        if (listIt != m_lruList.end()) {
            m_lruList.erase(listIt);
        }
        m_lruList.push_front(thumbId);
        return it->second.Get();
    }
    return nullptr;
}

void ThumbnailDatabase::RequestThumbnailLoad(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory) {
    if (!renderTarget || !wicFactory) return;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_cache.find(thumbId) != m_cache.end()) {
            return;
        }
        if (m_loadingSet.find(thumbId) != m_loadingSet.end()) {
            return;
        }
        m_loadingSet.insert(thumbId);
    }

    Microsoft::WRL::ComPtr<ID2D1RenderTarget> rt(renderTarget);
    Microsoft::WRL::ComPtr<IWICImagingFactory> wf(wicFactory);

    std::thread([this, thumbId, rt, wf]() {
        HRESULT hrInit = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
        SectorInfo sector = {0, 0};
        {
            std::lock_guard<std::mutex> ioLock(m_ioMutex);
            auto it = m_sectorMap.find(thumbId);
            if (it != m_sectorMap.end()) {
                sector = it->second;
            }
        }

        std::vector<BYTE> binaryData;
        if (sector.size > 0) {
            std::lock_guard<std::mutex> ioLock(m_ioMutex);
            std::ifstream ifs(m_imgPath, std::ios::binary);
            if (ifs) {
                ifs.clear();
                ifs.seekg(sector.offset, std::ios::beg);
                binaryData.resize(sector.size);
                ifs.read(reinterpret_cast<char*>(binaryData.data()), sector.size);
                if (ifs.fail()) {
                    binaryData.clear();
                }
            }
        }

        if (!binaryData.empty()) {
            Microsoft::WRL::ComPtr<IWICStream> stream;
            if (SUCCEEDED(wf->CreateStream(&stream)) &&
                SUCCEEDED(stream->InitializeFromMemory(binaryData.data(), static_cast<DWORD>(binaryData.size())))) {

                Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
                if (SUCCEEDED(wf->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder))) {

                    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
                    if (SUCCEEDED(decoder->GetFrame(0, &frame))) {

                        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
                        if (SUCCEEDED(wf->CreateFormatConverter(&converter)) &&
                            SUCCEEDED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {

                            rt->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap);
                        }
                    }
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (bitmap) {
                m_cache[thumbId] = bitmap;
                m_lruList.push_front(thumbId);

                size_t maxCache = m_config ? m_config->GetMaxThumbnailCache() : 100;
                while (m_lruList.size() > maxCache) {
                    uint32_t oldestId = m_lruList.back();
                    m_lruList.pop_back();
                    m_cache.erase(oldestId);
                }
            }
            m_loadingSet.erase(thumbId);
        }

        if (SUCCEEDED(hrInit)) {
            CoUninitialize();
        }
    }).detach();
}

Microsoft::WRL::ComPtr<ID2D1Bitmap> ThumbnailDatabase::GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory) {
    if (!renderTarget || !wicFactory) return nullptr;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        // 1. キャッシュヒット
        auto cacheIt = m_cache.find(thumbId);
        if (cacheIt != m_cache.end()) {
            auto listIt = std::find(m_lruList.begin(), m_lruList.end(), thumbId);
            if (listIt != m_lruList.end()) {
                m_lruList.erase(listIt);
            }
            m_lruList.push_front(thumbId);
            return cacheIt->second;
        }
    }

    // 2. キャッシュミス時は sectorMap を確認
    SectorInfo sector = {0, 0};
    {
        std::lock_guard<std::mutex> ioLock(m_ioMutex);
        auto it = m_sectorMap.find(thumbId);
        if (it == m_sectorMap.end()) {
            return nullptr; // まだクックされていないか存在しない
        }
        sector = it->second;
    }

    // IOロック取得してファイル読み込み
    std::vector<BYTE> binaryData;
    if (sector.size > 0) {
        std::lock_guard<std::mutex> ioLock(m_ioMutex);
        std::ifstream ifs(m_imgPath, std::ios::binary);
        if (ifs) {
            ifs.clear();
            ifs.seekg(sector.offset, std::ios::beg);
            binaryData.resize(sector.size);
            ifs.read(reinterpret_cast<char*>(binaryData.data()), sector.size);
            if (ifs.fail()) {
                binaryData.clear();
            }
        }
    }

    if (binaryData.empty()) return nullptr;

    // ミューテックス外で重い処理 (WICデコード)
    Microsoft::WRL::ComPtr<IWICStream> stream;
    if (FAILED(wicFactory->CreateStream(&stream))) return nullptr;

    if (FAILED(stream->InitializeFromMemory(binaryData.data(), static_cast<DWORD>(binaryData.size())))) return nullptr;

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    if (FAILED(wicFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder))) return nullptr;

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, &frame))) return nullptr;

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    if (FAILED(wicFactory->CreateFormatConverter(&converter))) return nullptr;

    if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) return nullptr;

    Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
    if (FAILED(renderTarget->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap))) return nullptr;

    // キャッシュへ登録
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache[thumbId] = bitmap;
        m_lruList.push_front(thumbId);

        // 最大キャッシュサイズ超過時の押し出し
        size_t maxCache = m_config ? m_config->GetMaxThumbnailCache() : 100;
        while (m_lruList.size() > maxCache) {
            uint32_t oldestId = m_lruList.back();
            m_lruList.pop_back();
            m_cache.erase(oldestId);
        }
    }

    return bitmap;
}

void ThumbnailDatabase::DrawThumbnail(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity) {
    if (!context || !wicFactory) return;

    auto bitmap = GetThumbnailBitmap(thumbId, context, wicFactory);
    if (bitmap) {
        context->DrawBitmap(bitmap.Get(), &destRect, opacity, D2D1_INTERPOLATION_MODE_LINEAR, nullptr);
    } else {
        // まだ解析・クック中（どちらにも無い）場合
        // ガラス板（プレースホルダー）を描画
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
        if (SUCCEEDED(context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.1f), &brush))) {
            context->FillRectangle(&destRect, brush.Get());
        }
    }
}

bool ThumbnailDatabase::StoreCookedData(uint32_t thumbId, const std::wstring& filepath, const std::vector<BYTE>& data) {
    if (data.empty()) return false;

    std::lock_guard<std::mutex> lock(m_ioMutex);

    std::ofstream imgOfs(m_imgPath, std::ios::app | std::ios::binary);
    if (!imgOfs) return false;

    imgOfs.seekp(0, std::ios::end);
    uint64_t offset = static_cast<uint64_t>(imgOfs.tellp());
    imgOfs.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (imgOfs.fail()) {
        imgOfs.close();
        return false;
    }
    imgOfs.close();

    std::ofstream idxOfs(m_idxPath, std::ios::app | std::ios::binary);
    if (!idxOfs) return false;

    std::string pathStr = std::filesystem::path(filepath).string();
    // Use binary mode and construct the string to ensure CRLF issues don't happen, or just write it normally
    std::string idxLine = std::to_string(thumbId) + "\t" + std::to_string(offset) + "\t" + std::to_string(data.size()) + "\t" + pathStr + "\n";
    idxOfs.write(idxLine.c_str(), idxLine.size());
    if (idxOfs.fail()) {
        idxOfs.close();
        return false;
    }
    idxOfs.close();

    m_sectorMap[thumbId] = {offset, data.size()};
    return true;
}
