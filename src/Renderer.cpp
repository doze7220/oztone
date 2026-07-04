#include "Renderer.h"
#include "ConfigManager.h"
#include "resource.h"
#include "Visualizer.h"
#include "AudioPlayer.h"
#include "TagManager.h"
#include "LayoutCalculator.h"
#include <initguid.h>
#include <d2d1effects.h>
#pragma comment(lib, "dxguid.lib")
#include <algorithm>
#include <filesystem>


Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr), m_dpiScale(1.0f), m_controlAlpha(0.0f), m_playlistSlideX(9999.0f) {}

Renderer::~Renderer() {}

bool Renderer::Initialize(HWND hwnd, const ConfigManager& config) {
    m_hwnd = hwnd;
    m_config = &config;
    HRESULT hr = S_OK;

    // 1. D3D11 デバイスとデバイスコンテキストの作成
    // D2Dとの連携に必須のフラグ
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; 

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL supportedFeatureLevel;

    hr = D3D11CreateDevice(
        nullptr,                    // 既定のアダプターを使用
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_d3dDevice,
        &supportedFeatureLevel,
        &m_d3dContext
    );

    if (FAILED(hr)) return false;

    // 2. DXGIファクトリの取得とスワップチェインの作成
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    hr = m_d3dDevice.As(&dxgiDevice);
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
    hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) return false;

    RECT rect;
    GetClientRect(m_hwnd, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2; // ダブルバッファリング
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Windows 8以降推奨
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    swapChainDesc.Flags = 0;

    // PREMULTIPLIED AlphaMode はCreateSwapChainForHwndでは未サポートのため、
    // CreateSwapChainForComposition を使用し、DirectComposition経由でウィンドウに合成する
    hr = dxgiFactory->CreateSwapChainForComposition(
        m_d3dDevice.Get(),
        &swapChainDesc,
        nullptr,
        &m_swapChain
    );

    if (FAILED(hr)) return false;

    // DirectComposition デバイスを作成し、スワップチェインをウィンドウにバインドする
    hr = DCompositionCreateDevice(dxgiDevice.Get(), IID_PPV_ARGS(&m_dcompDevice));
    if (FAILED(hr)) return false;

    hr = m_dcompDevice->CreateTargetForHwnd(m_hwnd, TRUE, &m_dcompTarget);
    if (FAILED(hr)) return false;

    hr = m_dcompDevice->CreateVisual(&m_dcompVisual);
    if (FAILED(hr)) return false;

    hr = m_dcompVisual->SetContent(m_swapChain.Get());
    if (FAILED(hr)) return false;

    hr = m_dcompTarget->SetRoot(m_dcompVisual.Get());
    if (FAILED(hr)) return false;

    hr = m_dcompDevice->Commit();
    if (FAILED(hr)) return false;

    // 3. D2D1 ファクトリの作成
    D2D1_FACTORY_OPTIONS options = {};
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&m_d2dFactory);
    if (FAILED(hr)) return false;

    // 4. D2D1 デバイスとデバイスコンテキストの作成
    hr = m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);
    if (FAILED(hr)) return false;

    hr = m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dContext);
    if (FAILED(hr)) return false;

    // 5. DXGI サーフェスから D2D1 レンダーターゲットを作成
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
    if (FAILED(hr)) return false;

    UINT dpi = GetDpiForWindow(m_hwnd);
    m_dpiScale = static_cast<float>(dpi) / 96.0f;

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, // DPI X (内部は96DPI基準)
        96.0f  // DPI Y
    );

    hr = m_d2dContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, &m_d2dTargetBitmap);
    if (FAILED(hr)) return false;

    m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());

    // 6. WICファクトリの初期化と画像ロード
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory));
    if (FAILED(hr)) return false;

    if (!LoadBitmapResource(L"app_logo.png", IDI_APP_LOGO, &m_appLogoBitmap)) return false;
    if (!LoadBitmapResource(L"app_logo_hover.png", IDI_APP_LOGO_HOVER, &m_appLogoHoverBitmap)) return false;
    if (!LoadBitmapResource(L"placeholder_art.png", IDI_PLACEHOLDER_ART, &m_placeholderArtBitmap)) return false;

    // 7. DirectWrite ファクトリの作成とテキストフォーマット・ブラシの初期化
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_dwriteFactory.GetAddressOf())
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetTitleFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetTitleFontSize(),
        L"ja-jp",
        &m_titleTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetArtistFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetArtistFontSize(),
        L"ja-jp",
        &m_artistTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetNextLabelFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetNextLabelFontSize(),
        L"ja-jp",
        &m_nextLabelTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetNextTitleFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetNextTitleFontSize(),
        L"ja-jp",
        &m_nextTitleTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetNextArtistFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetNextArtistFontSize(),
        L"ja-jp",
        &m_nextArtistTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetSeekBarTimeFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetSeekBarTimeFontSize(),
        L"en-us",
        &m_timeTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetVolumeFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetVolumeFontSize(),
        L"en-us",
        &m_volumeTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetTrackCountFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetTrackCountFontSize(),
        L"en-us",
        &m_trackCountTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetPlaylistTitleFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetPlaylistTitleFontSize(),
        L"ja-jp",
        &m_playlistTitleTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetPlaylistArtistFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetPlaylistArtistFontSize(),
        L"ja-jp",
        &m_playlistArtistTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_dwriteFactory->CreateTextFormat(
        m_config->GetPlaylistTimeFontFamily().c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetPlaylistTimeFontSize(),
        L"en-us",
        &m_playlistTimeTextFormat
    );
    if (FAILED(hr)) return false;

    // 7.5 テキストトリミングの設定
    auto ApplyTrimming = [&](Microsoft::WRL::ComPtr<IDWriteTextFormat>& format) {
        if (!format) return;
        DWRITE_TRIMMING trimmingOptions = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
        Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
        HRESULT hrTrim = m_dwriteFactory->CreateEllipsisTrimmingSign(format.Get(), &ellipsis);
        if (SUCCEEDED(hrTrim)) {
            format->SetTrimming(&trimmingOptions, ellipsis.Get());
            format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        }
    };
    ApplyTrimming(m_titleTextFormat);
    ApplyTrimming(m_artistTextFormat);
    ApplyTrimming(m_timeTextFormat);
    ApplyTrimming(m_nextLabelTextFormat);
    ApplyTrimming(m_nextTitleTextFormat);
    ApplyTrimming(m_nextArtistTextFormat);
    ApplyTrimming(m_trackCountTextFormat);
    ApplyTrimming(m_playlistTitleTextFormat);
    ApplyTrimming(m_playlistArtistTextFormat);
    ApplyTrimming(m_playlistTimeTextFormat);

    hr = m_timeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (FAILED(hr)) return false;
    hr = m_timeTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (FAILED(hr)) return false;

    hr = m_trackCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (FAILED(hr)) return false;

    hr = m_playlistTimeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (FAILED(hr)) return false;

    // HEXから D2D1_COLOR_F への変換ラムダ
    auto ParseHexColor = [](const std::wstring& hexColor) -> D2D1_COLOR_F {
        if (hexColor.empty() || hexColor[0] != L'#') return D2D1::ColorF(D2D1::ColorF::White);
        try {
            unsigned int hexValue = std::stoul(hexColor.substr(1), nullptr, 16);
            if (hexColor.length() == 7) { // #RRGGBB
                return D2D1::ColorF(
                    ((hexValue >> 16) & 0xFF) / 255.0f,
                    ((hexValue >> 8) & 0xFF) / 255.0f,
                    (hexValue & 0xFF) / 255.0f
                );
            }
        } catch (...) {}
        return D2D1::ColorF(D2D1::ColorF::White);
    };

    hr = m_d2dContext->CreateSolidColorBrush(ParseHexColor(m_config->GetPlaylistArtistColor()), &m_playlistArtistBrush);
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateSolidColorBrush(ParseHexColor(m_config->GetPlaylistTimeColor()), &m_playlistTimeBrush);
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &m_textBrush
    );
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
        &m_shadowBrush
    );
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateEffect(CLSID_D2D1Shadow, &m_shadowEffect);
    if (FAILED(hr)) return false;

    D2D1_COLOR_F gripLineColor = ParseHexColor(m_config->GetPlaylistGripLineColor());
    hr = m_d2dContext->CreateSolidColorBrush(gripLineColor, &m_playlistGripLineBrush);
    if (FAILED(hr)) return false;

    D2D1_COLOR_F gripArrowColor = ParseHexColor(m_config->GetPlaylistGripArrowColor());
    hr = m_d2dContext->CreateSolidColorBrush(gripArrowColor, &m_playlistGripArrowBrush);
    if (FAILED(hr)) return false;

    hr = m_d2dFactory->CreatePathGeometry(&m_playlistGripArrowGeometry);
    if (SUCCEEDED(hr)) {
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        hr = m_playlistGripArrowGeometry->Open(&sink);
        if (SUCCEEDED(hr)) {
            float width = m_config->GetPlaylistGripArrowWidth();
            float height = m_config->GetPlaylistGripArrowHeight();
            
            sink->BeginFigure(D2D1::Point2F(0.0f, -height / 2.0f), D2D1_FIGURE_BEGIN_FILLED);
            sink->AddLine(D2D1::Point2F(-width, 0.0f));
            sink->AddLine(D2D1::Point2F(0.0f, height / 2.0f));
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();
        }
    }

    m_visualizer.SetConfig(m_config);

    return true;

}

