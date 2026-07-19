#include "ThumbnailDatabase.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

#define OZTHUMB_MAGIC "OZTHUMB_V1"

ThumbnailDatabase::ThumbnailDatabase(ConfigManager* config)
    : m_config(config)
{
    if (m_config) {
        std::wstring exeDir = m_config->GetExecutablePath();
        m_idxPath = exeDir + L"oztone_track_thumb_idx.odb";
        m_imgPath = exeDir + L"oztone_track_thumb_img.odb";
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

uint32_t ThumbnailDatabase::GetThumbnailId(const std::wstring& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_pathToId.find(filepath);
    if (it != m_pathToId.end()) {
        return it->second;
    }
    
    uint32_t newId = m_nextId++;
    m_pathToId[filepath] = newId;
    return newId;
}

void ThumbnailDatabase::DrawThumbnail(ID2D1DeviceContext* context, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity) {
    if (!context) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. LRUキャッシュに存在する場合
    auto cacheIt = m_cache.find(thumbId);
    if (cacheIt != m_cache.end()) {
        // リストの先頭（最近使われた）に移動させる
        auto listIt = std::find(m_lruList.begin(), m_lruList.end(), thumbId);
        if (listIt != m_lruList.end()) {
            m_lruList.erase(listIt);
        }
        m_lruList.push_front(thumbId);

        // 指定された矩形へ描画する
        context->DrawBitmap(cacheIt->second.Get(), &destRect, opacity, D2D1_INTERPOLATION_MODE_LINEAR, nullptr);
        return;
    }

    // 2. キャッシュには無いが、idx（セクタ情報）には存在する場合
    bool existsInIdx = false; // TODO: idxに存在するか判定する処理
    if (existsInIdx) {
        // TODO: パックファイルからデコードしてLRUへ登録
        return;
    }

    // 3. まだ解析・クック中（どちらにも無い）場合
    // ガラス板（プレースホルダー）を描画
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
    if (SUCCEEDED(context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.1f), &brush))) {
        context->FillRectangle(&destRect, brush.Get());
    }
}
