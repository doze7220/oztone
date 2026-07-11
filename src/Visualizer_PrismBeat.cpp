#include "Visualizer_PrismBeat.h"
#include <cmath>
#include <random>

// ==========================================
// PrismBeat の設定
// ==========================================
namespace {
    constexpr float NEON_GLOW_THICKNESS = 3.0f; // 外側の発光（グロー）の太さ
    constexpr float NEON_CORE_THICKNESS = 1.0f; // 内側の芯（ライン）の太さ
    constexpr float AMPLITUDE_MULTIPLIER = 4.0f; // 波形の高さの全体倍率
    constexpr float AMPLITUDE_MAX_RATIO = 0.6f; // 波形の最大高さ
    constexpr float HIGH_FREQ_BOOST = 8.0f;      // 高音域の強調度
}

VisualizerPrismBeat::VisualizerPrismBeat() : m_initialized(false) {}

VisualizerPrismBeat::~VisualizerPrismBeat() {
    ReleaseResources();
}

bool VisualizerPrismBeat::Initialize(ID2D1DeviceContext* context) {
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

void VisualizerPrismBeat::ReleaseResources() {
    for (int i = 0; i < 7; ++i) {
        m_neonBrushes[i].Reset();
    }
    m_coreBrush.Reset();
    m_initialized = false;
}

void VisualizerPrismBeat::Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) {
    if (!m_initialized) {
        Initialize(context);
    }
    if (spectrum.size() < 2 || !m_initialized) return;

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
        if (floatIndex > static_cast<float>(spectrumSize - 1)) floatIndex = static_cast<float>(spectrumSize - 1);
        
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