bool Renderer::LoadBitmapResource(const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap) {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;

    // ① exeと同階層のファイルからロードを試みる
    hr = m_wicFactory->CreateDecoderFromFilename(
        filename.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if (FAILED(hr)) {
        // ② 失敗した場合は埋め込みリソースからロード
        HMODULE hModule = GetModuleHandle(nullptr);
        HRSRC imageResHandle = FindResource(hModule, MAKEINTRESOURCE(resourceId), RT_RCDATA);
        if (!imageResHandle) { return false; }

        HGLOBAL imageResDataHandle = LoadResource(hModule, imageResHandle);
        if (!imageResDataHandle) { return false; }

        void* pImageFile = LockResource(imageResDataHandle);
        DWORD imageFileSize = SizeofResource(hModule, imageResHandle);
        if (!pImageFile || imageFileSize == 0) { return false; }

        Microsoft::WRL::ComPtr<IWICStream> stream;
        hr = m_wicFactory->CreateStream(&stream);
        if (FAILED(hr)) { return false; }

        hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile), imageFileSize);
        if (FAILED(hr)) { return false; }

        hr = m_wicFactory->CreateDecoderFromStream(
            stream.Get(),
            nullptr,
            WICDecodeMetadataCacheOnLoad,
            &decoder
        );
        if (FAILED(hr)) { return false; }
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) { return false; }

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = m_wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) { return false; }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeMedianCut
    );
    if (FAILED(hr)) { return false; }

    hr = m_d2dContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, ppBitmap);
    return SUCCEEDED(hr);
}

