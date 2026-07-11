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


### 作業指示書 REQ: Phase 19-2: ビジュアライザ究極進化 (実装実行: 第2陣 タスク3 究極完全版)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 19-2の「実装実行（第2陣）」である。計画書の **「タスク3」** の次項目として、本プロンプトの【実装要件】で挿入し、実行すること。
2. コード修正が完全に終わった後、計画書（RESファイル）のタスク3.1の記述を修正して `[x]` に変更し、詳細作業内容を追記して更新すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「タスク3の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件（タスク3.1: 事前スキャンON/OFFとリアルタイム自己学習の統合）】
CPU負荷の高い事前波形スキャンをオプション化し、基本は「1回目の再生中にリアルタイムで最大値を学習・計測し、曲の終了時にプレイリスト（TSV）へ保存する」自己学習型アーキテクチャを構築する。不完全なデータの保存を厳格に防ぐ。

*   **3-1. 事前スキャンのON/OFF設定 (ConfigManager)**
    *   `src/ConfigManager.h` / `.cpp` にて、`[Visualizer]` セクションに `EnablePreScan` (bool, デフォルト 0/false) を追加し、読み書きとゲッターを実装する。

*   **3-2. バックグラウンドスキャンのオプション化 (AudioPlayer / Application)**
    *   `AudioPlayer::ScanAudioData` メソッド（ファイル全編を高速デコードし `peakAmplitude` と `maxFrequency` を算出する機能）を実装する（ワイド文字対応 `ma_decoder_init_file_w` 必須）。
    *   `Application::ParseThreadFunc` にて、未解析トラックの波形スキャン処理を行うが、**`m_config->GetEnablePreScan() == true` の場合のみ** `ScanAudioData` を実行するよう条件を追加する（falseならスキップし、タグ解析のみで終わらせる）。

*   **3-3. リアルタイム自己学習ロジックと「学習有効フラグ」の実装 (AudioPlayer)**
    *   `AudioPlayer` クラスに、再生中の曲の最大値を追跡するメンバ変数（例: `m_learningPeakAmplitude`, `m_learningMaxFrequency`）と、**学習の有効性を示すフラグ `m_isLearningValid` (bool)** を追加する。
    *   `AudioPlayer::Play` の呼び出し直後に、これらを `0.0f` にリセットし、**`m_isLearningValid = true` に設定**する。
    *   **`AudioPlayer::Seek` (10秒スキップ等) が呼び出された場合、一部の波形データを読み飛ばして不完全な学習になるため、即座に `m_isLearningValid = false` に設定**する。
    *   再生中の音声処理コールバック（`AudioPlayerOnProcess`）等でFFTが計算されるたびに、そのフレームの最大振幅と最高高音を算出し、最大値を更新し続ける。
    *   これらを取得するゲッターメソッドを用意する。

*   **3-4. 学習結果のプレイリストへの反映（完走時かつフラグ有効時のみ） (Application)**
    *   自己学習データのプレイリストへの反映は、**「曲が最後まで完全に再生終了した時（`AudioPlayer::IsAtEnd()` が true になり自動的に次の曲へ進むタイミング）」かつ「`m_audioPlayer.IsLearningValid() == true`（途中でSeekされていない）」の場合にのみ限定**すること。
    *   手動スキップ、別曲選択、アプリ終了、または10秒スキップを使用した等でフラグが折れている場合は、不完全なデータによる上書きを防ぐため、学習した波形データは一切保存せずに破棄すること。
    *   条件を満たして完走時に取得した学習値が、現在の `peakAmplitude` より大きい場合、または未保存（`0.0f`）の場合にメタデータを更新し、直後に `m_playlistManager.SaveToFile` でTSVファイルへ永続化する。

##### 作業指示書 REQ: Hotfix Phase 19-2: 描画プラグインの旧ブースト補正の完全撤廃 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（YYYYMMDD_HHMM_RES_Phase19-2_VisualizerEvolution.md）の末尾に「HOTFIX」の項目を追加し、対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
現在、`Visualizer::Draw`（本体側）にて 0.0f 〜 1.0f 基準への完璧なノーマライズとEQ補正が行われたスペクトルデータが渡されているにも関わらず、描画プラグイン側（`Visualizer_PrismBeat.cpp` および `Visualizer_HaloDust.cpp`）に過去のハードコードされた強制ブースト定数が残っているため、描画が破綻（飽和）している。これを撤廃し、純粋な描画のみを行うように修正する。

*   **対象ファイル1: `src/Visualizer_PrismBeat.cpp`**
    *   ファイル内に残存している `AMPLITUDE_MULTIPLIER`, `HIGH_FREQ_BOOST` などの振幅を強制的に増幅させる定数、およびそれらを乗算している計算ロジックを完全に削除すること。
    *   渡された `spectrum` の各値はすでに 0.0f 〜 1.0f 基準であるため、これを純粋な割合として扱い、描画領域の最大高さ（`height` 等）に掛け合わせるだけの素直なマッピング計算へ修正すること。

*   **対象ファイル2: `src/Visualizer_HaloDust.cpp`**
    *   同様に、波形の振幅やパーティクル・レーザーの放出判定において、過去の「低い数値を無理やり引き上げる（または不自然に制限する）ための謎の定数倍率やマジックナンバー」を完全に削除すること。
    *   渡された `spectrum` の値（0.0f 〜 1.0f）をそのままパーセンテージとして信頼し、円の半径や飛び出しの強さとして純粋に適用するロジックへ修正すること。
