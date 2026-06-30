#include "Visualizer.h"
#include "ConfigManager.h"
#include <cmath>
#include <random>

// ==========================================
// ビジュアル設定
// ==========================================
constexpr float NEON_GLOW_THICKNESS = 3.0f; // 外側の発光（グロー）の太さ
constexpr float NEON_CORE_THICKNESS = 1.0f; // 内側の芯（ライン）の太さ

// PrismBeat (直線型) の設定
constexpr float AMPLITUDE_MULTIPLIER = 4.0f; // 波形の高さの全体倍率（大きくすると激しく揺れる）
constexpr float AMPLITUDE_MAX_RATIO = 0.6f; // 波形の最大高さ（画面の高さに対する割合。0.5で画面半分が上限）
constexpr float HIGH_FREQ_BOOST = 8.0f;      // 高音域の強調度（右側に行くほど揺れやすくする係数）

// CircleParticle (円形型) の設定
constexpr float CIRCLE_BASE_RADIUS_RATIO = 0.30f; // 円の基本半径（画面の短辺に対する割合）
constexpr float CIRCLE_AMPLITUDE_MULTIPLIER = 06.0f; // 波形の高さの全体倍率
constexpr float CIRCLE_BASS_ATTENUATION = 0.6f;  // 低音域の減衰率（小さいほど低音が抑えられる）
constexpr float CIRCLE_TREBLE_BOOST = 12.0f;      // 高音域の強調度
constexpr float CIRCLE_NEON_GLOW_THICKNESS = 5.0f; // ネオングローの太さ
constexpr float CIRCLE_GLOW_OPACITY = 0.3f;      // ネオングローの透明度
constexpr float CIRCLE_FILL_OPACITY = 0.2f;      // ポリゴン内部の塗りつぶし透明度
constexpr float CIRCLE_LASER_OPACITY = 0.2f;     // レーザーの最大透明度
constexpr float CIRCLE_LASER_SPEED_MULTIPLIER = 0.2f; // レーザーの飛行速度の倍率
constexpr float CIRCLE_PARTICLE_SPEED_MULTIPLIER = 0.3f; // パーティクルの飛散速度の倍率
constexpr float CIRCLE_LASER_LIFE_MULTIPLIER = 3.0f; // レーザーの寿命倍率
constexpr float CIRCLE_PARTICLE_LIFE_MULTIPLIER = 2.0f; // パーティクルの寿命倍率
// ==========================================

Visualizer::Visualizer() : m_initialized(false) {}

Visualizer::~Visualizer() {}

bool Visualizer::Initialize(ID2D1DeviceContext* context) {
    if (m_initialized) return true;

    // 7色の定義: 赤、オレンジ、黄、緑、水色、青、紫
    D2D1_COLOR_F colors[7] = {
        D2D1::ColorF(1.0f, 0.0f, 0.0f, 0.6f), // 赤
        D2D1::ColorF(1.0f, 0.5f, 0.0f, 0.6f), // オレンジ
        D2D1::ColorF(1.0f, 1.0f, 0.0f, 0.6f), // 黄
        D2D1::ColorF(0.0f, 1.0f, 0.0f, 0.6f), // 緑
        D2D1::ColorF(0.0f, 1.0f, 1.0f, 0.6f), // 水色
        D2D1::ColorF(0.0f, 0.0f, 1.0f, 0.6f), // 青
        D2D1::ColorF(0.5f, 0.0f, 1.0f, 0.6f)  // 紫
    };

    for (int i = 0; i < 7; ++i) {
        context->CreateSolidColorBrush(colors[i], &m_neonBrushes[i]);
    }

    context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_coreBrush);

    m_initialized = true;
    return true;
}

void Visualizer::Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) {
    if (!m_initialized) {
        Initialize(context);
    }
    if (spectrum.empty() || !m_initialized) return;

    int mode = m_config ? m_config->GetVisualizerMode() : 0;
    if (mode == 1) {
        DrawCircleParticle(context, spectrum, drawRect, trackTitle, trackArtist);
    } else {
        DrawPrismBeat(context, spectrum, drawRect);
    }
}

