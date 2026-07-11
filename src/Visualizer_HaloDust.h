#pragma once
#include "IVisualizerStyle.h"
#include <wrl/client.h>

/**
 * @brief パーティクル・レーザー演出付きの円形ビジュアライザ (Halo Dust)
 */
class VisualizerHaloDust : public IVisualizerStyle {
public:
    VisualizerHaloDust();
    ~VisualizerHaloDust() override;

    bool Initialize(ID2D1DeviceContext* context) override;
    void ReleaseResources() override;
    void SetConfig(const ConfigManager* config) override { m_config = config; }
    void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) override;

private:
    const ConfigManager* m_config = nullptr;
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
        float startRadius;
    };

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_coreBrush;
    std::vector<float> m_circleAmplitudes;
    std::vector<Particle> m_particles;
    std::vector<LaserRay> m_laserRays;
    int m_particleCooldown = 0;
    bool m_initialized;
};