void Renderer::SetTrackInfo(const std::wstring& title, const std::wstring& artist) {
    m_trackTitle = title;
    m_trackArtist = artist;
}

void Renderer::SetAlbumArt(ID2D1Bitmap* bitmap) {
    m_currentArtBitmap = bitmap;
}

void Renderer::SetNextTrackInfo(bool isReady, ID2D1Bitmap* art, const std::wstring& title, const std::wstring& artist) {
    m_nextIsReady = isReady;
    m_nextArtBitmap = art;
    m_nextTrackTitle = title;
    m_nextTrackArtist = artist;
}

bool Renderer::LoadBitmapFromMemory(const std::vector<uint8_t>& data, ID2D1Bitmap** ppBitmap) {
    if (data.empty() || !m_wicFactory || !m_d2dContext) return false;

    Microsoft::WRL::ComPtr<IWICStream> stream;
    HRESULT hr = m_wicFactory->CreateStream(&stream);
    if (FAILED(hr)) return false;

    hr = stream->InitializeFromMemory(const_cast<BYTE*>(data.data()), static_cast<DWORD>(data.size()));
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = m_wicFactory->CreateDecoderFromStream(
        stream.Get(),
        nullptr,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr)) {
        char buf[256];
        sprintf_s(buf, "LoadBitmapFromMemory: CreateDecoderFromStream failed with hr=0x%08X\n", hr);
        OutputDebugStringA(buf);
        return false;
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = m_wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return false;

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeMedianCut
    );
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, ppBitmap);
    if (FAILED(hr)) {
        OutputDebugStringA("LoadBitmapFromMemory: CreateBitmapFromWicBitmap failed\n");
        return false;
    }
    return true;
}

void Renderer::AddPlaylistScroll(float delta) {
    m_playlistManualScrollY += delta;
}

float Renderer::GetPlaylistManualScrollY() const {
    return m_playlistManualScrollY;
}

void Renderer::Render(bool isHovered, bool isControlHovered, bool isPlaylistHovered, bool isPlaying, float progress, const std::wstring& timeString, const std::vector<float>& spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<std::wstring>& shuffleList) {
    if (!m_d2dContext) return;

    m_d2dContext->BeginDraw();
    
    // DPIスケールを適用（論理ピクセルから物理ピクセルへの変換）
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
    
    // 1. 画面全体を黒でクリア
    m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
    
    // 2. 背景アルバムアートの描画 (Cover)
    DrawBackground();

    // 2.5 7色ネオン心電図ビジュアライザの描画
    DrawVisualizer(spectrum);

    // 3. アイコンの描画
    DrawAppLogo(isHovered);

    // 4. 左下アルバムアートの描画
    // 5. 曲情報テキストの描画
    DrawTrackInfo();

    // 6. シークバーと時間テキストの描画
    DrawSeekBar(progress, timeString);

    // 7. 「次の曲」表示の描画
    DrawNextTrack();

    // 8. 再生コントロールの描画
    if (isControlHovered) {
        m_controlAlpha += 0.05f;
        if (m_controlAlpha > 1.0f) m_controlAlpha = 1.0f;
    } else {
        m_controlAlpha -= 0.05f;
        if (m_controlAlpha < 0.0f) m_controlAlpha = 0.0f;
    }

    DrawPlaybackControls(isPlaying);
    DrawVolumeControl(volume);

    // 9. プレイリスト TRACK XXX/XXX と スライドインUI
    DrawPlaylist(isPlaylistHovered, currentTrackIndex, totalTracks, shuffleList);

    // 10. ウィンドウリサイズ用のグリップ（右下）
    DrawResizeGrip();

    HRESULT hr = m_d2dContext->EndDraw();

    if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) {
        // VSync同期でPresent (1)
        m_swapChain->Present(1, 0);
    }
}

void Renderer::Resize(UINT width, UINT height) {
    if (!m_d2dContext || !m_swapChain) return;

    m_d2dContext->SetTarget(nullptr);
    m_d2dTargetBitmap.Reset();

    HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr)) {
        Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
        if (SUCCEEDED(hr)) {
            D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                96.0f,
                96.0f
            );
            hr = m_d2dContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, &m_d2dTargetBitmap);
            if (SUCCEEDED(hr)) {
                m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());
            }
        }
    }
}

void Renderer::DrawBackground() {
    int bgMode = m_config ? m_config->GetBackgroundArtMode() : 0;
    ID2D1Bitmap* artBitmap = nullptr;
    if (bgMode == 0) {
        artBitmap = m_currentArtBitmap ? m_currentArtBitmap.Get() : m_placeholderArtBitmap.Get();
    } else if (bgMode == 2) {
        artBitmap = m_placeholderArtBitmap.Get();
    }
    
    if (artBitmap && m_config) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        float logicWidth = renderTargetSize.width / m_dpiScale;
        float logicHeight = renderTargetSize.height / m_dpiScale;
        
        BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, artBitmap->GetSize());
        
        m_d2dContext->DrawBitmap(
            artBitmap,
            &layout.destRect,
            m_config->GetBgOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &layout.srcRect
        );
    }

    if (m_config && m_config->GetBgDarkenOpacity() > 0.0f) {
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> darkenBrush;
        m_d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0.0f, 0.0f, 0.0f, m_config->GetBgDarkenOpacity()),
            &darkenBrush
        );
        if (darkenBrush) {
            D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
            float logicWidth = rtSize.width / m_dpiScale;
            float logicHeight = rtSize.height / m_dpiScale;
            
            BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, D2D1::SizeF(0.0f, 0.0f));
            m_d2dContext->FillRectangle(&layout.overlayRect, darkenBrush.Get());
        }
    }
}

