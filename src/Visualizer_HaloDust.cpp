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
    size_t hash = std::hash<std::wstring>{}(combined);
    float hue = static_cast<float>(hash % 360);

    // Convert Hue to RGB (Saturation 1.0, Lightness 0.6)
    float c = 1.0f * (1.0f - std::abs(2.0f * 0.6f - 1.0f));
    float x_val = c * (1.0f - std::abs(std::fmod(hue / 60.0f, 2.0f) - 1.0f));
    float m = 0.6f - c / 2.0f;
    float r = 0, g = 0, b = 0;
    if(0 <= hue && hue < 60) { r = c; g = x_val; b = 0; }
    else if(60 <= hue && hue < 120) { r = x_val; g = c; b = 0; }
    else if(120 <= hue && hue < 180) { r = 0; g = c; b = x_val; }
    else if(180 <= hue && hue < 240) { r = 0; g = x_val; b = c; }
    else if(240 <= hue && hue < 300) { r = x_val; g = 0; b = c; }
    else { r = c; g = 0; b = x_val; }
    
    // Additive synthesis with #888888 (0.533f) base for realistic neon glow
    float finalR = (std::min)(1.0f, r + m + 0.533f);
    float finalG = (std::min)(1.0f, g + m + 0.533f);
    float finalB = (std::min)(1.0f, b + m + 0.533f);

    D2D1_COLOR_F themeColorFill = D2D1::ColorF(finalR, finalG, finalB, CIRCLE_FILL_OPACITY);
    D2D1_COLOR_F themeColorGlow = D2D1::ColorF(finalR, finalG, finalB, CIRCLE_GLOW_OPACITY);

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

    float hzResolution = 44100.0f / 8192.0f;
    float minHz = 20.0f;
    float maxHz = 11000.0f;
    float angleStep = 360.0f / numBands;

    for (int i = 0; i < numBands; ++i) {
        float ct = (float)i / (numBands - 1);
        float freq = minHz * std::pow(maxHz / minHz, ct);
        float floatIndex = freq / hzResolution;
        
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
            
            // Randomly spawn laser (Reduced rate to 50%)
            if (std::abs(randDist(rng)) > 0.9f) {
                std::uniform_real_distribution<float> speedDist(2.5f * CIRCLE_LASER_SPEED_MULTIPLIER, 7.5f * CIRCLE_LASER_SPEED_MULTIPLIER);
                std::uniform_real_distribution<float> lenDist(20.0f, 60.0f);
                std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
                LaserRay ray;
                ray.angle = angleDist(rng);
                ray.life = ray.maxLife = (60.0f + std::abs(randDist(rng)) * 40.0f) * CIRCLE_LASER_LIFE_MULTIPLIER; // Doubled life
                ray.distance = 0.0f;
                ray.length = lenDist(rng) * scaleFactor; // Fixed length of the beam
                ray.speed = speedDist(rng) * scaleFactor;
                ray.startRadius = baseRadius + prevAmpPx;
                m_laserRays.push_back(ray);
            }
            
            if (m_particleCooldown <= 0) {
                std::uniform_real_distribution<float> sizeDist(8.0f, 16.0f);
                std::uniform_real_distribution<float> radDist(0.0f, 2.0f * 3.14159265f);
                Particle p;
                float pAngle = radDist(rng);
                float rDist = baseRadius + prevAmpPx;
                float pSpeed = (1.0f + std::abs(randDist(rng))*2.0f) * CIRCLE_PARTICLE_SPEED_MULTIPLIER * scaleFactor;
                p.x = centerX + std::cos(pAngle) * rDist;
                p.y = centerY + std::sin(pAngle) * rDist;
                p.vx = std::cos(pAngle) * pSpeed;
                p.vy = std::sin(pAngle) * pSpeed;
                p.life = p.maxLife = (60.0f + std::abs(randDist(rng)) * 30.0f) * CIRCLE_PARTICLE_LIFE_MULTIPLIER;
                p.size = sizeDist(rng) * scaleFactor;
                p.axis = D2D1::Vector3F(randDist(rng), randDist(rng), randDist(rng));
                p.angle = 0.0f;
                p.angularVelocity = randDist(rng) * 5.0f;
                m_particles.push_back(p);
                m_particleCooldown = 15; // 4 particles per second
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
            context->DrawGeometry(path.Get(), glowBrush.Get(), CIRCLE_NEON_GLOW_THICKNESS);
            context->DrawGeometry(path.Get(), m_coreBrush.Get(), 1.0f); // White core line
        }
    }

    // Restore original transform before drawing lasers and particles
    context->SetTransform(originalTransform);
    
    // Update and draw Laser Rays
    for (auto it = m_laserRays.begin(); it != m_laserRays.end(); ) {
        it->life -= 1.0f;
        it->distance += it->speed;
        
        if (it->life <= 0) {
            it = m_laserRays.erase(it);
            continue;
        }
        
        context->SetTransform(D2D1::Matrix3x2F::Rotation(it->angle, D2D1::Point2F(centerX, centerY)) * originalTransform);
        
        float fade = it->life / it->maxLife;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> currentRayBrush;
        context->CreateSolidColorBrush(D2D1::ColorF(finalR, finalG, finalB, CIRCLE_LASER_OPACITY * fade), &currentRayBrush);
        
        D2D1_RECT_F rayRect = D2D1::RectF(centerX - 1.0f, centerY - it->startRadius - it->distance - it->length, centerX + 1.0f, centerY - it->startRadius - it->distance);
        if (currentRayBrush) {
            context->FillRectangle(&rayRect, currentRayBrush.Get());
        }
        
        ++it;
    }
    
    context->SetTransform(originalTransform);

    // Draw Particles
    Microsoft::WRL::ComPtr<ID2D1Factory> factory;
    context->GetFactory(&factory);

    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        it->x += it->vx;
        it->y += it->vy;
        it->angle += it->angularVelocity;
        it->life -= 1.0f;
        
        if (it->life <= 0) {
            it = m_particles.erase(it);
            continue;
        }

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

        // Rodrigues' rotation formula for 4 points of a quad
        float s = it->size;
        D2D1_VECTOR_3F pts[4] = {
            {-s, -s, 0}, { s, -s, 0}, { s,  s, 0}, {-s,  s, 0}
        };
        
        D2D1_POINT_2F projPts[4];
        for (int k = 0; k < 4; ++k) {
            D2D1_VECTOR_3F v = pts[k];
            float dot = v.x * it->axis.x + v.y * it->axis.y + v.z * it->axis.z;
            D2D1_VECTOR_3F cross = {
                it->axis.y * v.z - it->axis.z * v.y,
                it->axis.z * v.x - it->axis.x * v.z,
                it->axis.x * v.y - it->axis.y * v.x
            };
            
            float rx = v.x * cosA + cross.x * sinA + it->axis.x * dot * (1 - cosA);
            float ry = v.y * cosA + cross.y * sinA + it->axis.y * dot * (1 - cosA);
            
            projPts[k].x = it->x + rx;
            projPts[k].y = it->y + ry;
        }

        Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
        factory->CreatePathGeometry(&path);
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        if (SUCCEEDED(path->Open(&sink))) {
            sink->BeginFigure(projPts[0], D2D1_FIGURE_BEGIN_FILLED);
            for (int k = 1; k < 4; ++k) {
                sink->AddLine(projPts[k]);
            }
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pFillBrush;
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pGlowBrush;
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pCoreBrush;
            float lifeFade = it->life / it->maxLife;
            context->CreateSolidColorBrush(D2D1::ColorF(finalR, finalG, finalB, lifeFade * CIRCLE_FILL_OPACITY), &pFillBrush);
            context->CreateSolidColorBrush(D2D1::ColorF(finalR, finalG, finalB, lifeFade * CIRCLE_GLOW_OPACITY), &pGlowBrush);
            context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, lifeFade), &pCoreBrush);
            
            if (pFillBrush && pGlowBrush && pCoreBrush) {
                context->FillGeometry(path.Get(), pFillBrush.Get());
                context->DrawGeometry(path.Get(), pGlowBrush.Get(), CIRCLE_NEON_GLOW_THICKNESS);
                context->DrawGeometry(path.Get(), pCoreBrush.Get(), 1.0f);
            }
        }
        
        ++it;
    }
}
