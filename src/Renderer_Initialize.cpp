#include "Renderer.h"
#include "Config/ConfigManager.h"
#include "resource.h"
#include <initguid.h>
#pragma comment(lib, "dxguid.lib")

#include "Widget_AppLogo.h"
#include "Widget_LogoMenu.h"
#include "Widget_TrackInfo.h"
#include "Widget_SeekBar.h"
#include "Widget_PlaybackControls.h"
#include "Widget_VolumeControl.h"
#include "Widget_GlobalHotkeys.h"
#include "Widget_Playlist.h"
#include "Widget_ResizeGrip.h"
#include "Widget_Osd.h"

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
    
    hr = m_d2dContext->CreateLayer(nullptr, &m_backgroundLayer);
    if (FAILED(hr)) return false;
    
    m_visualizer.SetConfig(m_config);
    m_widgets.clear();
    m_widgets.push_back(std::make_unique<AppLogoWidget>());
    m_widgets.push_back(std::make_unique<LogoMenuWidget>());
    m_widgets.push_back(std::make_unique<TrackInfoWidget>());
    m_widgets.push_back(std::make_unique<SeekBarWidget>());
    m_widgets.push_back(std::make_unique<PlaybackControlsWidget>());
    m_widgets.push_back(std::make_unique<VolumeControlWidget>());
    m_widgets.push_back(std::make_unique<GlobalHotkeysWidget>());
    m_widgets.push_back(std::make_unique<PlaylistWidget>());
    m_widgets.push_back(std::make_unique<ResizeGripWidget>());
    m_widgets.push_back(std::make_unique<OsdWidget>());
    for (auto& widget : m_widgets) {
        widget->CreateResources(m_d2dContext.Get(), m_wicFactory.Get(), m_dwriteFactory.Get(), m_config);
    }

    return true;
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
