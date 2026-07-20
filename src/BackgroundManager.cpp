#include "BackgroundManager.h"
#include "ConfigManager.h"
#include "FileManager.h"
#include <objbase.h>
#include <vector>

BackgroundManager::BackgroundManager()
    : m_isRunning(false)
    , m_hasNewImage(false)
    , m_fadeProgress(1.0f)
    , m_oldScale(1.0f)
    , m_oldOffsetX(0.0f)
    , m_oldOffsetY(0.0f)
    , m_currentScale(1.0f)
    , m_currentOffsetX(0.0f)
    , m_currentOffsetY(0.0f)
{
}

BackgroundManager::~BackgroundManager()
{
    Uninitialize();
}

void BackgroundManager::Initialize(const ConfigManager* config)
{
    if (m_isRunning) {
        return;
    }

    m_config = config;
    m_isRunning = true;
    m_workerThread = std::thread(&BackgroundManager::WorkerLoop, this);
}

void BackgroundManager::Uninitialize()
{
    if (!m_isRunning) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning = false;
    }
    // ワーカー・スレッドを起床させて終了を促す
    m_cv.notify_one();

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void BackgroundManager::RequestLoad(const std::wstring& filePath)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_requestQueue.push(filePath);
    }
    m_cv.notify_one();
}

void BackgroundManager::UpdateAnimation(float deltaTime)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_hasNewImage) {
            // 要件1: OLDへのスワップ処理やNEWのフレーミングリセットを禁止
            m_currentWicImage = m_nextWicImage;
            m_hasNewImage = false;
            m_fadeProgress = 0.0f; // フェード開始
        }
    }

    if (m_fadeProgress < 1.0f) {
        // クロスフェード完了までの時間（秒）を ConfigManager から取得
        float fadeDuration = 0.5f;
        if (m_config) {
            fadeDuration = m_config->GetCrossfadeDuration();
        }
        if (fadeDuration <= 0.0f) {
            m_fadeProgress = 1.0f;
        } else {
            m_fadeProgress += deltaTime / fadeDuration;
        }

        if (m_fadeProgress >= 1.0f) {
            m_fadeProgress = 1.0f;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            // 要件2: 1.0f 到達時に NEW を OLD へスワップ（格上げ）し、NEW を解放
            m_oldWicImage = m_currentWicImage;
            m_oldScale = m_currentScale;
            m_oldOffsetX = m_currentOffsetX;
            m_oldOffsetY = m_currentOffsetY;
            
            m_currentWicImage.Reset();
        }
    }
}

void BackgroundManager::SetArtFramingScale(float scale)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // 要件2: フェード中はユーザー操作を無効化
    if (m_currentWicImage && m_fadeProgress < 1.0f) {
        return;
    }
    // 平常時のみOLDを更新
    m_oldScale = scale;
}

void BackgroundManager::SetArtFramingScroll(float offsetX, float offsetY)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_currentWicImage && m_fadeProgress < 1.0f) {
        return;
    }
    m_oldOffsetX = offsetX;
    m_oldOffsetY = offsetY;
}

void BackgroundManager::SetBackgroundFraming(float scale, float offsetX, float offsetY)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // 要件1: ロード時の絶対値はNEWのみを更新する
    m_currentScale = scale;
    m_currentOffsetX = offsetX;
    m_currentOffsetY = offsetY;
}

void BackgroundManager::GetFraming(float& x, float& y, float& scale) const
{
    // 要件3: ゲッターのステート対応
    if (m_currentWicImage && m_fadeProgress < 1.0f) {
        x = m_currentOffsetX;
        y = m_currentOffsetY;
        scale = m_currentScale;
    } else {
        x = m_oldOffsetX;
        y = m_oldOffsetY;
        scale = m_oldScale;
    }
}

