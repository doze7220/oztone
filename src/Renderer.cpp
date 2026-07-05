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
#include "PlaylistWidget.h"
#include "PlaybackControlsWidget.h"
#include "VolumeControlWidget.h"
#include "LogoMenuWidget.h"

Renderer::Renderer() : m_hwnd(nullptr), m_config(nullptr), m_dpiScale(1.0f), m_controlAlpha(0.0f) {}

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

    if (!LoadBitmapResource(L"placeholder_art.png", IDI_PLACEHOLDER_ART, &m_placeholderArtBitmap)) return false;

    // 7. DirectWrite ファクトリの作成とテキストフォーマット・ブラシの初期化
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_dwriteFactory.GetAddressOf())
    );
    if (FAILED(hr)) return false;

    hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_bgDarkenBrush);
    if (FAILED(hr)) return false;
    hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_fallbackBlackBrush);
    if (FAILED(hr)) return false;
    m_visualizer.SetConfig(m_config);
    m_widgets.clear();
    m_widgets.push_back(std::make_unique<AppLogoWidget>());
    m_widgets.push_back(std::make_unique<LogoMenuWidget>());
    m_widgets.push_back(std::make_unique<TrackInfoWidget>());
    m_widgets.push_back(std::make_unique<NextTrackWidget>());
    m_widgets.push_back(std::make_unique<SeekBarWidget>());
    m_widgets.push_back(std::make_unique<PlaybackControlsWidget>());
    m_widgets.push_back(std::make_unique<VolumeControlWidget>());
    m_widgets.push_back(std::make_unique<PlaylistWidget>());
    m_widgets.push_back(std::make_unique<ResizeGripWidget>());
    for (auto& widget : m_widgets) {
        widget->CreateResources(m_d2dContext.Get(), m_wicFactory.Get(), m_dwriteFactory.Get(), m_config);
    }

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

