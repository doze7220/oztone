#pragma once
#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <wrl/client.h>
#include <vector>
#include <string>

class ConfigManager;

struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    float size;
    D2D1_VECTOR_3F axis;
    float angle;
    float angularVelocity;
};

struct LaserRay {
    float angle;
    float life;
    float maxLife;
    float distance;
    float length;
    float speed;
};

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
     * @brief ConfigManagerへのポインタを設定する
     */
    void SetConfig(const ConfigManager* config) { m_config = config; }

    /**
     * @brief ビジュアライザを描画する
     * @param context D2D描画コンテキスト
     * @param spectrum スペクトルデータ
     * @param drawRect 描画領域の矩形
     * @param trackTitle 曲名
     * @param trackArtist アーティスト名
     */
    void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist);

private:
    void DrawPrismBeat(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect);
    void DrawCircleParticle(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist);

    const ConfigManager* m_config = nullptr;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_neonBrushes[7];
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_coreBrush;
    std::vector<float> m_smoothedAmplitudes;
    std::vector<float> m_circleAmplitudes;
    std::vector<Particle> m_particles;
    std::vector<LaserRay> m_laserRays;
    int m_particleCooldown = 0;
    bool m_initialized;
};
