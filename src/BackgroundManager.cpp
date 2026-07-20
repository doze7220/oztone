#include "BackgroundManager.h"
#include "FileManager.h"
#include <objbase.h>
#include <vector>

BackgroundManager::BackgroundManager()
    : m_isRunning(false)
    , m_hasNewImage(false)
    , m_fadeProgress(1.0f)
{
}

BackgroundManager::~BackgroundManager()
{
    Uninitialize();
}

void BackgroundManager::Initialize()
{
    if (m_isRunning) {
        return;
    }

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
            m_oldWicImage = m_currentWicImage;
            m_currentWicImage = m_nextWicImage;
            m_hasNewImage = false;
            m_fadeProgress = 0.0f; // フェード開始
        }
    }

    if (m_fadeProgress < 1.0f) {
        // クロスフェード完了までの時間（秒）
        const float FADE_DURATION = 0.5f;
        m_fadeProgress += deltaTime / FADE_DURATION;
        if (m_fadeProgress >= 1.0f) {
            m_fadeProgress = 1.0f;
            // フェード完了時に古い画像を解放する
            m_oldWicImage.Reset();
        }
    }
}

void BackgroundManager::WorkerLoop()
{
    // WICデコード等のため、バックグラウンドスレッドでもCOMを初期化する
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    
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

    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
}
