#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <string>

class ConfigManager;

/**
 * @brief Direct3D 11 と Direct2D を用いたハイブリッド描画エンジン
 * 
 * 将来的な3Dビジュアライザと2D UI(アルバムアート等)を共存させるため、
 * D3D11のSwapChainからDXGIサーフェスを取り出し、D2D1のレンダーターゲットを作成する。
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * @brief DirectX関連のデバイスやスワップチェインを初期化する
     * @param hwnd ウィンドウハンドル
     * @param config 設定マネージャの参照
     * @return 成功ならtrue
     */
    bool Initialize(HWND hwnd, const ConfigManager& config);

    /**
     * @brief 毎フレーム呼び出される描画処理
     * @param isHovered ロゴ領域がホバーされているか
     */
    void Render(bool isHovered);

private:
    // D3D11 リソース
    Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;

    // D2D1 リソース
    Microsoft::WRL::ComPtr<ID2D1Factory1> m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device> m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_d2dContext;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;

    // WIC および 画像リソース
    Microsoft::WRL::ComPtr<IWICImagingFactory> m_wicFactory;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoHoverBitmap;

    HWND m_hwnd;
    const ConfigManager* m_config;

    /**
     * @brief 画像をファイルまたはリソースからロードする
     */
    bool LoadBitmapResource(const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap);
};