void Renderer::DrawVisualizer(const std::vector<float>& spectrum) {
    if (m_config && m_config->GetVisualizerMode() != 0 && !spectrum.empty()) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        float logicWidth = renderTargetSize.width / m_dpiScale;
        float logicHeight = renderTargetSize.height / m_dpiScale;
        
        VisualizerLayout layout = LayoutCalculator::CalculateVisualizerLayout(logicWidth, logicHeight);
        m_visualizer.Draw(m_d2dContext.Get(), spectrum, layout.drawRect, m_trackTitle, m_trackArtist);
    }
}

void Renderer::DrawAppLogo(bool isHovered) {

ID2D1Bitmap* bitmapToDraw = isHovered ? m_appLogoHoverBitmap.Get() : m_appLogoBitmap.Get();
if (m_config && m_config->GetShowAppLogo() && bitmapToDraw) {
    AppLogoLayout layout = LayoutCalculator::CalculateAppLogoLayout(m_config);
    
    if (m_shadowEffect && m_config->GetEnableShadow()) {
        m_shadowEffect->SetInput(0, bitmapToDraw);
        m_shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(0.0f, 0.0f, 0.0f, m_config->GetShadowOpacity()));
        m_d2dContext->DrawImage(m_shadowEffect.Get(), &layout.shadowOffset, nullptr, D2D1_INTERPOLATION_MODE_LINEAR, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    }

    m_d2dContext->DrawBitmap(bitmapToDraw, &layout.destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

}


}

void Renderer::DrawTrackInfo() {
// 4. 左下アルバムアートの描画
if (m_config && m_config->GetShowNowPlaying()) {
    D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
    float logicWidth = rtSize.width / m_dpiScale;
    float logicHeight = rtSize.height / m_dpiScale;
    
    D2D1_SIZE_F bitmapSize = m_currentArtBitmap ? m_currentArtBitmap->GetSize() : D2D1::SizeF(0.0f, 0.0f);
    TrackInfoLayout layout = LayoutCalculator::CalculateTrackInfoLayout(logicWidth, logicHeight, m_config, bitmapSize);

    if (m_currentArtBitmap) {
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            m_d2dContext->FillRectangle(&layout.artShadowRect, m_shadowBrush.Get());
        }

        m_d2dContext->DrawBitmap(
            m_currentArtBitmap.Get(),
            &layout.artDestRect,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
    } else {
        // 正規の画像がない場合は黒い板を描画
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
        m_d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0.0f, 0.0f, 0.0f, m_config->GetFallbackArtOpacity()),
            &blackBrush
        );
        if (blackBrush) {
            m_d2dContext->FillRectangle(&layout.fallbackArtRect, blackBrush.Get());
        }
    }

// 5. 曲情報テキストの描画
if (m_config && m_config->GetShowNowPlaying() && m_textBrush && m_titleTextFormat && m_artistTextFormat) {
    // 曲名描画
    if (m_shadowBrush && m_config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
        m_d2dContext->DrawText(
            m_trackTitle.c_str(),
            static_cast<UINT32>(m_trackTitle.length()),
            m_titleTextFormat.Get(),
            &layout.titleShadowRect,
            m_shadowBrush.Get()
        );
    }

    m_d2dContext->DrawText(
        m_trackTitle.c_str(),
        static_cast<UINT32>(m_trackTitle.length()),
        m_titleTextFormat.Get(),
        &layout.titleRect,
        m_textBrush.Get()
    );

    // アーティスト名描画
    if (m_shadowBrush && m_config->GetEnableShadow()) {
        m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
        m_d2dContext->DrawText(
            m_trackArtist.c_str(),
            static_cast<UINT32>(m_trackArtist.length()),
            m_artistTextFormat.Get(),
            &layout.artistShadowRect,
            m_shadowBrush.Get()
        );
    }

    m_d2dContext->DrawText(
        m_trackArtist.c_str(),
        static_cast<UINT32>(m_trackArtist.length()),
        m_artistTextFormat.Get(),
        &layout.artistRect,
        m_textBrush.Get()
    );

}

}

}