std::vector<BackgroundLayer> BackgroundManager::GetLayers() const {
    std::vector<BackgroundLayer> layers;
    if (!m_config) return layers;

    // 1. 下敷き
    BackgroundLayer baseLayer;
    baseLayer.type = BackgroundLayerType::SolidColor;
    baseLayer.color = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black
    baseLayer.opacity = 1.0f;
    layers.push_back(baseLayer);

    int bgMode = m_config->GetBackgroundArtMode();

    if (bgMode == 0) {
        // 2. OLD画像
        if (m_oldWicImage) {
            BackgroundLayer oldLayer;
            oldLayer.type = BackgroundLayerType::Image;
            oldLayer.image = m_oldWicImage;
            oldLayer.opacity = 1.0f;
            oldLayer.x = m_oldOffsetX;
            oldLayer.y = m_oldOffsetY;
            oldLayer.scale = m_oldScale;
            layers.push_back(oldLayer);
        }

        // 3. NEW画像
        if (m_currentWicImage) {
            BackgroundLayer newLayer;
            newLayer.type = BackgroundLayerType::Image;
            newLayer.image = m_currentWicImage;
            newLayer.opacity = m_fadeProgress;
            newLayer.x = m_currentOffsetX;
            newLayer.y = m_currentOffsetY;
            newLayer.scale = m_currentScale;
            layers.push_back(newLayer);
        }
    } else if (bgMode == 2) {
        // プレイバックモード：プレースホルダー画像を描画させる指示
        BackgroundLayer placeholderLayer;
        placeholderLayer.type = BackgroundLayerType::Image;
        placeholderLayer.image = nullptr;
        placeholderLayer.opacity = 1.0f;
        placeholderLayer.x = m_currentOffsetX;
        placeholderLayer.y = m_currentOffsetY;
        placeholderLayer.scale = m_currentScale;
        layers.push_back(placeholderLayer);
    }

    // 4. ポストエフェクト (カラーフィル)
    float darken = m_config->GetBgDarkenOpacity();
    if (darken > 0.0f) {
        BackgroundLayer overlay;
        overlay.type = BackgroundLayerType::SolidColor;
        overlay.color = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black
        overlay.opacity = darken;
        layers.push_back(overlay);
    }

    return layers;
}

void BackgroundManager::WorkerLoop()
{
    // WICデコード等のため、バックグラウンドスレッドでもCOMを初期化する
    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HRESULT hr = S_OK;
    
    while (true) {
        std::wstring filePath;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            
            m_cv.wait(lock, [this]() {
                // 終了フラグが立つか、新規ロードタスクが積まれるまで待機する
                return !m_isRunning || !m_requestQueue.empty(); 
            });

            if (!m_isRunning) {
                break;
            }

            // キューから最新のリクエストだけを取り出す（途中のものはスキップ）
            while (!m_requestQueue.empty()) {
                filePath = m_requestQueue.front();
                m_requestQueue.pop();
            }
        }

        // FileManagerを用いた非同期画像抽出とWICデコード処理
        std::vector<BYTE> binaryData = FileManager::ExtractAlbumArtBinary(filePath);
        
        Microsoft::WRL::ComPtr<IWICFormatConverter> decodedImage;

        if (!binaryData.empty()) {
            Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
            hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&factory)
            );

            if (SUCCEEDED(hr)) {
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, binaryData.size());
                if (hGlobal) {
                    void* pData = GlobalLock(hGlobal);
                    if (pData) {
                        memcpy(pData, binaryData.data(), binaryData.size());
                        GlobalUnlock(hGlobal);
                    }

                    Microsoft::WRL::ComPtr<IStream> stream;
                    hr = CreateStreamOnHGlobal(hGlobal, TRUE, &stream);
                    if (SUCCEEDED(hr)) {
                        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
                        hr = factory->CreateDecoderFromStream(
                            stream.Get(),
                            nullptr,
                            WICDecodeMetadataCacheOnDemand,
                            &decoder
                        );

                        if (SUCCEEDED(hr)) {
                            Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
                            hr = decoder->GetFrame(0, &frame);
                            if (SUCCEEDED(hr)) {
                                hr = factory->CreateFormatConverter(&decodedImage);
                                if (SUCCEEDED(hr)) {
                                    hr = decodedImage->Initialize(
                                        frame.Get(),
                                        GUID_WICPixelFormat32bppPBGRA,
                                        WICBitmapDitherTypeNone,
                                        nullptr,
                                        0.f,
                                        WICBitmapPaletteTypeMedianCut
                                    );
                                    if (FAILED(hr)) {
                                        decodedImage.Reset();
                                    }
                                }
                            }
                        }
                    } else {
                        GlobalFree(hGlobal);
                    }
                }
            }
        }

        // スレッドセーフにクラス内部へ保持する
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_nextWicImage = decodedImage;
            m_hasNewImage = true;
        }
    }

    if (SUCCEEDED(hrInit)) {
        CoUninitialize();
    }
}
