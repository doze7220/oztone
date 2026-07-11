##### 作業指示書 REQ: Phase 19-5: プリズムビート(Prism Beat)の究極ブラッシュアップ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトは「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-5_PrismBeatEvolution.md）を新規作成し、本タスクの詳細作業内容とタスクリストを記載すること。
3. チャットにて「Phase 19-5の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
Phase 19-5として、直線型オシロスコープ（`Visualizer_PrismBeat.cpp`）を3パス描画（コア＋2段階グロー）へと進化させ、パラメータの完全INI化を行う。

*   **1. `ConfigManager` の拡張 (PrismBeatパラメータのINI化)**
    *   `src/ConfigManager.h` / `.cpp` を修正し、`[Visualizer_PrismBeat]` セクションに以下の設定を追加・パース処理を実装する。
        *   `PrismLineThickness` (float, デフォルト: 1.0f): コア（芯線）の太さ
        *   `PrismGlow1Thickness` (float, デフォルト: 6.0f): グロー1（内側）の太さ
        *   `PrismGlow1Opacity` (float, デフォルト: 0.6f): グロー1（内側）の不透明度
        *   `PrismGlow2Thickness` (float, デフォルト: 16.0f): グロー2（外側）の太さ
        *   `PrismGlow2Opacity` (float, デフォルト: 0.2f): グロー2（外側）の不透明度

*   **2. `Visualizer_PrismBeat.cpp`: 3パス・ネオン描画ロジックの実装**
    *   描画処理において、現在「太い半透明（グロー）」と「細い不透明（コア）」で行われている2パス描画を廃止し、以下の **3パス描画** へと改修する。
    *   それぞれのパスを描画する際、INIから取得した Thickness と Opacity を適用すること。
        *   **第1パス (グロー外側)**: 太さ `PrismGlow2Thickness`。色は算出カラー（7色等）を使用し、アルファ値に `PrismGlow2Opacity` を適用する。
        *   **第2パス (グロー内側)**: 太さ `PrismGlow1Thickness`。色は算出カラーを使用し、アルファ値に `PrismGlow1Opacity` を適用する。
        *   **第3パス (コア芯線)**: 太さ `PrismLineThickness`。 **色は算出カラーではなく純白 (`D2D1::ColorF::White`)** を強制適用し、アルファ値は1.0fとする。