void Renderer::DrawNextTrack() {
if (m_config && m_config->GetShowNextTrack() && m_config->GetEnableNextTrack() && m_textBrush && m_nextTitleTextFormat && m_nextArtistTextFormat) {
    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    D2D1_SIZE_F bitmapSize = m_nextArtBitmap ? m_nextArtBitmap->GetSize() : D2D1::SizeF(0.0f, 0.0f);
    NextTrackLayout layout = LayoutCalculator::CalculateNextTrackLayout(logicWidth, logicHeight, m_config, bitmapSize);

    if (m_nextLabelTextFormat) {
        std::wstring labelText = L"Next Track";
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            m_d2dContext->DrawText(
                labelText.c_str(),
                static_cast<UINT32>(labelText.length()),
                m_nextLabelTextFormat.Get(),
                &layout.labelShadowRect,
                m_shadowBrush.Get()
            );
        }

        m_d2dContext->DrawText(
            labelText.c_str(),
            static_cast<UINT32>(labelText.length()),
            m_nextLabelTextFormat.Get(),
            &layout.labelRect,
            m_textBrush.Get()
        );
    }

    if (!m_nextIsReady) {
        // ロード中
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
        m_d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0.0f, 0.0f, 0.0f, m_config->GetNextFallbackArtOpacity()),
            &blackBrush
        );
        if (blackBrush) {
            m_d2dContext->FillRectangle(&layout.fallbackArtRect, blackBrush.Get());
        }

        std::wstring loadingText = L"Loading...";
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            m_d2dContext->DrawText(
                loadingText.c_str(),
                static_cast<UINT32>(loadingText.length()),
                m_nextTitleTextFormat.Get(),
                &layout.titleShadowRect,
                m_shadowBrush.Get()
            );
        }

        m_d2dContext->DrawText(
            loadingText.c_str(),
            static_cast<UINT32>(loadingText.length()),
            m_nextTitleTextFormat.Get(),
            &layout.titleRect,
            m_textBrush.Get()
        );

    } else {
        // ロード完了
        if (m_nextArtBitmap) {
            if (m_shadowBrush && m_config->GetEnableShadow()) {
                m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                m_d2dContext->FillRectangle(&layout.artShadowRect, m_shadowBrush.Get());
            }

            m_d2dContext->DrawBitmap(
                m_nextArtBitmap.Get(),
                &layout.artDestRect,
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
            );
        } else {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
            m_d2dContext->CreateSolidColorBrush(
                D2D1::ColorF(0.0f, 0.0f, 0.0f, m_config->GetNextFallbackArtOpacity()),
                &blackBrush
            );
            if (blackBrush) {
                m_d2dContext->FillRectangle(&layout.fallbackArtRect, blackBrush.Get());
            }
        }

        // Next曲名テキスト描画
        std::wstring nextText = m_nextTrackTitle;
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            m_d2dContext->DrawText(
                nextText.c_str(),
                static_cast<UINT32>(nextText.length()),
                m_nextTitleTextFormat.Get(),
                &layout.titleShadowRect,
                m_shadowBrush.Get()
            );
        }

        m_d2dContext->DrawText(
            nextText.c_str(),
            static_cast<UINT32>(nextText.length()),
            m_nextTitleTextFormat.Get(),
            &layout.titleRect,
            m_textBrush.Get()
        );

        // Nextアーティスト名テキスト描画
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            m_d2dContext->DrawText(
                m_nextTrackArtist.c_str(),
                static_cast<UINT32>(m_nextTrackArtist.length()),
                m_nextArtistTextFormat.Get(),
                &layout.artistShadowRect,
                m_shadowBrush.Get()
            );
        }

        m_d2dContext->DrawText(
            m_nextTrackArtist.c_str(),
            static_cast<UINT32>(m_nextTrackArtist.length()),
            m_nextArtistTextFormat.Get(),
            &layout.artistRect,
            m_textBrush.Get()
        );
    }
}

}

void Renderer::DrawSeekBar(float progress, const std::wstring& timeString) {
if (m_config && m_config->GetShowSeekBar() && m_textBrush && m_timeTextFormat) {
    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    SeekBarLayout layout = LayoutCalculator::CalculateSeekBarLayout(logicWidth, logicHeight, m_config, progress);

    float dimFactor = 1.0f - (m_controlAlpha * 0.5f);

    // シークバーの背景 (BgOpacity)
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, m_config->GetSeekBarBgOpacity() * dimFactor), &bgBrush);
    if (bgBrush) {
        m_d2dContext->FillRectangle(&layout.bgRect, bgBrush.Get());
    }

    // シークバーの現在位置および時間テキストのブラシ
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fgBrush;
    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, dimFactor), &fgBrush);

    if (fgBrush) {
        m_d2dContext->FillRectangle(&layout.fgRect, fgBrush.Get());
    }

    // 時間テキストの描画
    Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
    HRESULT hrLayout = m_dwriteFactory->CreateTextLayout(
        timeString.c_str(),
        static_cast<UINT32>(timeString.length()),
        m_timeTextFormat.Get(),
        layout.textMaxWidth,
        layout.textMaxHeight,
        &textLayout
    );

    if (SUCCEEDED(hrLayout)) {
        Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
        if (SUCCEEDED(textLayout.As(&textLayout1))) {
            DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(timeString.length())};
            textLayout1->SetCharacterSpacing(0.0f, m_config->GetSeekBarTimeLetterSpacing(), 0.0f, textRange);
        }

        m_d2dContext->DrawTextLayout(
            layout.textOrigin,
            textLayout.Get(),
            fgBrush ? fgBrush.Get() : m_textBrush.Get(),
            D2D1_DRAW_TEXT_OPTIONS_NONE
        );
    }
}

}

