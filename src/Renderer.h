#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <dwrite.h>
#include <dwrite_1.h>
#include <string>
#include <vector>

class ConfigManager;

/**
 * @brief Direct3D 11 と Direct2D を用いたハイブリッド描画エンジン
 * 
 * 将来的な3Dビジュアライザと2D UI(アルバムアート等)を共存させるため、
 * D3D11のSwapChainからDXGIサーフェスを取り出し、D2D1のレンダーターゲットを作成する。
 * ウィンドウ透過にはDirectCompositionを使用し、アルファブレンドされたスワップチェインを合成する。
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
     * @param progress 再生進捗度 (0.0f - 1.0f)
     * @param timeString フォーマット済みの再生時間文字列
     * @param spectrum スペクトルデータ
     */
    void Render(bool isHovered, float progress, const std::wstring& timeString, const std::vector<float>& spectrum);

    /**
     * @brief 再生中の曲情報を設定する
     */
    void SetTrackInfo(const std::wstring& title, const std::wstring& artist);

    /**
     * @brief アルバムアートを設定する。nullptrの場合はプレースホルダーが使用される。
     */
    void SetAlbumArt(ID2D1Bitmap* bitmap);

    /**
     * @brief 次の曲情報を設定する
     */
    void SetNextTrackInfo(bool isReady, ID2D1Bitmap* art, const std::wstring& title, const std::wstring& artist);

    /**
     * @brief メモリ上のバイナリデータからID2D1Bitmapを生成する
     */
    bool LoadBitmapFromMemory(const std::vector<uint8_t>& data, ID2D1Bitmap** ppBitmap);

private:
    // D3D11 リソース
    Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;

    // DirectComposition リソース（ウィンドウ透過合成用）
    Microsoft::WRL::ComPtr<IDCompositionDevice> m_dcompDevice;
    Microsoft::WRL::ComPtr<IDCompositionTarget> m_dcompTarget;
    Microsoft::WRL::ComPtr<IDCompositionVisual> m_dcompVisual;

    // D2D1 リソース
    Microsoft::WRL::ComPtr<ID2D1Factory1> m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device> m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_d2dContext;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;

    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_titleTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_artistTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_timeTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_nextLabelTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_nextTitleTextFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_nextArtistTextFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_shadowBrush;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_shadowEffect;

    // WIC および 画像リソース

    Microsoft::WRL::ComPtr<IWICImagingFactory> m_wicFactory;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_appLogoHoverBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_placeholderArtBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_currentArtBitmap; // 現在再生中のアルバムアート

    std::wstring m_trackTitle = L"Unknown";
    std::wstring m_trackArtist = L"Unknown";

    bool m_nextIsReady = false;
    std::wstring m_nextTrackTitle = L"";
    std::wstring m_nextTrackArtist = L"";
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_nextArtBitmap;

    HWND m_hwnd;
    const ConfigManager* m_config;
    float m_dpiScale = 1.0f;

    /**
     * @brief 画像をファイルまたはリソースからロードする
     */
    bool LoadBitmapResource(const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap);
};
