##### 作業指示書 REQ: Hotfix Phase 19-2: ピュアリニアのピンクノイズ補正とEQ対数アンカー化 (実装実行)

以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260711_1300_RES_Phase19-2_VisualizerEvolution.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（20260711_1300_RES_Phase19-2_VisualizerEvolution.md）の末尾に「HOTFIX」を追加し、対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
コミットを巻き戻したピュアリニア計算状態をベースに、前回成功した「5バンドEQの対数位置合わせ（十字アンカー）」を再適用する。また、キック（低音）の最大音圧で全帯域をリニア除算すると中高音域が潰れてしまう物理法則（1/fゆらぎ）を解決するため、ノーマライズ前に周波数に応じたピンクノイズ補正（高音域ブースト）を適用する。

*   **対象ファイル: `src/Visualizer.cpp`**
    *   `Visualizer::Draw` 内の各帯域の計算ループにおいて、以下の2点を修正・追加すること。
    *   **1. EQと補正のための対数割合（ratio）計算**
        *   インデックス `i` が全体 `maxFrequency` に対してどの位置にいるかを、`std::log10` を用いた対数スケールで計算する（前回の十字アンカー化と同じロジック）。
        *   `float logI = std::log10(static_cast<float>(i) + 1.0f);`
        *   `float logMax = std::log10(static_cast<float>(maxFrequency) + 1.0f);`
        *   `float ratio = (logMax > 0.0f) ? (logI / logMax) : 0.0f;`
        *   `ratio = std::clamp(ratio, 0.0f, 1.0f);`
    *   **2. ピンクノイズ補正とノーマライズ**
        *   高音域になるほどエネルギーが小さくなる自然界の法則を補正するため、計算した `ratio` を用いて高音域を持ち上げるウェイトを算出する。
        *   `float pinkNoiseWeight = 1.0f + (ratio * 15.0f);` （※高音域に向かって最大16倍程度まで滑らかに生データをブーストする）
        *   このウェイトを生データに掛けた上で、`peakAmplitude` でリニア除算（ノーマライズ）する。
        *   `float normalized = (peakAmplitude > 0.001f) ? ((spectrum[i] * pinkNoiseWeight) / peakAmplitude) : 0.0f;`
    *   **3. 仕上げ**
        *   その後 `normalized = std::clamp(normalized, 0.0f, 1.0f);` で安全装置を通し、5バンドEQの `bandGain` を乗算して `processedSpectrum[i]` に代入する既存のロジックへ繋ぐこと。