void Renderer::DrawPlaybackControls(bool isPlaying) {
    if (m_controlAlpha <= 0.0f || !m_config || !m_config->GetShowPlaybackControls()) return;

    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    PlaybackControlsLayout layout = LayoutCalculator::CalculatePlaybackControlsLayout(logicWidth, logicHeight, m_config);
    
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> controlBrush;
    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, m_controlAlpha), &controlBrush);
    
    if (controlBrush) {
        auto DrawTriangle = [&](float cx, float cy, float w, float h, bool right) {
            Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
            m_d2dFactory->CreatePathGeometry(&path);
            Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
            path->Open(&sink);
            sink->SetFillMode(D2D1_FILL_MODE_WINDING);
            if (right) {
                sink->BeginFigure(D2D1::Point2F(cx - w/2, cy - h/2), D2D1_FIGURE_BEGIN_FILLED);
                sink->AddLine(D2D1::Point2F(cx + w/2, cy));
                sink->AddLine(D2D1::Point2F(cx - w/2, cy + h/2));
            } else {
                sink->BeginFigure(D2D1::Point2F(cx + w/2, cy - h/2), D2D1_FIGURE_BEGIN_FILLED);
                sink->AddLine(D2D1::Point2F(cx - w/2, cy));
                sink->AddLine(D2D1::Point2F(cx + w/2, cy + h/2));
            }
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();
            m_d2dContext->FillGeometry(path.Get(), controlBrush.Get());
        };

        auto DrawRect = [&](float cx, float cy, float w, float h) {
            D2D1_RECT_F rect = D2D1::RectF(cx - w/2, cy - h/2, cx + w/2, cy + h/2);
            m_d2dContext->FillRectangle(rect, controlBrush.Get());
        };

        // Previous Button (⏮)
        float prevX = layout.centerX - layout.spacing;
        DrawRect(prevX - layout.half + layout.size*0.1f, layout.centerY, layout.size*0.2f, layout.size);
        DrawTriangle(prevX - layout.size*0.1f, layout.centerY, layout.size*0.4f, layout.size, false);
        DrawTriangle(prevX + layout.size*0.3f, layout.centerY, layout.size*0.4f, layout.size, false);

        // Play/Pause Button
        if (isPlaying) {
            // Pause (⏸)
            DrawRect(layout.centerX - layout.size*0.2f, layout.centerY, layout.size*0.3f, layout.size);
            DrawRect(layout.centerX + layout.size*0.2f, layout.centerY, layout.size*0.3f, layout.size);
        } else {
            // Play (▶)
            DrawTriangle(layout.centerX, layout.centerY, layout.size, layout.size, true);
        }

        // Next Button (⏭)
        float nextX = layout.centerX + layout.spacing;
        DrawTriangle(nextX - layout.size*0.3f, layout.centerY, layout.size*0.4f, layout.size, true);
        DrawTriangle(nextX + layout.size*0.1f, layout.centerY, layout.size*0.4f, layout.size, true);
        DrawRect(nextX + layout.half - layout.size*0.1f, layout.centerY, layout.size*0.2f, layout.size);
    }
}

void Renderer::DrawVolumeControl(float volume) {
    if (m_controlAlpha <= 0.0f || !m_config || !m_config->GetShowVolumeControl()) return;

    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    VolumeControlLayout layout = LayoutCalculator::CalculateVolumeControlLayout(logicWidth, logicHeight, m_config);

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> controlBrush;
    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, m_controlAlpha), &controlBrush);

    if (controlBrush) {
        Microsoft::WRL::ComPtr<ID2D1PathGeometry> spkPath;
        m_d2dFactory->CreatePathGeometry(&spkPath);
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> spkSink;
        spkPath->Open(&spkSink);
        spkSink->SetFillMode(D2D1_FILL_MODE_WINDING);
        
        // rect
        spkSink->BeginFigure(D2D1::Point2F(layout.volX, layout.volY - layout.spkH/2), D2D1_FIGURE_BEGIN_FILLED);
        spkSink->AddLine(D2D1::Point2F(layout.volX + layout.spkW, layout.volY - layout.spkH/2));
        spkSink->AddLine(D2D1::Point2F(layout.volX + layout.spkW, layout.volY + layout.spkH/2));
        spkSink->AddLine(D2D1::Point2F(layout.volX, layout.volY + layout.spkH/2));
        spkSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        
        // cone
        spkSink->BeginFigure(D2D1::Point2F(layout.volX + layout.spkW, layout.volY - layout.spkH/2), D2D1_FIGURE_BEGIN_FILLED);
        spkSink->AddLine(D2D1::Point2F(layout.volX + layout.spkW + layout.spkConeW, layout.volY - layout.spkConeH/2));
        spkSink->AddLine(D2D1::Point2F(layout.volX + layout.spkW + layout.spkConeW, layout.volY + layout.spkConeH/2));
        spkSink->AddLine(D2D1::Point2F(layout.volX + layout.spkW, layout.volY + layout.spkH/2));
        spkSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        
        spkSink->Close();

        int volPercent = static_cast<int>(volume * 100.0f + 0.5f);
        wchar_t volBuf[16];
        swprintf_s(volBuf, L"%d%%", volPercent);
        
        Microsoft::WRL::ComPtr<IDWriteTextLayout> volTextLayout;
        if (m_volumeTextFormat) {
            float letterSpacing = m_config->GetVolumeTextLetterSpacing();
            HRESULT hr = m_dwriteFactory->CreateTextLayout(
                volBuf,
                static_cast<UINT32>(wcslen(volBuf)),
                m_volumeTextFormat.Get(),
                layout.textMaxWidth,
                layout.textMaxHeight,
                &volTextLayout
            );
            if (SUCCEEDED(hr) && letterSpacing != 0.0f) {
                Microsoft::WRL::ComPtr<IDWriteTextLayout1> volTextLayout1;
                if (SUCCEEDED(volTextLayout.As(&volTextLayout1))) {
                    DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(wcslen(volBuf))};
                    volTextLayout1->SetCharacterSpacing(0.0f, letterSpacing, 0.0f, textRange);
                }
            }
        }

        if (m_shadowBrush && m_config->GetVolumeEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetVolumeShadowOpacity() * m_controlAlpha);
            
            D2D1::Matrix3x2F oldTransform;
            m_d2dContext->GetTransform(&oldTransform);
            m_d2dContext->SetTransform(
                oldTransform * D2D1::Matrix3x2F::Translation(layout.shadowX, layout.shadowY)
            );
            
            m_d2dContext->FillGeometry(spkPath.Get(), m_shadowBrush.Get());
            
            if (volume > 0.0f) {
                float arcX = layout.volX + layout.spkW + layout.spkConeW + 4.0f;
                m_d2dContext->DrawLine(D2D1::Point2F(arcX, layout.volY - layout.volSize*0.2f), D2D1::Point2F(arcX, layout.volY + layout.volSize*0.2f), m_shadowBrush.Get(), 2.0f);
            }
            if (volume > 0.5f) {
                float arcX = layout.volX + layout.spkW + layout.spkConeW + 8.0f;
                m_d2dContext->DrawLine(D2D1::Point2F(arcX, layout.volY - layout.volSize*0.35f), D2D1::Point2F(arcX, layout.volY + layout.volSize*0.35f), m_shadowBrush.Get(), 2.0f);
            }
            
            m_d2dContext->SetTransform(oldTransform);

            if (volTextLayout) {
                m_d2dContext->DrawTextLayout(
                    D2D1::Point2F(layout.textX + layout.shadowX, layout.textY + layout.shadowY),
                    volTextLayout.Get(),
                    m_shadowBrush.Get()
                );
            }
        }

        m_d2dContext->FillGeometry(spkPath.Get(), controlBrush.Get());
        
        if (volume > 0.0f) {
            float arcX = layout.volX + layout.spkW + layout.spkConeW + 4.0f;
            m_d2dContext->DrawLine(D2D1::Point2F(arcX, layout.volY - layout.volSize*0.2f), D2D1::Point2F(arcX, layout.volY + layout.volSize*0.2f), controlBrush.Get(), 2.0f);
        }
        if (volume > 0.5f) {
            float arcX = layout.volX + layout.spkW + layout.spkConeW + 8.0f;
            m_d2dContext->DrawLine(D2D1::Point2F(arcX, layout.volY - layout.volSize*0.35f), D2D1::Point2F(arcX, layout.volY + layout.volSize*0.35f), controlBrush.Get(), 2.0f);
        }
        
        if (volTextLayout) {
            m_d2dContext->DrawTextLayout(
                D2D1::Point2F(layout.textX, layout.textY),
                volTextLayout.Get(),
                controlBrush.Get()
            );
        }
    }
}

