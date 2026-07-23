#include "Visualizer_PrismBeat.h"
#include "Config/ConfigManager.h"
#include <cmath>
#include <random>

// ==========================================
// PrismBeat の設定 (Constants removed, now uses ConfigManager)
// ==========================================


VisualizerPrismBeat::VisualizerPrismBeat() : m_initialized(false) {}

VisualizerPrismBeat::~VisualizerPrismBeat() {
    ReleaseResources();
}

bool VisualizerPrismBeat::Initialize(ID2D1DeviceContext* context) {
    if (m_initialized) return true;

    // 7色の定義: 赤、オレンジ、黄、緑、水色、青、紫 (基本アルファは1.0f)
    D2D1_COLOR_F colors[7] = {
        D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f), // 赤
        D2D1::ColorF(1.0f, 0.5f, 0.0f, 1.0f), // オレンジ
        D2D1::ColorF(1.0f, 1.0f, 0.0f, 1.0f), // 黄
        D2D1::ColorF(0.0f, 1.0f, 0.0f, 1.0f), // 緑
        D2D1::ColorF(0.0f, 1.0f, 1.0f, 1.0f), // 水色
        D2D1::ColorF(0.0f, 0.0f, 1.0f, 1.0f), // 青
        D2D1::ColorF(0.5f, 0.0f, 1.0f, 1.0f)  // 紫
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
    float WAVE_STEP_X = 4.0f;

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
        
        // 色と帯域パラメータを、X座標(clamped_ct)の均等7分割で決定する
        int colorIndex = static_cast<int>(clamped_ct * 7.0f);
        if (colorIndex < 0) colorIndex = 0;
        if (colorIndex > 6) colorIndex = 6;
        
        bandColor = colorIndex;
        
        // すでに対数・EQ適用済みの256個のデータを線形マッピング
        float floatIndex = clamped_ct * static_cast<float>(spectrumSize - 1);
        if (floatIndex < 0.0f) floatIndex = 0.0f;
        if (floatIndex > static_cast<float>(spectrumSize - 1)) floatIndex = static_cast<float>(spectrumSize - 1);
        
        size_t idx1 = static_cast<size_t>(floatIndex);
        size_t idx2 = idx1 + 1;
        if (idx2 > spectrumSize - 1) idx2 = spectrumSize - 1;
        
        float frac = floatIndex - idx1;
        float val = spectrum[idx1] + frac * (spectrum[idx2] - spectrum[idx1]);
        
        float raw_amp = val;
        
        // アタック＆ディケイ処理（落下減衰）
        if (raw_amp > m_smoothedAmplitudes[x_index]) {
            m_smoothedAmplitudes[x_index] = raw_amp; // 瞬時に跳ね上がる（アタック）
        } else {
            m_smoothedAmplitudes[x_index] -= m_smoothedAmplitudes[x_index] * 0.15f; // 滑らかに落ちる（ディケイ）
        }
        
        float normalized = m_smoothedAmplitudes[x_index];
        
        float maxHeightRatio = m_config ? m_config->GetVisualizerPrismBeat().MaxHeightRatio : 0.8f;
        float amplitude = normalized * (height * maxHeightRatio * 0.5f);
        
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

        float glow1Thick = m_config ? m_config->GetVisualizerPrismBeat().PrismGlow1Thickness : 6.0f;
        float glow1Opac = m_config ? m_config->GetVisualizerPrismBeat().PrismGlow1Opacity : 0.6f;
        float glow2Thick = m_config ? m_config->GetVisualizerPrismBeat().PrismGlow2Thickness : 16.0f;
        float glow2Opac = m_config ? m_config->GetVisualizerPrismBeat().PrismGlow2Opacity : 0.2f;
        float coreThick = m_config ? m_config->GetVisualizerPrismBeat().PrismLineThickness : 1.0f;

        // 第1パス (グロー外側)
        neonBrush->SetOpacity(glow2Opac);
        context->DrawGeometry(path.Get(), neonBrush, glow2Thick);
        
        // 第2パス (グロー内側)
        neonBrush->SetOpacity(glow1Opac);
        context->DrawGeometry(path.Get(), neonBrush, glow1Thick);
        
        // 第3パス (コア芯線)
        m_coreBrush->SetOpacity(1.0f);
        context->DrawGeometry(path.Get(), m_coreBrush.Get(), coreThick);
    }
}