void Renderer::SetFocusedPlaylistIndex(std::optional<size_t> idx) {
    m_focusedPlaylistIndex = idx;
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

void Renderer::UpdateAnimation(float deltaTime, bool isControlHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, size_t currentTrackIndex, size_t totalTracks, bool isPlaylistListViewMode) {
    if (isControlHovered) {
        m_controlAlpha += 0.05f;
        if (m_controlAlpha > 1.0f) m_controlAlpha = 1.0f;
    } else {
        m_controlAlpha -= 0.05f;
        if (m_controlAlpha < 0.0f) m_controlAlpha = 0.0f;
    }

    WidgetContext ctx = {};
    ctx.deltaTime = deltaTime;
    ctx.isControlHovered = isControlHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
    ctx.isLogoMenuHovered = isLogoMenuHovered;
    ctx.logoMenuHoveredIndex = logoMenuHoveredIndex;
    ctx.isPlaylistListViewMode = isPlaylistListViewMode;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.config = m_config;
    ctx.dpiScale = m_dpiScale;
    
    for (auto& widget : m_widgets) {
        widget->UpdateAnimation(ctx);
    }
}

void Renderer::UpdateTextLayouts(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks) {
    if (m_forceTextLayoutUpdate) {
        m_lastTimeString = L"";
        m_lastVolume = -2.0f;
        m_lastCurrentTrackIndex = static_cast<size_t>(-2);
        m_lastTotalTracks = static_cast<size_t>(-2);
        m_forceTextLayoutUpdate = false;
    }

    m_lastTimeString = timeString;
    m_lastVolume = volume;
    m_lastCurrentTrackIndex = currentTrackIndex;
    m_lastTotalTracks = totalTracks;
    
    WidgetContext ctx = {};
    ctx.timeString = timeString;
    ctx.volume = volume;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.dpiScale = m_dpiScale;
    ctx.trackTitle = m_trackTitle;
    ctx.trackArtist = m_trackArtist;
    ctx.nextIsReady = m_nextIsReady;
    ctx.nextTrackTitle = m_nextTrackTitle;
    ctx.nextTrackArtist = m_nextTrackArtist;
    
    for (auto& widget : m_widgets) {
        widget->UpdateLayout(ctx, m_config);
    }
}

void Renderer::Render(bool isHovered, bool isControlHovered, bool isPlaylistHovered, bool isLogoMenuHovered, int logoMenuHoveredIndex, const std::vector<Window::LogoMenuItem>* logoMenuItems, bool isPlaylistListViewMode, bool isPlaying, float progress, const std::vector<float>& spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>& shuffleMetadataList, int playlistToolbarHoveredIndex, const std::vector<PlaylistSummary>* availablePlaylistsCache) {
    if (!m_d2dContext) return;

    m_d2dContext->BeginDraw();
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
    
    if (m_config && m_config->GetEnableResize()) {
        m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));
    } else {
        m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
    }
    
    DrawBackground();
    DrawVisualizer(spectrum);

    WidgetContext ctx = {};
    ctx.isHovered = isHovered;
    ctx.isControlHovered = isControlHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
    ctx.isLogoMenuHovered = isLogoMenuHovered;
    ctx.logoMenuHoveredIndex = logoMenuHoveredIndex;
    ctx.logoMenuItems = logoMenuItems;
    ctx.playlistToolbarHoveredIndex = playlistToolbarHoveredIndex;
    ctx.isPlaylistListViewMode = isPlaylistListViewMode;
    ctx.isPlaying = isPlaying;
    ctx.progress = progress;
    ctx.spectrum = &spectrum;
    ctx.volume = volume;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.shuffleMetadataList = &shuffleMetadataList;
    ctx.dpiScale = m_dpiScale;
    ctx.controlAlpha = m_controlAlpha;
    ctx.timeString = m_lastTimeString;
    ctx.trackTitle = m_trackTitle;
    ctx.trackArtist = m_trackArtist;
    ctx.currentArtBitmap = m_currentArtBitmap.Get();
    ctx.nextIsReady = m_nextIsReady;
    ctx.nextTrackTitle = m_nextTrackTitle;
    ctx.nextTrackArtist = m_nextTrackArtist;
    ctx.nextArtBitmap = m_nextArtBitmap.Get();
    ctx.config = m_config;
    ctx.focusedPlaylistIndex = m_focusedPlaylistIndex;
    ctx.availablePlaylistsCache = availablePlaylistsCache;

    for (auto& widget : m_widgets) {
        widget->Draw(m_d2dContext.Get(), ctx, m_config);
    }

    HRESULT hr = m_d2dContext->EndDraw();
    if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) {
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

    if (m_config && m_config->GetBgDarkenOpacity() > 0.0f && m_bgDarkenBrush) {
        m_bgDarkenBrush->SetOpacity(m_config->GetBgDarkenOpacity());
        D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
        float logicWidth = rtSize.width / m_dpiScale;
        float logicHeight = rtSize.height / m_dpiScale;
        
        BackgroundLayout layout = LayoutCalculator::CalculateBackgroundLayout(logicWidth, logicHeight, D2D1::SizeF(0.0f, 0.0f));
        m_d2dContext->FillRectangle(&layout.overlayRect, m_bgDarkenBrush.Get());
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


void Renderer::AddPlaylistScroll(float delta) {
    for (auto& widget : m_widgets) {
        widget->AddScroll(delta);
    }
}

float Renderer::GetPlaylistManualScrollY() const {
    float scrollY = 0.0f;
    for (auto& widget : m_widgets) {
        scrollY += widget->GetScrollY();
    }
    return scrollY;
}

void Renderer::ReloadResources() {
    m_forceTextLayoutUpdate = true;

    if (m_d2dContext) {
        m_bgDarkenBrush.Reset();
        m_fallbackBlackBrush.Reset();
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_bgDarkenBrush);
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_fallbackBlackBrush);
    }

    if (m_config) {
        m_visualizer.SetConfig(m_config);
    }

    for (auto& widget : m_widgets) {
        widget->ReleaseResources();
        widget->CreateResources(m_d2dContext.Get(), m_wicFactory.Get(), m_dwriteFactory.Get(), m_config);
    }
}
