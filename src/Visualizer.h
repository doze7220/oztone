#pragma once
#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <wrl/client.h>
#include <vector>

/**
 * @brief 7色ネオンの心電図風ビジュアライザ
 */
class Visualizer {
public:
    Visualizer();
    ~Visualizer();

    /**
     * @brief 初期化処理。描画用ブラシを作成する
     */
    bool Initialize(ID2D1DeviceContext* context);

    /**
     * @brief ビジュアライザを描画する
     * @param context D2D描画コンテキスト
     * @param spectrum スペクトルデータ
     * @param drawRect 描画領域の矩形
     */
    void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect);

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_neonBrushes[7];
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_coreBrush;
    std::vector<float> m_smoothedAmplitudes;
    bool m_initialized;
};