void Renderer::DrawPlaylist(bool isPlaylistHovered, size_t currentTrackIndex, size_t totalTracks, const std::vector<std::wstring>& shuffleList) {
if (m_config && m_trackCountTextFormat && m_textBrush) {
    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;

    // スライドインアニメーション
    float configPlaylistWidth = static_cast<float>(m_config->GetPlaylistWidth());
    if (m_playlistSlideX > configPlaylistWidth * 2.0f) m_playlistSlideX = configPlaylistWidth;

    float targetSlideX = isPlaylistHovered ? 0.0f : configPlaylistWidth;
    m_playlistSlideX += (targetSlideX - m_playlistSlideX) * 0.2f;

    if (!isPlaylistHovered) {
        m_playlistManualScrollY = 0.0f;
    }

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicWidth, logicHeight, m_config, m_playlistSlideX, m_playlistManualScrollY, currentTrackIndex, totalTracks);
    
    m_playlistManualScrollY = layout.newManualScrollY;

    wchar_t trackCountBuf[64];
    if (totalTracks == 0) {
        swprintf_s(trackCountBuf, L"TRACK ---/---");
    } else {
        swprintf_s(trackCountBuf, L"TRACK %zu/%zu", currentTrackIndex + 1, totalTracks);
    }
    std::wstring trackCountStr(trackCountBuf);
    
    Microsoft::WRL::ComPtr<IDWriteTextLayout> trackCountLayout;
    HRESULT hrLayout = m_dwriteFactory->CreateTextLayout(
        trackCountStr.c_str(),
        static_cast<UINT32>(trackCountStr.length()),
        m_trackCountTextFormat.Get(),
        layout.trackCountMaxWidth,
        layout.trackCountMaxHeight,
        &trackCountLayout
    );
    if (SUCCEEDED(hrLayout)) {
        Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
        if (SUCCEEDED(trackCountLayout.As(&textLayout1))) {
            DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(trackCountStr.length())};
            textLayout1->SetCharacterSpacing(0.0f, m_config->GetTrackCountLetterSpacing(), 0.0f, textRange);
        }
        
        if (m_shadowBrush && m_config->GetTrackCountShadowOpacity() > 0.0f) {
            m_shadowBrush->SetOpacity(m_config->GetTrackCountShadowOpacity());
            m_d2dContext->DrawTextLayout(layout.trackCountShadowOrigin, trackCountLayout.Get(), m_shadowBrush.Get());
        }
        m_d2dContext->DrawTextLayout(layout.trackCountOrigin, trackCountLayout.Get(), m_textBrush.Get());
    }

    if (m_playlistGripLineBrush && m_playlistGripArrowBrush && m_playlistGripArrowGeometry) {
        if (m_shadowBrush && m_config->GetPlaylistGripShadowOpacity() > 0.0f) {
            m_shadowBrush->SetOpacity(m_config->GetPlaylistGripShadowOpacity());
            
            m_d2dContext->DrawLine(D2D1::Point2F(layout.gripShadowX, layout.playlistY), D2D1::Point2F(layout.gripShadowX, layout.playlistY + layout.playlistHeight), m_shadowBrush.Get(), layout.gripLineWidth);
            
            D2D1_MATRIX_3X2_F shadowTransform = D2D1::Matrix3x2F::Translation(layout.gripShadowX, layout.gripShadowY + layout.playlistHeight / 2.0f);
            m_d2dContext->SetTransform(shadowTransform * D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
            m_d2dContext->FillGeometry(m_playlistGripArrowGeometry.Get(), m_shadowBrush.Get());
            m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
        }

        m_d2dContext->DrawLine(D2D1::Point2F(layout.gripX, layout.playlistY), D2D1::Point2F(layout.gripX, layout.playlistY + layout.playlistHeight), m_playlistGripLineBrush.Get(), layout.gripLineWidth);

        D2D1_MATRIX_3X2_F arrowTransform = D2D1::Matrix3x2F::Translation(layout.gripX, layout.playlistY + layout.playlistHeight / 2.0f);
        m_d2dContext->SetTransform(arrowTransform * D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
        m_d2dContext->FillGeometry(m_playlistGripArrowGeometry.Get(), m_playlistGripArrowBrush.Get());
        m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
    }

    if (m_playlistSlideX < layout.playlistWidth - 0.5f) {

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, m_config->GetPlaylistBgOpacity()), &bgBrush);
        if (bgBrush) {
            m_d2dContext->FillRectangle(&layout.bgRect, bgBrush.Get());
        }

        m_d2dContext->PushAxisAlignedClip(&layout.clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        float currentY = layout.startY;
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> highlightBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.2f), &highlightBrush);

        for (size_t i = 0; i < totalTracks && i < shuffleList.size(); ++i) {
            if (currentY + layout.itemHeight > 0 && currentY < layout.playlistHeight) {
                PlaylistItemLayout itemLayout = LayoutCalculator::CalculatePlaylistItemLayout(layout, m_config, currentY);

                if (i == currentTrackIndex && highlightBrush) {
                    m_d2dContext->FillRectangle(&itemLayout.hlRect, highlightBrush.Get());
                }

                std::wstring path = shuffleList[i];
                std::wstring title;
                try { title = std::filesystem::path(path).filename().wstring(); } catch(...) { title = L"Unknown"; }
                std::wstring artist = L"Unknown Artist";
                std::wstring timeStr = L"00:00";

                m_d2dContext->DrawText(title.c_str(), static_cast<UINT32>(title.length()), m_playlistTitleTextFormat.Get(), &itemLayout.titleRect, m_textBrush.Get());

                m_d2dContext->DrawText(artist.c_str(), static_cast<UINT32>(artist.length()), m_playlistArtistTextFormat.Get(), &itemLayout.artistRect, m_playlistArtistBrush ? m_playlistArtistBrush.Get() : m_textBrush.Get());

                Microsoft::WRL::ComPtr<IDWriteTextLayout> timeLayout;
                HRESULT hrTime = m_dwriteFactory->CreateTextLayout(
                    timeStr.c_str(),
                    static_cast<UINT32>(timeStr.length()),
                    m_playlistTimeTextFormat.Get(),
                    itemLayout.timeMaxWidth,
                    itemLayout.timeMaxHeight,
                    &timeLayout
                );
                if (SUCCEEDED(hrTime)) {
                    Microsoft::WRL::ComPtr<IDWriteTextLayout1> timeLayout1;
                    if (SUCCEEDED(timeLayout.As(&timeLayout1))) {
                        DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(timeStr.length())};
                        timeLayout1->SetCharacterSpacing(0.0f, m_config->GetPlaylistTimeLetterSpacing(), 0.0f, textRange);
                    }
                    m_d2dContext->DrawTextLayout(itemLayout.timeOrigin, timeLayout.Get(), m_playlistTimeBrush ? m_playlistTimeBrush.Get() : m_textBrush.Get());
                }
            }
            currentY += layout.itemHeight;
        }

        m_d2dContext->PopAxisAlignedClip();
    }
}

}

void Renderer::DrawResizeGrip() {
if (m_config && m_config->GetEnableResize()) {
    D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    float logicWidth = renderTargetSize.width / m_dpiScale;
    float logicHeight = renderTargetSize.height / m_dpiScale;
    
    ResizeGripLayout layout = LayoutCalculator::CalculateResizeGripLayout(logicWidth, logicHeight);

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> gripBrush;
    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.5f), &gripBrush);
    if (gripBrush) {
        Microsoft::WRL::ComPtr<ID2D1PathGeometry> gripPath;
        m_d2dFactory->CreatePathGeometry(&gripPath);
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> gripSink;
        gripPath->Open(&gripSink);
        gripSink->SetFillMode(D2D1_FILL_MODE_WINDING);
        gripSink->BeginFigure(layout.pt1, D2D1_FIGURE_BEGIN_FILLED);
        gripSink->AddLine(layout.pt2);
        gripSink->AddLine(layout.pt3);
        gripSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        gripSink->Close();
        m_d2dContext->FillGeometry(gripPath.Get(), gripBrush.Get());
    }
}

}

