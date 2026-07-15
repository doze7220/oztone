#include "Visualizer_HaloDust.h"
#include "ConfigManager.h"
#include <cmath>
#include <random>

// ==========================================
// Halo Dust の設定
// ==========================================
namespace {
    constexpr float CIRCLE_NEON_GLOW_THICKNESS = 5.0f; // ネオングローの太さ
    constexpr float CIRCLE_GLOW_OPACITY = 0.3f;      // ネオングローの透明度
    constexpr float CIRCLE_FILL_OPACITY = 0.2f;      // ポリゴン内部の塗りつぶし透明度
    constexpr float CIRCLE_LASER_OPACITY = 0.2f;     // レーザーの最大透明度
    constexpr float CIRCLE_LASER_SPEED_MULTIPLIER = 0.2f; // レーザーの飛行速度の倍率
    constexpr float CIRCLE_PARTICLE_SPEED_MULTIPLIER = 0.3f; // パーティクルの飛散速度の倍率
    constexpr float CIRCLE_LASER_LIFE_MULTIPLIER = 3.0f; // レーザーの寿命倍率
    constexpr float CIRCLE_PARTICLE_LIFE_MULTIPLIER = 2.0f; // パーティクルの寿命倍率

    D2D1::ColorF HsvToRgb(float h, float s, float v) {
        float c = v * s;
        float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        float r = 0, g = 0, b = 0;
        if (h >= 0 && h < 60) { r = c; g = x; b = 0; }
        else if (h >= 60 && h < 120) { r = x; g = c; b = 0; }
        else if (h >= 120 && h < 180) { r = 0; g = c; b = x; }
        else if (h >= 180 && h < 240) { r = 0; g = x; b = c; }
        else if (h >= 240 && h < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }
        return D2D1::ColorF(r + m, g + m, b + m, 1.0f);
    }
}

VisualizerHaloDust::VisualizerHaloDust() : m_initialized(false), m_particleCooldown(0) {}

VisualizerHaloDust::~VisualizerHaloDust() {
    ReleaseResources();
}

bool VisualizerHaloDust::Initialize(ID2D1DeviceContext* context) {
    if (m_initialized) return true;
    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_coreBrush);
    m_initialized = true;
    return true;
}

void VisualizerHaloDust::ReleaseResources() {
    m_coreBrush.Reset();
    m_initialized = false;
}

