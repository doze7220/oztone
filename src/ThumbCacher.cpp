#include "ThumbCacher.h"
#include "ThumbnailDatabase.h"
#include "FileManager.h"
#include <wincodec.h>
#include <wrl/client.h>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;

ThumbCacher::ThumbCacher(ThumbnailDatabase* db)
    : m_db(db)
    , m_stopFlag(false)
{
}

ThumbCacher::~ThumbCacher()
{
    Uninitialize();
}

void ThumbCacher::Initialize()
{
    if (!m_workerThread.joinable())
    {
        m_stopFlag = false;
        m_workerThread = std::thread(&ThumbCacher::WorkerLoop, this);
    }
}

void ThumbCacher::Uninitialize()
{
    if (m_workerThread.joinable())
    {
        m_stopFlag = true;
        m_cv.notify_all();
        m_workerThread.join();
    }
}

void ThumbCacher::EnqueueTrack(uint32_t thumbId, const std::wstring& filepath)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskQueue.push({thumbId, filepath});
    }
    m_cv.notify_one();
}

void ThumbCacher::WorkerLoop()
{
    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    while (true)
    {
        std::pair<uint32_t, std::wstring> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() { return m_stopFlag || !m_taskQueue.empty(); });

            // 終了フラグが立ったら直ちにループを抜ける（キューに残りがあっても破棄して終了）
            if (m_stopFlag)
            {
                break;
            }

            task = m_taskQueue.front();
            m_taskQueue.pop();
        }

        uint32_t thumbId = task.first;
        std::wstring filepath = task.second;

        if (filepath.empty())
        {
            continue;
        }

        if (m_db->HasCookedData(thumbId))
        {
            continue;
        }

        try
        {
            std::vector<BYTE> rawBinary = FileManager::ExtractAlbumArtBinary(filepath);
            if (!rawBinary.empty())
            {
                const UINT THUMBNAIL_SIZE = 160;
                const float JPEG_QUALITY = 0.85f;
                std::vector<BYTE> cookedBinary = CookThumbnailImage(rawBinary, THUMBNAIL_SIZE, JPEG_QUALITY);
                if (!cookedBinary.empty())
                {
                    if (!m_db->StoreCookedData(thumbId, filepath, cookedBinary))
                    {
                        m_db->RollbackThumbId(filepath);
                    }
                }
                else
                {
                    m_db->RollbackThumbId(filepath);
                }
            }
            else
            {
                m_db->RollbackThumbId(filepath);
            }
        }
        catch (...)
        {
            m_db->RollbackThumbId(filepath);
        }
    }

    if (SUCCEEDED(hrInit))
    {
        CoUninitialize();
    }
}

std::vector<BYTE> ThumbCacher::CookThumbnailImage(const std::vector<BYTE>& rawBinary, UINT targetSize, float jpegQuality)
{
    if (rawBinary.empty() || targetSize == 0)
    {
        return {};
    }

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return {};

    ComPtr<IWICStream> stream;
    hr = factory->CreateStream(&stream);
    if (FAILED(hr)) return {};

    hr = stream->InitializeFromMemory(const_cast<BYTE*>(rawBinary.data()), static_cast<DWORD>(rawBinary.size()));
    if (FAILED(hr)) return {};

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return {};

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return {};

    UINT width = 0, height = 0;
    hr = frame->GetSize(&width, &height);
    if (FAILED(hr)) return {};

    UINT newWidth = width;
    UINT newHeight = height;
    if (width > 0 && height > 0)
    {
        if (width > height)
        {
            newWidth = targetSize;
            newHeight = static_cast<UINT>(std::round(targetSize * static_cast<float>(height) / width));
            if (newHeight == 0) newHeight = 1;
        }
        else
        {
            newHeight = targetSize;
            newWidth = static_cast<UINT>(std::round(targetSize * static_cast<float>(width) / height));
            if (newWidth == 0) newWidth = 1;
        }
    }

    ComPtr<IWICBitmapScaler> scaler;
    hr = factory->CreateBitmapScaler(&scaler);
    if (FAILED(hr)) return {};

    hr = scaler->Initialize(frame.Get(), newWidth, newHeight, WICBitmapInterpolationModeFant);
    if (FAILED(hr)) return {};

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return {};

    hr = converter->Initialize(
        scaler.Get(),
        GUID_WICPixelFormat24bppBGR,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) return {};

    ComPtr<IWICBitmapEncoder> encoder;
    hr = factory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &encoder);
    if (FAILED(hr)) return {};

    ComPtr<IStream> outStream;
    hr = CreateStreamOnHGlobal(NULL, TRUE, &outStream);
    if (FAILED(hr)) return {};

    hr = encoder->Initialize(outStream.Get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) return {};

    ComPtr<IWICBitmapFrameEncode> frameEncode;
    ComPtr<IPropertyBag2> propBag;
    hr = encoder->CreateNewFrame(&frameEncode, &propBag);
    if (FAILED(hr)) return {};

    PROPBAG2 option = { 0 };
    option.pstrName = const_cast<LPOLESTR>(L"ImageQuality");
    VARIANT varValue;
    VariantInit(&varValue);
    varValue.vt = VT_R4;
    varValue.fltVal = (std::max)(0.0f, (std::min)(1.0f, jpegQuality));

    hr = propBag->Write(1, &option, &varValue);

    hr = frameEncode->Initialize(propBag.Get());
    if (FAILED(hr)) return {};

    hr = frameEncode->SetSize(newWidth, newHeight);
    if (FAILED(hr)) return {};

    WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;
    hr = frameEncode->SetPixelFormat(&format);
    if (FAILED(hr)) return {};

    hr = frameEncode->WriteSource(converter.Get(), nullptr);
    if (FAILED(hr)) return {};

    hr = frameEncode->Commit();
    if (FAILED(hr)) return {};

    hr = encoder->Commit();
    if (FAILED(hr)) return {};

    STATSTG statstg = { 0 };
    hr = outStream->Stat(&statstg, STATFLAG_NONAME);
    if (FAILED(hr)) return {};

    std::vector<BYTE> resultData(statstg.cbSize.LowPart);
    LARGE_INTEGER liZero = { 0 };
    hr = outStream->Seek(liZero, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) return {};

    ULONG bytesRead = 0;
    hr = outStream->Read(resultData.data(), statstg.cbSize.LowPart, &bytesRead);
    if (FAILED(hr) || bytesRead != statstg.cbSize.LowPart) return {};

    return resultData;
}
