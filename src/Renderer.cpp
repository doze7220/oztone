#include "Renderer.h"
#include "ConfigManager.h"
#include "resource.h"
#include <algorithm>

Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr) {}

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
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&m_d2dFactory);
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

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, // DPI X
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
        L"Meiryo",
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
        L"Meiryo",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_config->GetArtistFontSize(),
        L"ja-jp",
        &m_artistTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &m_textBrush
    );
    if (FAILED(hr)) return false;

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

void Renderer::Render(bool isHovered) {
    if (!m_d2dContext) return;

    m_d2dContext->BeginDraw();
    
    // 1. 画面全体を黒でクリア
    m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
    
    // 2. 背景アルバムアートの描画 (Cover)
    if (m_placeholderArtBitmap && m_config) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        D2D1_SIZE_F bitmapSize = m_placeholderArtBitmap->GetSize();
        
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
            m_placeholderArtBitmap.Get(),
            &destRect,
            m_config->GetBgOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &srcRect
        );
    }

    // 3. アイコンの描画
    ID2D1Bitmap* bitmapToDraw = isHovered ? m_appLogoHoverBitmap.Get() : m_appLogoBitmap.Get();
    if (bitmapToDraw && m_config) {
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<FLOAT>(m_config->GetLogoX()),
            static_cast<FLOAT>(m_config->GetLogoY()),
            static_cast<FLOAT>(m_config->GetLogoX() + m_config->GetLogoWidth()),
            static_cast<FLOAT>(m_config->GetLogoY() + m_config->GetLogoHeight())
        );
        m_d2dContext->DrawBitmap(bitmapToDraw, &destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    }

    // 4. 左下アルバムアートの描画
    if (m_placeholderArtBitmap && m_config) {
        float size = static_cast<float>(m_config->GetArtSize());
        float x = static_cast<float>(m_config->GetBaseX() + m_config->GetArtOffsetX());
        float y = static_cast<float>(m_config->GetBaseY() + m_config->GetArtOffsetY());
        D2D1_RECT_F destRectArt = D2D1::RectF(x, y, x + size, y + size);
        m_d2dContext->DrawBitmap(
            m_placeholderArtBitmap.Get(),
            &destRectArt,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
    }

    // 5. 曲情報テキストの描画
    if (m_textBrush && m_titleTextFormat && m_artistTextFormat && m_config) {
        std::wstring dummyTitle = L"Dummy Track Title";
        std::wstring dummyArtist = L"Dummy Artist Name";

        float baseX = static_cast<float>(m_config->GetBaseX());
        float baseY = static_cast<float>(m_config->GetBaseY());

        // 曲名描画
        float titleX = baseX + static_cast<float>(m_config->GetTitleOffsetX());
        float titleY = baseY + static_cast<float>(m_config->GetTitleOffsetY());
        D2D1_RECT_F titleRect = D2D1::RectF(titleX, titleY, titleX + 800.0f, titleY + 100.0f);
        m_d2dContext->DrawText(
            dummyTitle.c_str(),
            static_cast<UINT32>(dummyTitle.length()),
            m_titleTextFormat.Get(),
            &titleRect,
            m_textBrush.Get()
        );

        // アーティスト名描画
        float artistX = baseX + static_cast<float>(m_config->GetArtistOffsetX());
        float artistY = baseY + static_cast<float>(m_config->GetArtistOffsetY());
        D2D1_RECT_F artistRect = D2D1::RectF(artistX, artistY, artistX + 800.0f, artistY + 50.0f);
        m_d2dContext->DrawText(
            dummyArtist.c_str(),
            static_cast<UINT32>(dummyArtist.length()),
            m_artistTextFormat.Get(),
            &artistRect,
            m_textBrush.Get()
        );
    }

    HRESULT hr = m_d2dContext->EndDraw();

    if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) {
        // VSync同期でPresent (1)
        m_swapChain->Present(1, 0);
    }
}
