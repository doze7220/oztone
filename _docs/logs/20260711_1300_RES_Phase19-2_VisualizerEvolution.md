# RES:実装計画・作業レポート Phase 19-2: ビジュアライザ究極進化（事前ノーマライズ＆5バンドEQ）

## 1. 実装目的
各ビジュアライザ描画プラグインへ渡す前の波形データに対し、事前スキャンによる100%ノーマライズ、ノイズの足切り、および5バンドEQによるダイナミクス調整を行う「最強の前処理頭脳」を実装する。これにより、楽曲ごとの音圧差や高音域の無音領域を自動補正し、常に最適なビジュアル表現を可能にする。

## 2. アーキテクチャ設計
### 要件1: INI設定とメタデータの拡張 (ConfigManager / TrackMetadata)
- `ConfigManager` に高音域ノイズ足切り閾値 (`HighFreqNoiseThreshold`、デフォルト0.001) と、5バンドのEQゲイン (`BandGain0`, `BandGain25`, `BandGain50`, `BandGain75`, `BandGain100`、デフォルト1.0f) の設定追加および読み書き処理を実装する。
- `TrackMetadata` に `peakAmplitude` (最大振幅) と `maxFrequency` (有効最高高音周波数のインデックス等) を追加し、`PlaylistManager` のTSVキャッシュでのシリアライズ・デシリアライズに対応させる。

### 要件2: バックグラウンドスレッドでの高速波形スキャン (ParseThreadFunc / AudioPlayer)
- `Application::ParseThreadFunc` 内のタグ解析完了直後に、対象音声ファイルをデコーダで開き全編を読み飛ばす高速スキャン処理を非同期で実行する。
- スキャン中、全サンプルの最大振幅を算出して `peakAmplitude` として記録する。
- FFT解析等を行い、高音域側から逆走査することで `HighFreqNoiseThreshold` を初めて超える周波数インデックスを特定し、`maxFrequency` として記録する。
- 解析結果を `TrackMetadata` に反映し、TSVファイルへ永続化する。

### 要件3: Visualizer本体による波形データの前処理
- `Visualizer::Draw` にて、生スペクトルデータに対して以下の前処理を行う。
  1. `peakAmplitude` を用いた固定スケール倍率の適用（100%ノーマライズ）。
  2. `maxFrequency` を用いた高音域無音部分のクリッピング（描画領域の有効活用）。
  3. 5バンドのゲイン設定を線形補間 (Lerp) して生成したEQカーブの適用。
- 前処理後の波形データを保持する `IVisualizerStyle` (PrismBeat / HaloDust) の `Draw` メソッドに渡す。
- **制約**: `Visualizer_PrismBeat.cpp` および `Visualizer_HaloDust.cpp` の描画ロジックは一切変更しない。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManagerの実装拡張
  - `HighFreqNoiseThreshold` および 5バンドEQゲイン (`BandGain0` ~ `BandGain100`) のゲッター/セッターとINI読み書き処理の追加。
- [x] タスク2: TrackMetadataとPlaylistManagerの拡張
  - `TrackMetadata` 構造体に `peakAmplitude`, `maxFrequency` を追加。
  - TSVファイルの読み書き処理において、新フィールドのパースおよび出力処理を追加。
- [ ] タスク3: バックグラウンド波形スキャン処理の実装
  - `Application::ParseThreadFunc` (またはそこから呼び出されるAudioPlayer/専用関数等) にて、ファイル全編の高速デコード処理を追加。
  - 最大振幅の計算およびFFTを用いた最高有効周波数(`maxFrequency`)の特定ロジックを実装。
- [ ] タスク4: Visualizerでの波形前処理の実装
  - `Visualizer::Draw` メソッド内にて、渡された生スペクトルに対するノーマライズ、クリッピング、5バンドEQ（Lerp補間）の処理を追加。
  - 各描画プラグインへの前処理済みデータの受け渡し。
- [ ] タスク5: 作業レポートおよびアーキテクチャドキュメントの更新
  - 本RESファイルの詳細作業内容の追記。
  - `PROJECT_ARCHITECTURE.md` への変更内容の反映。

## 4. 詳細作業内容
### タスク1: ConfigManagerの実装拡張
- `ConfigManager.h` に `m_highFreqNoiseThreshold` および 5バンドのEQゲイン (`m_bandGain0` 〜 `m_bandGain100`) を追加し、各ゲッターメソッドと一括更新する `SetBandGains` 等を定義した。
- `ConfigManager.cpp` にて、`DEFAULT_INI_CONTENT` および `ResetToDefaults()` を更新し初期値を設定した。
- `LoadSettings()` で INIファイル（`[Visualizer]` セクション）からの読み込みを行い、セッターメソッドで INIファイルへの保存機能も実装した。

### タスク2: TrackMetadataとPlaylistManagerの拡張
- `PlaylistManager.h` 内の `TrackMetadata` 構造体に `peakAmplitude` (デフォルト `0.0f`) と `maxFrequency` (デフォルト `0.0f`) を追加した。
- `PlaylistManager.cpp` の `SaveToFile()` にて、追加したフィールドをタブ区切りで出力するよう改修した。
- `LoadFromFile()` にて、フィールド長をチェックしながら読み込みを行う処理を組み込み、古いフォーマットからの後方互換性を確保した。

### タスク3: バックグラウンド波形スキャン処理の実装
- 

### タスク4: Visualizerでの波形前処理の実装
- 

### タスク5: 作業レポートおよびアーキテクチャドキュメントの更新
- 

## 5. HOTFIX
(発生時に追記)