void VisualizerHaloDust::Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) {
    if (!m_initialized) {
        Initialize(context);
    }
    if (spectrum.size() < 2 || !m_initialized) return;

    float width = drawRect.right - drawRect.left;
    float height = drawRect.bottom - drawRect.top;
    float centerX = drawRect.left + width / 2.0f;
    float centerY = drawRect.top + height / 2.0f;
    
    float refSize = (std::min)(width, height);
    float baseRadiusRatio = m_config ? m_config->GetHaloDustBaseRadiusRatio() : 0.25f;
    float graphLengthRatio = m_config ? m_config->GetHaloDustGraphLengthRatio() : 0.30f;
    float baseRadius = refSize * baseRadiusRatio;
    float graphLength = refSize * graphLengthRatio;

    // Hash track info for hue
    std::wstring combined = trackTitle + trackArtist;
    size_t hashValue = std::hash<std::wstring>{}(combined);

    // ハッシュ値から純粋なHUEの生成
    float hue = static_cast<float>(hashValue % 3600) / 10.0f;

    // HSV純色を生成 (彩度100%、明度100%)
    D2D1::ColorF pureColor = HsvToRgb(hue, 1.0f, 1.0f);

    float finalR = pureColor.r;
    float finalG = pureColor.g;
    float finalB = pureColor.b;

    float glowOpacity = m_config ? m_config->GetHaloGlowOpacity() : CIRCLE_GLOW_OPACITY;
    float glowThickness = m_config ? m_config->GetHaloGlowThickness() : CIRCLE_NEON_GLOW_THICKNESS;

    D2D1_COLOR_F themeColorFill = D2D1::ColorF(finalR, finalG, finalB, CIRCLE_FILL_OPACITY);
    D2D1_COLOR_F themeColorGlow = D2D1::ColorF(finalR, finalG, finalB, glowOpacity);

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fillBrush;
    context->CreateSolidColorBrush(themeColorFill, &fillBrush);
    
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> glowBrush;
    context->CreateSolidColorBrush(themeColorGlow, &glowBrush);


    // Get current transform
    D2D1_MATRIX_3X2_F originalTransform;
    context->GetTransform(&originalTransform);

    static std::mt19937 rng(std::random_device{}());
    
    // Draw Rays
    if (m_particleCooldown > 0) {
        m_particleCooldown--;
    }
    
    float scaleFactor = (std::min)(width, height) / 1024.0f;
    
    // Process spectrum to bands
    int numBands = 64;
    if (m_circleAmplitudes.size() < numBands) {
        m_circleAmplitudes.resize(numBands, 0.0f);
    }

    float angleStep = 360.0f / numBands;

    for (int i = 0; i < numBands; ++i) {
        float ct = (float)i / (numBands - 1);
        
        // すでに対数・EQ適用済みの256個のデータを線形マッピング
        float floatIndex = ct * static_cast<float>(spectrum.size() - 1);
        
        size_t idx1 = static_cast<size_t>(floatIndex);
        size_t idx2 = idx1 + 1;
        if (idx2 >= spectrum.size()) idx2 = spectrum.size() - 1;
        if (idx1 >= spectrum.size()) idx1 = spectrum.size() - 1;
        
        float frac = floatIndex - idx1;
        float val = spectrum[idx1] + frac * (spectrum[idx2] - spectrum[idx1]);
        
        float raw_amp = val;
        
        float prevAmp = m_circleAmplitudes[i];
        if (raw_amp > m_circleAmplitudes[i]) {
            m_circleAmplitudes[i] = raw_amp;
        } else {
            m_circleAmplitudes[i] -= m_circleAmplitudes[i] * 0.15f;
        }
        
        float maxAmpPx = graphLength / 2.0f;
        if (maxAmpPx < 0.0f) maxAmpPx = 0.0f;
        float ampPx = m_circleAmplitudes[i] * maxAmpPx;
        float prevAmpPx = prevAmp * maxAmpPx;
        
        // Spawn particle if amplitude dropped from peak
        if (prevAmp > 0.4f && m_circleAmplitudes[i] < prevAmp - 0.05f) {
            std::uniform_real_distribution<float> randDist(-1.0f, 1.0f);
            
            float laserSpawnRate = m_config ? m_config->GetHaloLaserSpawnRate() : 0.1f;
            if (std::abs(randDist(rng)) < laserSpawnRate) {
                std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
                float lSpeed = m_config ? m_config->GetHaloLaserSpeed() : 1.0f;
                float lLife = m_config ? m_config->GetHaloLaserLifeTime() : 30.0f;

                LaserRay ray;
                ray.angle = angleDist(rng);
                ray.maxLifeTime = lLife * (1.0f + std::abs(randDist(rng))*0.5f);
                ray.lifeTime = 0.0f;
                ray.distance = 0.0f;
                float baseLenRatio = m_config ? m_config->GetHaloLaserLengthRatio() : 0.2f;
                ray.length = baseLenRatio * refSize * (0.5f + std::abs(randDist(rng))*0.5f);
                ray.speed = 0.0f;
                ray.startRadius = baseRadius + prevAmpPx;
                ray.acceleration = lSpeed * (0.2f + std::abs(randDist(rng))*0.3f) * scaleFactor;
                m_laserRays.push_back(ray);
            }
            
            float particleSpawnRate = m_config ? m_config->GetHaloParticleSpawnRate() : 0.2f;
            if (std::abs(randDist(rng)) < particleSpawnRate) {
                std::uniform_real_distribution<float> radDist(0.0f, 2.0f * 3.14159265f);
                float pSpeed = m_config ? m_config->GetHaloParticleSpeed() : 1.0f;
                float pLife = m_config ? m_config->GetHaloParticleLifeTime() : 60.0f;

                Particle p;
                float pAngle = radDist(rng);
                float rDist = baseRadius + prevAmpPx;
                p.x = centerX + std::cos(pAngle) * rDist;
                p.y = centerY + std::sin(pAngle) * rDist;
                p.vx = 0.0f;
                p.vy = 0.0f;
                p.maxLifeTime = pLife * (1.0f + std::abs(randDist(rng))*0.5f);
                p.lifeTime = 0.0f;
                float baseSizeRatio = m_config ? m_config->GetHaloParticleSizeRatio() : 0.05f;
                p.size = baseSizeRatio * refSize * (0.5f + std::abs(randDist(rng))*0.5f);
                p.axis = D2D1::Vector3F(randDist(rng), randDist(rng), randDist(rng));
                p.angle = 0.0f;
                p.angularVelocity = randDist(rng) * 5.0f;
                p.acceleration = pSpeed * (0.2f + std::abs(randDist(rng))*0.3f) * scaleFactor;
                m_particles.push_back(p);
            }
        }

        context->SetTransform(D2D1::Matrix3x2F::Rotation(i * angleStep, D2D1::Point2F(centerX, centerY)) * originalTransform);
        
        // Symmetrical trapezoid bar
        float halfAngleRad = (angleStep / 2.0f) * (3.14159265f / 180.0f);
        float innerR = baseRadius - ampPx;
        float outerR = baseRadius + ampPx;
        
        float sinH = std::sin(halfAngleRad);
        float cosH = std::cos(halfAngleRad);
        
        D2D1_POINT_2F pts[4] = {
            { centerX - sinH * innerR, centerY - cosH * innerR },
            { centerX + sinH * innerR, centerY - cosH * innerR },
            { centerX + sinH * outerR, centerY - cosH * outerR },
            { centerX - sinH * outerR, centerY - cosH * outerR }
        };
        
        Microsoft::WRL::ComPtr<ID2D1Factory> factory;
        context->GetFactory(&factory);
        Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
        factory->CreatePathGeometry(&path);
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        if (SUCCEEDED(path->Open(&sink))) {
            sink->BeginFigure(pts[0], D2D1_FIGURE_BEGIN_FILLED);
            for (int k = 1; k < 4; ++k) sink->AddLine(pts[k]);
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();
            context->FillGeometry(path.Get(), fillBrush.Get());
            context->DrawGeometry(path.Get(), glowBrush.Get(), glowThickness);
            context->DrawGeometry(path.Get(), m_coreBrush.Get(), 1.0f); // White core line
        }
    }

    // Restore original transform before drawing lasers and particles
    context->SetTransform(originalTransform);
    
    // Update and draw Laser Rays
    for (auto it = m_laserRays.begin(); it != m_laserRays.end(); ) {
        it->lifeTime += 1.0f;
        
        if (it->lifeTime >= it->maxLifeTime) {
            it = m_laserRays.erase(it);
            continue;
        }
        
        float progress = it->lifeTime / it->maxLifeTime;
        
        it->speed += it->acceleration;
        if (progress > 0.5f) {
             it->speed -= it->acceleration * 2.0f;
        }
        it->distance += it->speed;

        context->SetTransform(D2D1::Matrix3x2F::Rotation(it->angle, D2D1::Point2F(centerX, centerY)) * originalTransform);
        
        float fade = std::sin(progress * 3.14159265f);
        float laserBaseOpacity = m_config ? m_config->GetHaloLaserBaseOpacity() : 0.3f;
        float finalFade = fade * laserBaseOpacity;
        
        float currentLength = (std::max)(0.0f, it->length * fade);
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> rayGlowBrush;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> rayCoreBrush;
        context->CreateSolidColorBrush(D2D1::ColorF(finalR, finalG, finalB, glowOpacity * finalFade), &rayGlowBrush);
        context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, finalFade), &rayCoreBrush);
        
        D2D1_POINT_2F p0 = D2D1::Point2F(centerX, centerY - it->startRadius - it->distance);
        D2D1_POINT_2F p1 = D2D1::Point2F(centerX, centerY - it->startRadius - it->distance - currentLength);
        
        float laserThickness = m_config ? m_config->GetHaloLaserThickness() : 2.0f;
        
        if (rayGlowBrush && rayCoreBrush) {
            context->DrawLine(p0, p1, rayGlowBrush.Get(), glowThickness);
            context->DrawLine(p0, p1, rayCoreBrush.Get(), laserThickness);
        }
        
        ++it;
    }
    
    context->SetTransform(originalTransform);

    // Draw Particles
    Microsoft::WRL::ComPtr<ID2D1Factory> factory;
    context->GetFactory(&factory);

    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        it->lifeTime += 1.0f;
        
        if (it->lifeTime >= it->maxLifeTime) {
            it = m_particles.erase(it);
            continue;
        }

        float progress = it->lifeTime / it->maxLifeTime;
        
        float currentAccel = it->acceleration;
        if (progress > 0.5f) {
            currentAccel = -it->acceleration;
        }
        
        float dx = it->x - centerX;
        float dy = it->y - centerY;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 0.001f) {
            it->vx += (dx / dist) * currentAccel;
            it->vy += (dy / dist) * currentAccel;
        }

        it->x += it->vx;
        it->y += it->vy;
        it->angle += it->angularVelocity;

        // Normalize axis
        float len = std::sqrt(it->axis.x * it->axis.x + it->axis.y * it->axis.y + it->axis.z * it->axis.z);
        if (len > 0.001f) {
            float invLen = 1.0f / len;
            it->axis.x *= invLen;
            it->axis.y *= invLen;
            it->axis.z *= invLen;
        }

        float rad = it->angle * (3.14159265f / 180.0f);
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        float fade = std::sin(progress * 3.14159265f);
        float particleBaseOpacity = m_config ? m_config->GetHaloParticleBaseOpacity() : 0.5f;
        float finalFade = fade * particleBaseOpacity;
        float s = it->size; // サイズ固定化
        D2D1_VECTOR_3F pts[4] = {
            {-s, -s, 0}, { s, -s, 0}, { s,  s, 0}, {-s,  s, 0}
        };
        
        for (int j=0; j<4; ++j) {
            float x1 = pts[j].x*cosA + (it->axis.y*pts[j].z - it->axis.z*pts[j].y)*sinA + it->axis.x*(it->axis.x*pts[j].x + it->axis.y*pts[j].y + it->axis.z*pts[j].z)*(1-cosA);
            float y1 = pts[j].y*cosA + (it->axis.z*pts[j].x - it->axis.x*pts[j].z)*sinA + it->axis.y*(it->axis.x*pts[j].x + it->axis.y*pts[j].y + it->axis.z*pts[j].z)*(1-cosA);
            pts[j].x = x1;
            pts[j].y = y1;
        }

        Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
        factory->CreatePathGeometry(&path);
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        if (SUCCEEDED(path->Open(&sink))) {
            sink->BeginFigure(D2D1::Point2F(it->x + pts[0].x, it->y + pts[0].y), D2D1_FIGURE_BEGIN_FILLED);
            sink->AddLine(D2D1::Point2F(it->x + pts[1].x, it->y + pts[1].y));
            sink->AddLine(D2D1::Point2F(it->x + pts[2].x, it->y + pts[2].y));
            sink->AddLine(D2D1::Point2F(it->x + pts[3].x, it->y + pts[3].y));
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pFillBrush;
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pGlowBrush;
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pCoreBrush;
            float lifeFade = finalFade;
            context->CreateSolidColorBrush(D2D1::ColorF(finalR, finalG, finalB, lifeFade * CIRCLE_FILL_OPACITY), &pFillBrush);
            context->CreateSolidColorBrush(D2D1::ColorF(finalR, finalG, finalB, lifeFade * glowOpacity), &pGlowBrush);
            context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, lifeFade), &pCoreBrush);
            
            if (pFillBrush && pGlowBrush && pCoreBrush) {
                context->FillGeometry(path.Get(), pFillBrush.Get());
                context->DrawGeometry(path.Get(), pGlowBrush.Get(), glowThickness);
                context->DrawGeometry(path.Get(), pCoreBrush.Get(), 1.0f);
            }
        }
        
        ++it;
    }
}