void Visualizer::DrawPrismBeat(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect) {

    float width = drawRect.right - drawRect.left;
    float height = drawRect.bottom - drawRect.top;
    float centerY = drawRect.top + height / 2.0f;
    
    // スペクトル合計値を計算し再生中かどうかを判定
    float sum = 0.0f;
    for (float v : spectrum) {
        sum += v;
    }
    bool isPlaying = sum > 0.001f;

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> noiseDist(-2.0f, 2.0f);

    size_t spectrumSize = spectrum.size();
    float hzResolution = 44100.0f / 8192.0f;
    float minHz = 20.0f;
    float maxHz = 10000.0f;
    float WAVE_STEP_X = 4.0f;

    struct BandConfig {
        float minHz;
        float maxHz;
        int colorIndex;
        float ratio;
        float threshold;
    };

    const BandConfig BANDS[] = {
        {20.0f,    60.0f,   0, 1.00f, 0.0f},
        {60.0f,    250.0f,  1, 1.00f, 0.0f},
        {250.0f,   500.0f,  2, 1.00f, 0.0f},
        {500.0f,   2000.0f, 3, 1.00f, 0.0f},
        {2000.0f,  4000.0f, 4, 1.20f, 0.0f},
        {4000.0f,  6000.0f, 5, 1.40f, 0.0f},
        {6000.0f,  10000.0f,6, 1.60f, 0.0f}
    };

    struct ColorSegment {
        int colorIndex;
        std::vector<D2D1_POINT_2F> points;
    };
    std::vector<ColorSegment> segments;

    int currentColor = -1;
    ColorSegment* currentSegment = nullptr;
    int flip = 0;
    int x_index = 0;

    for (float x = drawRect.left; x <= drawRect.right; x += WAVE_STEP_X) {
        float t = (x - drawRect.left) / width;
        float y = centerY;
        int bandColor = 6;
        
        if (x_index >= m_smoothedAmplitudes.size()) {
            m_smoothedAmplitudes.push_back(0.0f);
        }
        
        float ct = (t - 0.05f) / 0.90f;
        float clamped_ct = (std::max)(0.0f, (std::min)(ct, 1.0f));
        
        float freq = minHz * std::pow(maxHz / minHz, clamped_ct);
        
        // 色と帯域パラメータを、X座標(clamped_ct)の均等7分割で決定する
        int colorIndex = static_cast<int>(clamped_ct * 7.0f);
        if (colorIndex < 0) colorIndex = 0;
        if (colorIndex > 6) colorIndex = 6;
        
        const BandConfig* band = &BANDS[colorIndex];
        bandColor = band->colorIndex;
        
        float floatIndex = freq / hzResolution;
        if (floatIndex < 0.0f) floatIndex = 0.0f;
        if (floatIndex > spectrumSize - 1) floatIndex = spectrumSize - 1;
        
        size_t idx1 = static_cast<size_t>(floatIndex);
        size_t idx2 = idx1 + 1;
        if (idx2 > spectrumSize - 1) idx2 = spectrumSize - 1;
        
        float frac = floatIndex - idx1;
        float val = spectrum[idx1] + frac * (spectrum[idx2] - spectrum[idx1]);
        
        float raw_amp = std::sqrt(val) / 60.0f;
        
        // アタック＆ディケイ処理（落下減衰）
        if (raw_amp > m_smoothedAmplitudes[x_index]) {
            m_smoothedAmplitudes[x_index] = raw_amp; // 瞬時に跳ね上がる（アタック）
        } else {
            m_smoothedAmplitudes[x_index] -= m_smoothedAmplitudes[x_index] * 0.15f; // 滑らかに落ちる（ディケイ）
        }
        
        float normalized = m_smoothedAmplitudes[x_index];
        if (normalized <= band->threshold) {
            normalized = 0.0f;
        } else {
            normalized *= band->ratio;
        }
        
        float boost = 1.0f + clamped_ct * HIGH_FREQ_BOOST;
        float amplitude = normalized * boost * (height * 0.3f) * AMPLITUDE_MULTIPLIER;
        amplitude = (std::min)(amplitude, height * AMPLITUDE_MAX_RATIO);
        
        // 減衰係数（フェードアウト）の計算
        float fade = 1.0f;
        if (t < 0.05f) {
            fade = t / 0.05f; // 左端で0.0、境界で1.0
        } else if (t > 0.95f) {
            fade = (1.0f - t) / 0.05f; // 右端で0.0、境界で1.0
        }
        // スムーズなカーブにする
        fade = fade * fade;
        
        amplitude *= fade;
        
        // アナログノイズもfadeを使って画面端で自然に直線へ収束させる
        float noise = 0.0f;
        if (isPlaying) {
            noise = noiseDist(rng) * fade;
        }
        
        if (flip % 2 == 0) {
            y -= (amplitude + noise);
        } else {
            y += (amplitude + noise);
        }
        
        D2D1_POINT_2F p = D2D1::Point2F(x, y);
        if (bandColor != currentColor) {
            D2D1_POINT_2F prevPoint = p;
            if (currentSegment && !currentSegment->points.empty()) {
                prevPoint = currentSegment->points.back();
            }
            segments.push_back({bandColor, {}});
            currentSegment = &segments.back();
            if (currentColor != -1) {
                currentSegment->points.push_back(prevPoint);
            }
            currentColor = bandColor;
        }
        currentSegment->points.push_back(p);
        flip++;
        x_index++;
    }

    Microsoft::WRL::ComPtr<ID2D1Factory> factory;
    context->GetFactory(&factory);

    for (const auto& seg : segments) {
        if (seg.points.size() < 2) continue;

        Microsoft::WRL::ComPtr<ID2D1PathGeometry> path;
        HRESULT hr = factory->CreatePathGeometry(&path);
        if (FAILED(hr)) continue;

        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        hr = path->Open(&sink);
        if (FAILED(hr)) continue;

        sink->BeginFigure(seg.points[0], D2D1_FIGURE_BEGIN_HOLLOW);
        for (size_t i = 1; i < seg.points.size(); ++i) {
            sink->AddLine(seg.points[i]);
        }
        sink->EndFigure(D2D1_FIGURE_END_OPEN);
        sink->Close();

        ID2D1SolidColorBrush* neonBrush = m_neonBrushes[seg.colorIndex].Get();

        // ネオン効果
        context->DrawGeometry(path.Get(), neonBrush, NEON_GLOW_THICKNESS);
        context->DrawGeometry(path.Get(), m_coreBrush.Get(), NEON_CORE_THICKNESS);
    }
}

