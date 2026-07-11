### 作業指示書 REQ: Phase 19-2: ビジュアライザ究極進化（事前ノーマライズ＆5バンドEQ） (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは絶対にコードの修正（ファイルの書き換え）を行わないこと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-2_VisualizerEvolution.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
各ビジュアライザ描画プラグインへ渡す前の波形データに対し、事前スキャンによる100%ノーマライズ、ノイズの足切り、および5バンドEQによるダイナミクス調整を行う「最強の前処理頭脳」を `Visualizer` クラスおよびバックグラウンドスレッドに実装する。

*   **要件1: INI設定とメタデータの拡張 (ConfigManager / TrackMetadata)**
    *   `ConfigManager` に、ノイズ足切り閾値 (`HighFreqNoiseThreshold`、例: 0.001等) と、5バンドのEQゲイン設定 (`BandGain0`, `BandGain25`, `BandGain50`, `BandGain75`, `BandGain100`、デフォルトは全て1.0f) を追加し、読み書きを実装する。
    *   `TrackMetadata` 構造体、および `PlaylistManager` のTSVキャッシュ入出力において、新たに `peakAmplitude` (最大振幅) と `maxFrequency` (有効最高高音の周波数インデックス等) を保存・復元できるように拡張する。

*   **要件2: バックグラウンドスレッドでの高速波形スキャン (ParseThreadFunc / AudioPlayer)**
    *   `Application::ParseThreadFunc` においてタグ解析が完了した直後に、対象ファイルをデコーダで開き、高速で全編を読み飛ばすスキャン処理を実行させる。
    *   全サンプルの最大振幅を求めて `peakAmplitude` とする。
    *   FFT等を用いて周波数を解析し、高音域側から低音域側へ逆走査を行うことで、`HighFreqNoiseThreshold` を初めて超える周波数を特定し、有効な最高高音 (`maxFrequency`) とする。これらを `TrackMetadata` へ保存し、TSVにキャッシュする。

*   **要件3: Visualizer本体による波形データの前処理**
    *   `Visualizer::Draw` メソッド内にて、渡された生スペクトルデータに対し、以下の加工を行う。
    *   1. TSVから取得した `peakAmplitude` を基準とした固定のスケール倍率を適用し、曲のダイナミクスを保ったまま100%基準のノーマライズを行う。
    *   2. `maxFrequency` を用いて無音の高音域を切り詰め、画面横幅を有効な音域だけで使い切るようにクリッピングする。
    *   3. 5バンドのゲイン設定間を線形補間 (Lerp) して滑らかなEQカーブを生成し、各周波数帯域のスペクトル値に掛け合わせる。
    *   加工が完了した「完璧な波形データ」を、保持している `IVisualizerStyle` (PrismBeat / HaloDust) の `Draw` メソッドへ渡す。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-2_VisualizerEvolution.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **描画ロジックの不変**: 既にプラグイン化して分離した各描画スタイル (`Visualizer_PrismBeat.cpp`, `Visualizer_HaloDust.cpp`) には一切手を加えないこと。波形の加工は必ず `Visualizer.cpp` (前処理) で完結させる。
*   **非同期処理の安全確保**: 全編スキャン処理は非常に重いため、必ずバックグラウンドの解析スレッド (`ParseThreadFunc`) または同等の専用非同期処理内で行い、メインスレッド（UI描画や現在の音声再生）を絶対にブロックしないよう実装すること。
