#pragma once
#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <wrl/client.h>
#include <vector>
#include <string>

#pragma once
#include "IVisualizerStyle.h"
#include <memory>

class ConfigManager;

/**
 * @brief ビジュアライザを統括するファサード（管理）クラス
 */
class Visualizer {
public:
    Visualizer();
    ~Visualizer();

    /**
     * @brief 初期化処理
     */
    bool Initialize(ID2D1DeviceContext* context);

    /**
     * @brief リソースの解放処理
     */
    void ReleaseResources();

    /**
     * @brief ConfigManagerへのポインタを設定する
     */
    void SetConfig(const ConfigManager* config) {
        m_config = config;
        if (m_prismBeat) m_prismBeat->SetConfig(config);
        if (m_haloDust) m_haloDust->SetConfig(config);
    }

    /**
     * @brief 選択されているモードに応じてビジュアライザを描画する
     * @param context D2D描画コンテキスト
     * @param spectrum スペクトルデータ
     * @param drawRect 描画領域の矩形
     * @param trackTitle 曲名
     * @param trackArtist アーティスト名
     */
    void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist, float peakAmplitude = 0.0f, float maxFrequency = 0.0f);

private:
    const ConfigManager* m_config = nullptr;
    std::unique_ptr<IVisualizerStyle> m_prismBeat;
    std::unique_ptr<IVisualizerStyle> m_haloDust;
    bool m_initialized;
};