void Visualizer::DrawCircleParticle(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) {
    float width = drawRect.right - drawRect.left;
    float height = drawRect.bottom - drawRect.top;
    float centerX = drawRect.left + width / 2.0f;
    float centerY = drawRect.top + height / 2.0f;
    float baseRadius = (std::min)(width, height) * CIRCLE_BASE_RADIUS_RATIO;

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
    
    D2D1_COLOR_F themeColorFill = D2D1::ColorF(r + m, g + m, b + m, CIRCLE_FILL_OPACITY);
    D2D1_COLOR_F themeColorGlow = D2D1::ColorF(r + m, g + m, b + m, CIRCLE_GLOW_OPACITY);

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
        
        // Low-freq attenuation and High-freq boost
        float boost = CIRCLE_BASS_ATTENUATION + ct * CIRCLE_TREBLE_BOOST;
        float raw_amp = (std::sqrt(val) * CIRCLE_AMPLITUDE_MULTIPLIER) * boost;
        
        float prevAmp = m_circleAmplitudes[i];
        if (raw_amp > m_circleAmplitudes[i]) {
            m_circleAmplitudes[i] = raw_amp;
        } else {
            m_circleAmplitudes[i] -= m_circleAmplitudes[i] * 0.15f;
        }
        
        // Spawn particle if amplitude dropped from peak
        if (prevAmp > 20.0f && m_circleAmplitudes[i] < prevAmp - 2.0f) {
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
                ray.length = lenDist(rng); // Fixed length of the beam
                ray.speed = speedDist(rng);
                m_laserRays.push_back(ray);
            }
            
            if (m_particleCooldown <= 0) {
                std::uniform_real_distribution<float> sizeDist(8.0f, 16.0f);
                std::uniform_real_distribution<float> radDist(0.0f, 2.0f * 3.14159265f);
                Particle p;
                float pAngle = radDist(rng);
                float rDist = baseRadius + prevAmp;
                float pSpeed = (1.0f + std::abs(randDist(rng))*2.0f) * CIRCLE_PARTICLE_SPEED_MULTIPLIER;
                p.x = centerX + std::cos(pAngle) * rDist;
                p.y = centerY + std::sin(pAngle) * rDist;
                p.vx = std::cos(pAngle) * pSpeed;
                p.vy = std::sin(pAngle) * pSpeed;
                p.life = p.maxLife = (60.0f + std::abs(randDist(rng)) * 30.0f) * CIRCLE_PARTICLE_LIFE_MULTIPLIER;
                p.size = sizeDist(rng);
                p.axis = D2D1::Vector3F(randDist(rng), randDist(rng), randDist(rng));
                p.angle = 0.0f;
                p.angularVelocity = randDist(rng) * 5.0f;
                m_particles.push_back(p);
                m_particleCooldown = 15; // 4 particles per second
            }
        }

        float amp = m_circleAmplitudes[i];
        if (amp > baseRadius / 2.0f) {
            amp = baseRadius / 2.0f; // Limit amplitude to half of baseRadius
        }
        
        context->SetTransform(D2D1::Matrix3x2F::Rotation(i * angleStep, D2D1::Point2F(centerX, centerY)) * originalTransform);
        
        // Symmetrical trapezoid bar
        float halfAngleRad = (angleStep / 2.0f) * (3.14159265f / 180.0f);
        float innerR = baseRadius - amp;
        float outerR = baseRadius + amp;
        
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
        context->CreateSolidColorBrush(D2D1::ColorF(r + m, g + m, b + m, CIRCLE_LASER_OPACITY * fade), &currentRayBrush);
        
        D2D1_RECT_F rayRect = D2D1::RectF(centerX - 1.0f, centerY - baseRadius - it->distance - it->length, centerX + 1.0f, centerY - baseRadius - it->distance);
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
            context->CreateSolidColorBrush(D2D1::ColorF(r + m, g + m, b + m, lifeFade * CIRCLE_FILL_OPACITY), &pFillBrush);
            context->CreateSolidColorBrush(D2D1::ColorF(r + m, g + m, b + m, lifeFade * CIRCLE_GLOW_OPACITY), &pGlowBrush);
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
