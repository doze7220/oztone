##### 作業指示書 REQ: Hotfix Phase 19-2: ビジュアライザ対数マッピングの下限修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポートの末尾に「HOTFIX」を追加し、対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
現在 `Visualizer.cpp` 内のファサードで行われている256ビンの対数マッピングにおいて、下限値（minIndex）が極端に低く設定されているため、低音域のカーブの立ち上がりが遅く、左側の複数のビンが20Hz以下の無音帯域を参照してしまい動かなくなるバグを修正する。

*   **対象ファイル: `src/Visualizer.cpp`**
    *   `Visualizer::Draw` メソッド内で `processedSpectrum` (256ビン) を生成するループの対数マッピング計算において、下限インデックス `minIndex` を `4.0f` (約20Hz相当) に明示的に設定すること。
    *   計算ロジックの修正例:
        `float minIndex = 4.0f;`
        `float maxIndex = std::max(maxFrequency, minIndex + 1.0f);`
        `float logMin = std::log10(minIndex);`
        `float logMax = std::log10(maxIndex);`
        ループ内 (`i = 0` to `255`):
        `float ratio = i / 255.0f;`
        `float targetIndex = std::pow(10.0f, logMin + (logMax - logMin) * ratio);`
    *   このように修正し、インデックス `4.0f` から `maxFrequency` までの範囲を綺麗に対数分割して生の振幅値を抽出するようにすること。
