#include "Renderer.h"
#include "ConfigManager.h"
#include "resource.h"
#include <algorithm>

Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr), m_dpiScale(1.0f) {}

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

    hr = m_timeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (FAILED(hr)) return false;
    hr = m_timeTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
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

void Renderer::SetTrackInfo(const std::wstring& title, const std::wstring& artist) {
    m_trackTitle = title;
    m_trackArtist = artist;
}

void Renderer::SetAlbumArt(ID2D1Bitmap* bitmap) {
    m_currentArtBitmap = bitmap;
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

void Renderer::Render(bool isHovered, float progress, const std::wstring& timeString) {
    if (!m_d2dContext) return;

    m_d2dContext->BeginDraw();
    
    // DPIスケールを適用（論理ピクセルから物理ピクセルへの変換）
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
    
    // 1. 画面全体を黒でクリア
    m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
    
    // 2. 背景アルバムアートの描画 (Cover)
    ID2D1Bitmap* artBitmap = m_currentArtBitmap ? m_currentArtBitmap.Get() : m_placeholderArtBitmap.Get();
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
    if (m_config) {
        float size = static_cast<float>(m_config->GetArtSize());
        float x = static_cast<float>(m_config->GetBaseX() + m_config->GetArtOffsetX());
        float y = static_cast<float>(m_config->GetBaseY() + m_config->GetArtOffsetY());

        if (m_currentArtBitmap) {
            D2D1_SIZE_F bitmapSize = m_currentArtBitmap->GetSize();
            float scaleX = size / bitmapSize.width;
            float scaleY = size / bitmapSize.height;
            float scale = (std::min)(scaleX, scaleY);
            
            float drawWidth = bitmapSize.width * scale;
            float drawHeight = bitmapSize.height * scale;
            
            float drawX = x + (size - drawWidth) / 2.0f;
            float drawY = y + (size - drawHeight) / 2.0f;
            
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
    if (m_textBrush && m_titleTextFormat && m_artistTextFormat && m_config) {
        float baseX = static_cast<float>(m_config->GetBaseX());
        float baseY = static_cast<float>(m_config->GetBaseY());

        // 曲名描画
        float titleX = baseX + static_cast<float>(m_config->GetTitleOffsetX());
        float titleY = baseY + static_cast<float>(m_config->GetTitleOffsetY());
        D2D1_RECT_F titleRect = D2D1::RectF(titleX, titleY, titleX + 800.0f, titleY + 100.0f);
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
        D2D1_RECT_F artistRect = D2D1::RectF(artistX, artistY, artistX + 800.0f, artistY + 50.0f);
        m_d2dContext->DrawText(
            m_trackArtist.c_str(),
            static_cast<UINT32>(m_trackArtist.length()),
            m_artistTextFormat.Get(),
            &artistRect,
            m_textBrush.Get()
        );
    }

    // 6. シークバーと時間テキストの描画
    if (m_textBrush && m_timeTextFormat && m_config) {
        D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
        renderTargetSize.width /= m_dpiScale;
        renderTargetSize.height /= m_dpiScale;

        float totalWidth = renderTargetSize.width * m_config->GetSeekBarWidthRatio();
        float startX = (renderTargetSize.width - totalWidth) / 2.0f;
        float barAreaWidth = totalWidth - static_cast<float>(m_config->GetSeekBarTimeAreaWidth());
        float y = renderTargetSize.height - static_cast<float>(m_config->GetSeekBarBottomOffset());
        float h = static_cast<float>(m_config->GetSeekBarHeight());

        // シークバーの背景 (BgOpacity)
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, m_config->GetSeekBarBgOpacity()), &bgBrush);
        if (bgBrush) {
            D2D1_RECT_F bgRect = D2D1::RectF(startX, y, startX + barAreaWidth, y + h);
            m_d2dContext->FillRectangle(&bgRect, bgBrush.Get());
        }

        // シークバーの現在位置 (不透明な白)
        D2D1_RECT_F fgRect = D2D1::RectF(startX, y, startX + barAreaWidth * progress, y + h);
        m_d2dContext->FillRectangle(&fgRect, m_textBrush.Get());

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
                m_textBrush.Get(),
                D2D1_DRAW_TEXT_OPTIONS_NONE
            );
        }
    }

    HRESULT hr = m_d2dContext->EndDraw();

    if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) {
        // VSync同期でPresent (1)
        m_swapChain->Present(1, 0);
    }
}
