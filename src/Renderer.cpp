#include "Renderer.h"
#include "ConfigManager.h"
#include "resource.h"
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
    int bgMode = m_config ? m_config->GetBackgroundArtMode() : 0;
    ID2D1Bitmap* artBitmap = nullptr;
    if (bgMode == 0) {
        artBitmap = m_currentArtBitmap ? m_currentArtBitmap.Get() : m_placeholderArtBitmap.Get();
    } else if (bgMode == 2) {
        artBitmap = m_placeholderArtBitmap.Get();
    }
    
    if (artBitmap && m_config) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        // 96DPIターゲットから得た物理サイズを論理サイズに変換
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;
        
        D2D1_SIZE_F bitmapSize = artBitmap->GetSize();
        
        float scaleX = renderTargetSize.width / bitmapSize.width;
        float scaleY = renderTargetSize.height / bitmapSize.height;
        float scale = (std::max)(scaleX, scaleY);
        
        float newWidth = renderTargetSize.width / scale;
        float newHeight = renderTargetSize.height / scale;
        float srcX = (bitmapSize.width - newWidth) / 2.0f;
        float srcY = (bitmapSize.height - newHeight) / 2.0f;
        
        D2D1_RECT_F srcRect = D2D1::RectF(srcX, srcY, srcX + newWidth, srcY + newHeight);
        D2D1_RECT_F destRect = D2D1::RectF(0.0f, 0.0f, renderTargetSize.width, renderTargetSize.height);
        
        m_d2dContext->DrawBitmap(
            artBitmap,
            &destRect,
            m_config->GetBgOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &srcRect
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
            D2D1_RECT_F bgRect = D2D1::RectF(0.0f, 0.0f, logicWidth, logicHeight);
            m_d2dContext->FillRectangle(&bgRect, darkenBrush.Get());
        }
    }

    // 2.5 7色ネオン心電図ビジュアライザの描画
    if (m_config && m_config->GetVisualizerMode() != 0 && !spectrum.empty()) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;
        
        D2D1_RECT_F drawRect = D2D1::RectF(0.0f, 0.0f, renderTargetSize.width, renderTargetSize.height);
        m_visualizer.Draw(m_d2dContext.Get(), spectrum, drawRect, m_trackTitle, m_trackArtist);
    }

    // 3. アイコンの描画

    ID2D1Bitmap* bitmapToDraw = isHovered ? m_appLogoHoverBitmap.Get() : m_appLogoBitmap.Get();
    if (m_config && m_config->GetShowAppLogo() && bitmapToDraw) {
        float x = static_cast<FLOAT>(m_config->GetLogoX());
        float y = static_cast<FLOAT>(m_config->GetLogoY());
        float w = static_cast<FLOAT>(m_config->GetLogoWidth());
        float h = static_cast<FLOAT>(m_config->GetLogoHeight());
        D2D1_RECT_F destRect = D2D1::RectF(x, y, x + w, y + h);
        
        if (m_shadowEffect && m_config->GetEnableShadow()) {
            m_shadowEffect->SetInput(0, bitmapToDraw);
            m_shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(0.0f, 0.0f, 0.0f, m_config->GetShadowOpacity()));
            D2D1_POINT_2F offset = D2D1::Point2F(x + m_config->GetShadowOffsetX(), y + m_config->GetShadowOffsetY());
            m_d2dContext->DrawImage(m_shadowEffect.Get(), &offset, nullptr, D2D1_INTERPOLATION_MODE_LINEAR, D2D1_COMPOSITE_MODE_SOURCE_OVER);
        }

        m_d2dContext->DrawBitmap(bitmapToDraw, &destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

    }


    // 4. 左下アルバムアートの描画
    if (m_config && m_config->GetShowNowPlaying()) {
        D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
        float logicHeight = rtSize.height / m_dpiScale;
        float size = static_cast<float>(m_config->GetArtSize());
        float x = static_cast<float>(m_config->GetBaseX() + m_config->GetArtOffsetX());
        float y = logicHeight - static_cast<float>(m_config->GetBaseBottomOffset()) + static_cast<float>(m_config->GetArtOffsetY());

        if (m_currentArtBitmap) {
            D2D1_SIZE_F bitmapSize = m_currentArtBitmap->GetSize();
            float scaleX = size / bitmapSize.width;
            float scaleY = size / bitmapSize.height;
            float scale = (std::min)(scaleX, scaleY);
            
            float drawWidth = bitmapSize.width * scale;
            float drawHeight = bitmapSize.height * scale;
            
            float drawX = x + (size - drawWidth) / 2.0f;
            float drawY = y + (size - drawHeight) / 2.0f;
            
            if (m_shadowBrush && m_config->GetEnableShadow()) {
                m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                D2D1_RECT_F shadowRect = D2D1::RectF(
                    drawX + m_config->GetShadowOffsetX(),
                    drawY + m_config->GetShadowOffsetY(),
                    drawX + drawWidth + m_config->GetShadowOffsetX(),
                    drawY + drawHeight + m_config->GetShadowOffsetY()
                );
                m_d2dContext->FillRectangle(&shadowRect, m_shadowBrush.Get());
            }

            D2D1_RECT_F destRectArt = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);

            m_d2dContext->DrawBitmap(

                m_currentArtBitmap.Get(),
                &destRectArt,
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
                D2D1_RECT_F destRectArt = D2D1::RectF(x, y, x + size, y + size);
                m_d2dContext->FillRectangle(&destRectArt, blackBrush.Get());
            }
        }
    }

    // 5. 曲情報テキストの描画
    if (m_config && m_config->GetShowNowPlaying() && m_textBrush && m_titleTextFormat && m_artistTextFormat) {
        D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
        float logicWidth = rtSize.width / m_dpiScale;
        float logicHeight = rtSize.height / m_dpiScale;
        float baseX = static_cast<float>(m_config->GetBaseX());
        float baseY = logicHeight - static_cast<float>(m_config->GetBaseBottomOffset());
        float rightMargin = 30.0f; // 右端の最低限のマージン

        // 曲名描画
        float titleX = baseX + static_cast<float>(m_config->GetTitleOffsetX());
        float titleY = baseY + static_cast<float>(m_config->GetTitleOffsetY());
        float titleRight = logicWidth - rightMargin;
        if (titleRight < titleX) titleRight = titleX + 1.0f; // 最小幅を確保
        
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            D2D1_RECT_F titleShadowRect = D2D1::RectF(
                titleX + m_config->GetShadowOffsetX(),
                titleY + m_config->GetShadowOffsetY(),
                titleRight + m_config->GetShadowOffsetX(),
                titleY + m_config->GetShadowOffsetY() + 100.0f
            );
            m_d2dContext->DrawText(
                m_trackTitle.c_str(),
                static_cast<UINT32>(m_trackTitle.length()),
                m_titleTextFormat.Get(),
                &titleShadowRect,
                m_shadowBrush.Get()
            );
        }

        D2D1_RECT_F titleRect = D2D1::RectF(titleX, titleY, titleRight, titleY + 100.0f);

        m_d2dContext->DrawText(
            m_trackTitle.c_str(),
            static_cast<UINT32>(m_trackTitle.length()),
            m_titleTextFormat.Get(),
            &titleRect,
            m_textBrush.Get()
        );

        // アーティスト名描画
        float artistX = baseX + static_cast<float>(m_config->GetArtistOffsetX());
        float artistY = baseY + static_cast<float>(m_config->GetArtistOffsetY());
        float artistRight = logicWidth - rightMargin;
        if (artistRight < artistX) artistRight = artistX + 1.0f;
        
        if (m_shadowBrush && m_config->GetEnableShadow()) {
            m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
            D2D1_RECT_F artistShadowRect = D2D1::RectF(
                artistX + m_config->GetShadowOffsetX(),
                artistY + m_config->GetShadowOffsetY(),
                artistRight + m_config->GetShadowOffsetX(),
                artistY + m_config->GetShadowOffsetY() + 50.0f
            );
            m_d2dContext->DrawText(
                m_trackArtist.c_str(),

                static_cast<UINT32>(m_trackArtist.length()),
                m_artistTextFormat.Get(),
                &artistShadowRect,
                m_shadowBrush.Get()
            );
        }

        D2D1_RECT_F artistRect = D2D1::RectF(artistX, artistY, artistRight, artistY + 50.0f);
        m_d2dContext->DrawText(
            m_trackArtist.c_str(),
            static_cast<UINT32>(m_trackArtist.length()),
            m_artistTextFormat.Get(),
            &artistRect,
            m_textBrush.Get()
        );

    }

    // 6. シークバーと時間テキストの描画
    if (m_config && m_config->GetShowSeekBar() && m_textBrush && m_timeTextFormat) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;

        float totalWidth = renderTargetSize.width * m_config->GetSeekBarWidthRatio();
        float startX = (renderTargetSize.width - totalWidth) / 2.0f;
        float barAreaWidth = totalWidth - static_cast<float>(m_config->GetSeekBarTimeAreaWidth());
        float y = renderTargetSize.height - static_cast<float>(m_config->GetSeekBarBottomOffset());
        float h = static_cast<float>(m_config->GetSeekBarHeight());

        float dimFactor = 1.0f - (m_controlAlpha * 0.5f);

        // シークバーの背景 (BgOpacity)
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, m_config->GetSeekBarBgOpacity() * dimFactor), &bgBrush);
        if (bgBrush) {
            D2D1_RECT_F bgRect = D2D1::RectF(startX, y, startX + barAreaWidth, y + h);
            m_d2dContext->FillRectangle(&bgRect, bgBrush.Get());
        }

        // シークバーの現在位置および時間テキストのブラシ
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fgBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, dimFactor), &fgBrush);

        if (fgBrush) {
            D2D1_RECT_F fgRect = D2D1::RectF(startX, y, startX + barAreaWidth * progress, y + h);
            m_d2dContext->FillRectangle(&fgRect, fgBrush.Get());
        }

        // 時間テキストの描画
        Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
        HRESULT hrLayout = m_dwriteFactory->CreateTextLayout(
            timeString.c_str(),
            static_cast<UINT32>(timeString.length()),
            m_timeTextFormat.Get(),
            totalWidth - barAreaWidth,
            h,
            &textLayout
        );

        if (SUCCEEDED(hrLayout)) {
            Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
            if (SUCCEEDED(textLayout.As(&textLayout1))) {
                DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(timeString.length())};
                textLayout1->SetCharacterSpacing(0.0f, m_config->GetSeekBarTimeLetterSpacing(), 0.0f, textRange);
            }

            D2D1_POINT_2F origin = D2D1::Point2F(startX + barAreaWidth, y);
            m_d2dContext->DrawTextLayout(
                origin,
                textLayout.Get(),
                fgBrush ? fgBrush.Get() : m_textBrush.Get(),
                D2D1_DRAW_TEXT_OPTIONS_NONE
            );
        }
    }

    // 7. 「次の曲」表示の描画
    if (m_config && m_config->GetShowNextTrack() && m_config->GetEnableNextTrack() && m_textBrush && m_nextTitleTextFormat && m_nextArtistTextFormat) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;

        float baseX = renderTargetSize.width - static_cast<float>(m_config->GetNextBaseRightOffset());
        float baseY = renderTargetSize.height - static_cast<float>(m_config->GetNextBaseBottomOffset());

        float artSize = static_cast<float>(m_config->GetNextArtSize());
        float artX = baseX + static_cast<float>(m_config->GetNextArtOffsetX());
        float artY = baseY + static_cast<float>(m_config->GetNextArtOffsetY());

        if (m_nextLabelTextFormat) {
            float labelX = baseX + static_cast<float>(m_config->GetNextLabelOffsetX());
            float labelY = baseY + static_cast<float>(m_config->GetNextLabelOffsetY());
            std::wstring labelText = L"Next Track";
            
            if (m_shadowBrush && m_config->GetEnableShadow()) {
                m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                D2D1_RECT_F labelShadowRect = D2D1::RectF(
                    labelX + m_config->GetShadowOffsetX(),
                    labelY + m_config->GetShadowOffsetY(),
                    labelX + m_config->GetShadowOffsetX() + 200.0f,
                    labelY + m_config->GetShadowOffsetY() + 30.0f
                );
                m_d2dContext->DrawText(
                    labelText.c_str(),

                    static_cast<UINT32>(labelText.length()),
                    m_nextLabelTextFormat.Get(),
                    &labelShadowRect,
                    m_shadowBrush.Get()
                );
            }

            D2D1_RECT_F labelRect = D2D1::RectF(labelX, labelY, labelX + 200.0f, labelY + 30.0f);
            m_d2dContext->DrawText(
                labelText.c_str(),
                static_cast<UINT32>(labelText.length()),
                m_nextLabelTextFormat.Get(),
                &labelRect,
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
                D2D1_RECT_F destRectArt = D2D1::RectF(artX, artY, artX + artSize, artY + artSize);
                m_d2dContext->FillRectangle(&destRectArt, blackBrush.Get());
            }

            float textX = baseX + static_cast<float>(m_config->GetNextTitleOffsetX());
            float textY = baseY + static_cast<float>(m_config->GetNextTitleOffsetY());
            std::wstring loadingText = L"Loading...";
            
            if (m_shadowBrush && m_config->GetEnableShadow()) {
                m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                D2D1_RECT_F textShadowRect = D2D1::RectF(
                    textX + m_config->GetShadowOffsetX(),
                    textY + m_config->GetShadowOffsetY(),
                    textX + m_config->GetShadowOffsetX() + 300.0f,
                    textY + m_config->GetShadowOffsetY() + 50.0f
                );
                m_d2dContext->DrawText(
                    loadingText.c_str(),

                    static_cast<UINT32>(loadingText.length()),
                    m_nextTitleTextFormat.Get(),
                    &textShadowRect,
                    m_shadowBrush.Get()
                );
            }

            D2D1_RECT_F textRect = D2D1::RectF(textX, textY, textX + 300.0f, textY + 50.0f);
            m_d2dContext->DrawText(
                loadingText.c_str(),
                static_cast<UINT32>(loadingText.length()),
                m_nextTitleTextFormat.Get(),
                &textRect,
                m_textBrush.Get()
            );

        } else {
            // ロード完了
            if (m_nextArtBitmap) {
                D2D1_SIZE_F bitmapSize = m_nextArtBitmap->GetSize();
                float scaleX = artSize / bitmapSize.width;
                float scaleY = artSize / bitmapSize.height;
                float scale = (std::min)(scaleX, scaleY);
                
                float drawWidth = bitmapSize.width * scale;
                float drawHeight = bitmapSize.height * scale;
                
                float drawX = artX + (artSize - drawWidth) / 2.0f;
                float drawY = artY + (artSize - drawHeight) / 2.0f;
                if (m_shadowBrush && m_config->GetEnableShadow()) {
                    m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                    D2D1_RECT_F shadowRect = D2D1::RectF(
                        drawX + m_config->GetShadowOffsetX(),
                        drawY + m_config->GetShadowOffsetY(),
                        drawX + drawWidth + m_config->GetShadowOffsetX(),
                        drawY + drawHeight + m_config->GetShadowOffsetY()
                    );
                    m_d2dContext->FillRectangle(&shadowRect, m_shadowBrush.Get());
                }

                D2D1_RECT_F destRectArt = D2D1::RectF(drawX, drawY, drawX + drawWidth, drawY + drawHeight);

                m_d2dContext->DrawBitmap(
                    m_nextArtBitmap.Get(),
                    &destRectArt,
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
                    D2D1_RECT_F destRectArt = D2D1::RectF(artX, artY, artX + artSize, artY + artSize);
                    m_d2dContext->FillRectangle(&destRectArt, blackBrush.Get());
                }
            }

            // Next曲名テキスト描画
            float titleX = baseX + static_cast<float>(m_config->GetNextTitleOffsetX());
            float titleY = baseY + static_cast<float>(m_config->GetNextTitleOffsetY());
            std::wstring nextText = m_nextTrackTitle;
            
            if (m_shadowBrush && m_config->GetEnableShadow()) {
                m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                D2D1_RECT_F titleShadowRect = D2D1::RectF(
                    titleX + m_config->GetShadowOffsetX(),
                    titleY + m_config->GetShadowOffsetY(),
                    titleX + m_config->GetShadowOffsetX() + 400.0f,
                    titleY + m_config->GetShadowOffsetY() + 50.0f
                );
                m_d2dContext->DrawText(
                    nextText.c_str(),

                    static_cast<UINT32>(nextText.length()),
                    m_nextTitleTextFormat.Get(),
                    &titleShadowRect,
                    m_shadowBrush.Get()
                );
            }

            D2D1_RECT_F titleRect = D2D1::RectF(titleX, titleY, titleX + 400.0f, titleY + 50.0f);
            m_d2dContext->DrawText(
                nextText.c_str(),

                static_cast<UINT32>(nextText.length()),
                m_nextTitleTextFormat.Get(),
                &titleRect,
                m_textBrush.Get()
            );

            // Nextアーティスト名テキスト描画
            float artistX = baseX + static_cast<float>(m_config->GetNextArtistOffsetX());
            float artistY = baseY + static_cast<float>(m_config->GetNextArtistOffsetY());
            
            if (m_shadowBrush && m_config->GetEnableShadow()) {
                m_shadowBrush->SetOpacity(m_config->GetShadowOpacity());
                D2D1_RECT_F artistShadowRect = D2D1::RectF(
                    artistX + m_config->GetShadowOffsetX(),
                    artistY + m_config->GetShadowOffsetY(),
                    artistX + m_config->GetShadowOffsetX() + 400.0f,
                    artistY + m_config->GetShadowOffsetY() + 50.0f
                );
                m_d2dContext->DrawText(
                    m_nextTrackArtist.c_str(),

                    static_cast<UINT32>(m_nextTrackArtist.length()),
                    m_nextArtistTextFormat.Get(),
                    &artistShadowRect,
                    m_shadowBrush.Get()
                );
            }

            D2D1_RECT_F artistRect = D2D1::RectF(artistX, artistY, artistX + 400.0f, artistY + 50.0f);
            m_d2dContext->DrawText(
                m_nextTrackArtist.c_str(),

                static_cast<UINT32>(m_nextTrackArtist.length()),
                m_nextArtistTextFormat.Get(),
                &artistRect,
                m_textBrush.Get()
            );
        }
    }

    // 8. 再生コントロールの描画
    if (isControlHovered) {
        m_controlAlpha += 0.05f;
        if (m_controlAlpha > 1.0f) m_controlAlpha = 1.0f;
    } else {
        m_controlAlpha -= 0.05f;
        if (m_controlAlpha < 0.0f) m_controlAlpha = 0.0f;
    }

    if (m_controlAlpha > 0.0f && m_config) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;

        float centerX = (renderTargetSize.width / 2.0f) + m_config->GetPlaybackCenterOffsetX();
        float centerY = renderTargetSize.height - m_config->GetPlaybackBaseBottomOffset();
        float size = static_cast<float>(m_config->GetPlaybackButtonSize());
        float spacing = static_cast<float>(m_config->GetPlaybackButtonSpacing());
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> controlBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, m_controlAlpha), &controlBrush);
        
        if (controlBrush) {
            float half = size / 2.0f;
            
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

            if (m_config->GetShowPlaybackControls()) {
                // Previous Button (⏮)
            float prevX = centerX - spacing;
            DrawRect(prevX - half + size*0.1f, centerY, size*0.2f, size);
            DrawTriangle(prevX - size*0.1f, centerY, size*0.4f, size, false);
            DrawTriangle(prevX + size*0.3f, centerY, size*0.4f, size, false);

            // Play/Pause Button
            if (isPlaying) {
                // Pause (⏸)
                DrawRect(centerX - size*0.2f, centerY, size*0.3f, size);
                DrawRect(centerX + size*0.2f, centerY, size*0.3f, size);
            } else {
                // Play (▶)
                DrawTriangle(centerX, centerY, size, size, true);
            }

            // Next Button (⏭)
            float nextX = centerX + spacing;
            DrawTriangle(nextX - size*0.3f, centerY, size*0.4f, size, true);
            DrawTriangle(nextX + size*0.1f, centerY, size*0.4f, size, true);
            DrawRect(nextX + half - size*0.1f, centerY, size*0.2f, size);
            }

            // 8.5 音量UIの描画
            if (m_config->GetShowVolumeControl()) {
                float volX = static_cast<float>(m_config->GetVolumeBaseLeftOffset());
            float volY = renderTargetSize.height - static_cast<float>(m_config->GetVolumeBaseBottomOffset());
            float volSize = static_cast<float>(m_config->GetVolumeIconSize());
            
            Microsoft::WRL::ComPtr<ID2D1PathGeometry> spkPath;
            m_d2dFactory->CreatePathGeometry(&spkPath);
            Microsoft::WRL::ComPtr<ID2D1GeometrySink> spkSink;
            spkPath->Open(&spkSink);
            spkSink->SetFillMode(D2D1_FILL_MODE_WINDING);
            
            float spkW = volSize * 0.35f;
            float spkH = volSize * 0.35f;
            float spkConeW = volSize * 0.45f;
            float spkConeH = volSize * 0.8f;
            
            // rect
            spkSink->BeginFigure(D2D1::Point2F(volX, volY - spkH/2), D2D1_FIGURE_BEGIN_FILLED);
            spkSink->AddLine(D2D1::Point2F(volX + spkW, volY - spkH/2));
            spkSink->AddLine(D2D1::Point2F(volX + spkW, volY + spkH/2));
            spkSink->AddLine(D2D1::Point2F(volX, volY + spkH/2));
            spkSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            
            // cone
            spkSink->BeginFigure(D2D1::Point2F(volX + spkW, volY - spkH/2), D2D1_FIGURE_BEGIN_FILLED);
            spkSink->AddLine(D2D1::Point2F(volX + spkW + spkConeW, volY - spkConeH/2));
            spkSink->AddLine(D2D1::Point2F(volX + spkW + spkConeW, volY + spkConeH/2));
            spkSink->AddLine(D2D1::Point2F(volX + spkW, volY + spkH/2));
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
                    100.0f,
                    volSize * 2.0f,
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
                float shadowX = m_config->GetVolumeShadowOffsetX();
                float shadowY = m_config->GetVolumeShadowOffsetY();
                
                D2D1::Matrix3x2F oldTransform;
                m_d2dContext->GetTransform(&oldTransform);
                m_d2dContext->SetTransform(
                    oldTransform * D2D1::Matrix3x2F::Translation(shadowX, shadowY)
                );
                
                m_d2dContext->FillGeometry(spkPath.Get(), m_shadowBrush.Get());
                
                if (volume > 0.0f) {
                    float arcX = volX + spkW + spkConeW + 4.0f;
                    m_d2dContext->DrawLine(D2D1::Point2F(arcX, volY - volSize*0.2f), D2D1::Point2F(arcX, volY + volSize*0.2f), m_shadowBrush.Get(), 2.0f);
                }
                if (volume > 0.5f) {
                    float arcX = volX + spkW + spkConeW + 8.0f;
                    m_d2dContext->DrawLine(D2D1::Point2F(arcX, volY - volSize*0.35f), D2D1::Point2F(arcX, volY + volSize*0.35f), m_shadowBrush.Get(), 2.0f);
                }
                
                m_d2dContext->SetTransform(oldTransform);

                if (volTextLayout) {
                    float textX = volX + static_cast<float>(m_config->GetVolumeTextOffsetX());
                    float textY = volY + static_cast<float>(m_config->GetVolumeTextOffsetY());
                    m_d2dContext->DrawTextLayout(
                        D2D1::Point2F(textX + shadowX, textY + shadowY),
                        volTextLayout.Get(),
                        m_shadowBrush.Get()
                    );
                }
            }

            m_d2dContext->FillGeometry(spkPath.Get(), controlBrush.Get());
            
            if (volume > 0.0f) {
                float arcX = volX + spkW + spkConeW + 4.0f;
                m_d2dContext->DrawLine(D2D1::Point2F(arcX, volY - volSize*0.2f), D2D1::Point2F(arcX, volY + volSize*0.2f), controlBrush.Get(), 2.0f);
            }
            if (volume > 0.5f) {
                float arcX = volX + spkW + spkConeW + 8.0f;
                m_d2dContext->DrawLine(D2D1::Point2F(arcX, volY - volSize*0.35f), D2D1::Point2F(arcX, volY + volSize*0.35f), controlBrush.Get(), 2.0f);
            }
            
            if (volTextLayout) {
                float textX = volX + static_cast<float>(m_config->GetVolumeTextOffsetX());
                float textY = volY + static_cast<float>(m_config->GetVolumeTextOffsetY());
                m_d2dContext->DrawTextLayout(
                    D2D1::Point2F(textX, textY),
                    volTextLayout.Get(),
                    controlBrush.Get()
                );
            }
            }
        }
    }
    // 9. プレイリスト TRACK XXX/XXX と スライドインUI
    if (m_config && m_trackCountTextFormat && m_textBrush) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;
        
        float trackCountRightOffset = static_cast<float>(m_config->GetTrackCountRightOffset());
        float trackCountBottomOffset = static_cast<float>(m_config->GetTrackCountBottomOffset());
        
        float trackCountX = renderTargetSize.width - trackCountRightOffset - 200.0f;
        float trackCountY = renderTargetSize.height - trackCountBottomOffset - 30.0f;
        
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
            200.0f,
            30.0f,
            &trackCountLayout
        );
        if (SUCCEEDED(hrLayout)) {
            Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
            if (SUCCEEDED(trackCountLayout.As(&textLayout1))) {
                DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(trackCountStr.length())};
                textLayout1->SetCharacterSpacing(0.0f, m_config->GetTrackCountLetterSpacing(), 0.0f, textRange);
            }
            
            D2D1_POINT_2F origin = D2D1::Point2F(trackCountX, trackCountY);
            
            if (m_shadowBrush && m_config->GetTrackCountShadowOpacity() > 0.0f) {
                m_shadowBrush->SetOpacity(m_config->GetTrackCountShadowOpacity());
                D2D1_POINT_2F shadowOrigin = D2D1::Point2F(trackCountX + m_config->GetTrackCountShadowOffsetX(), trackCountY + m_config->GetTrackCountShadowOffsetY());
                m_d2dContext->DrawTextLayout(shadowOrigin, trackCountLayout.Get(), m_shadowBrush.Get());
            }
            m_d2dContext->DrawTextLayout(origin, trackCountLayout.Get(), m_textBrush.Get());
        }

        // スライドインアニメーション
        float playlistWidth = static_cast<float>(m_config->GetPlaylistWidth());
        if (m_playlistSlideX > playlistWidth * 2.0f) m_playlistSlideX = playlistWidth;

        float targetSlideX = isPlaylistHovered ? 0.0f : playlistWidth;
        m_playlistSlideX += (targetSlideX - m_playlistSlideX) * 0.2f;

        if (!isPlaylistHovered) {
            m_playlistManualScrollY = 0.0f;
        }

        if (m_playlistSlideX < playlistWidth - 0.5f) {
            float playlistX = renderTargetSize.width - playlistWidth + m_playlistSlideX;
            float playlistY = 0.0f;
            float playlistHeight = renderTargetSize.height;

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
            m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, m_config->GetPlaylistBgOpacity()), &bgBrush);
            if (bgBrush) {
                D2D1_RECT_F bgRect = D2D1::RectF(playlistX, playlistY, playlistX + playlistWidth, playlistY + playlistHeight);
                m_d2dContext->FillRectangle(&bgRect, bgBrush.Get());
            }

            D2D1_RECT_F clipRect = D2D1::RectF(playlistX, playlistY, renderTargetSize.width, renderTargetSize.height);
            m_d2dContext->PushAxisAlignedClip(&clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            float itemHeight = static_cast<float>(m_config->GetPlaylistItemOffsetY());
            float baseScrollY = (playlistHeight / 2.0f) - (currentTrackIndex * itemHeight);
            float scrollY = baseScrollY + m_playlistManualScrollY;
            float maxScroll = 0.0f;
            float minScroll = playlistHeight - (totalTracks * itemHeight);
            if (minScroll > 0) minScroll = 0;
            scrollY = std::clamp(scrollY, minScroll, maxScroll);
            
            m_playlistManualScrollY = scrollY - baseScrollY;

            float currentY = scrollY;
            
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> highlightBrush;
            m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.2f), &highlightBrush);

            for (size_t i = 0; i < totalTracks && i < shuffleList.size(); ++i) {
                if (currentY + itemHeight > 0 && currentY < playlistHeight) {
                    if (i == currentTrackIndex && highlightBrush) {
                        D2D1_RECT_F hlRect = D2D1::RectF(playlistX, currentY, playlistX + playlistWidth, currentY + itemHeight);
                        m_d2dContext->FillRectangle(&hlRect, highlightBrush.Get());
                    }

                    std::wstring path = shuffleList[i];
                    std::wstring title;
                    try { title = std::filesystem::path(path).filename().wstring(); } catch(...) { title = L"Unknown"; }
                    std::wstring artist = L"Unknown Artist";
                    std::wstring timeStr = L"00:00";

                    float textX = playlistX + static_cast<float>(m_config->GetPlaylistTitleOffsetX());
                    float textY = currentY + static_cast<float>(m_config->GetPlaylistTitleOffsetY());
                    D2D1_RECT_F titleRect = D2D1::RectF(textX, textY, playlistX + playlistWidth - 10.0f, textY + 30.0f);
                    m_d2dContext->DrawText(title.c_str(), static_cast<UINT32>(title.length()), m_playlistTitleTextFormat.Get(), &titleRect, m_textBrush.Get());

                    float artistX = playlistX + static_cast<float>(m_config->GetPlaylistArtistOffsetX());
                    float artistY = currentY + static_cast<float>(m_config->GetPlaylistArtistOffsetY());
                    D2D1_RECT_F artistRect = D2D1::RectF(artistX, artistY, playlistX + playlistWidth - 100.0f, artistY + 20.0f);
                    m_d2dContext->DrawText(artist.c_str(), static_cast<UINT32>(artist.length()), m_playlistArtistTextFormat.Get(), &artistRect, m_playlistArtistBrush ? m_playlistArtistBrush.Get() : m_textBrush.Get());

                    float itemRightX = playlistX + playlistWidth;
                    float timeX = itemRightX - 100.0f - static_cast<float>(m_config->GetPlaylistTimeOffsetX());
                    float timeY = currentY + static_cast<float>(m_config->GetPlaylistTimeOffsetY());
                    
                    Microsoft::WRL::ComPtr<IDWriteTextLayout> timeLayout;
                    HRESULT hrTime = m_dwriteFactory->CreateTextLayout(
                        timeStr.c_str(),
                        static_cast<UINT32>(timeStr.length()),
                        m_playlistTimeTextFormat.Get(),
                        100.0f,
                        20.0f,
                        &timeLayout
                    );
                    if (SUCCEEDED(hrTime)) {
                        Microsoft::WRL::ComPtr<IDWriteTextLayout1> timeLayout1;
                        if (SUCCEEDED(timeLayout.As(&timeLayout1))) {
                            DWRITE_TEXT_RANGE textRange = {0, static_cast<UINT32>(timeStr.length())};
                            timeLayout1->SetCharacterSpacing(0.0f, m_config->GetPlaylistTimeLetterSpacing(), 0.0f, textRange);
                        }
                        m_d2dContext->DrawTextLayout(D2D1::Point2F(timeX, timeY), timeLayout.Get(), m_playlistTimeBrush ? m_playlistTimeBrush.Get() : m_textBrush.Get());
                    }
                }
                currentY += itemHeight;
            }

            m_d2dContext->PopAxisAlignedClip();
        }
    }

    if (m_config && m_config->GetEnableResize()) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> gripBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.5f), &gripBrush);
        if (gripBrush) {
            float size = 15.0f;
            Microsoft::WRL::ComPtr<ID2D1PathGeometry> gripPath;
            m_d2dFactory->CreatePathGeometry(&gripPath);
            Microsoft::WRL::ComPtr<ID2D1GeometrySink> gripSink;
            gripPath->Open(&gripSink);
            gripSink->SetFillMode(D2D1_FILL_MODE_WINDING);
            gripSink->BeginFigure(D2D1::Point2F(renderTargetSize.width - size, renderTargetSize.height), D2D1_FIGURE_BEGIN_FILLED);
            gripSink->AddLine(D2D1::Point2F(renderTargetSize.width, renderTargetSize.height));
            gripSink->AddLine(D2D1::Point2F(renderTargetSize.width, renderTargetSize.height - size));
            gripSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            gripSink->Close();
            m_d2dContext->FillGeometry(gripPath.Get(), gripBrush.Get());
        }
    }

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
