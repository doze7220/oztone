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
- [x] タスク3.1: Pre-Scan ON/OFFとリアルタイム自己学習の実装
  - `ConfigManager` に `EnablePreScan` の設定・INI読み書きを追加。
  - `Application::ParseThreadFunc` のスキャン処理を `EnablePreScan` が有効な場合のみ実行するよう条件化。
  - `AudioPlayer` クラスに再生中の波形データ（振幅・高音）を自己学習する仕組み（`m_learningPeakAmplitude`, `m_learningMaxFrequency`）を実装。
  - 再生終了後（`IsAtEnd()` 時）に自己学習データでTSVを自動更新するロジックを `Application::Run` へ追加し、バックグラウンドスキャンがOFFでも再生するだけで学習が完了する機構を実現。
- [x] タスク4: Visualizerでの波形前処理の実装
  - `Visualizer::Draw` メソッド内にて、渡された生スペクトルに対するノーマライズ、クリッピング、5バンドEQ（Lerp補間）の処理を追加。
  - 各描画プラグインへの前処理済みデータの受け渡し。
- [x] タスク5: 作業レポートおよびアーキテクチャドキュメントの更新
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
- `AudioPlayer.h` / `AudioPlayer.cpp` に、音声ファイル全編を高速デコードして `peakAmplitude` と `maxFrequency` を算出する静的メソッド `ScanAudioData` を追加した。
- `PlaylistManager` に、スキャン結果のみを更新する `UpdateScanData` と、未解析および未スキャン（`peakAmplitude == 0.0f`）のトラックを抽出するよう `GetUnparsedTracks` を改修した。
- `Application::ParseThreadFunc` の処理フローを見直し、「タグ解析」と「波形スキャン」を独立したステップとして実行するよう改善し、UIスレッドをブロックしない完全なバックグラウンド処理とした。

### タスク3.1: Pre-Scan ON/OFFとリアルタイム自己学習の実装
- `ConfigManager` に `EnablePreScan` (デフォルト `false`) を追加し、INIファイルからの読み書き処理を実装した。
- `Application::ParseThreadFunc` での波形スキャン（`AudioPlayer::ScanAudioData`）を `EnablePreScan` が有効な場合のみ実行するよう条件付きに改修した。
- `AudioPlayer` クラスに自己学習用の変数 (`m_learningPeakAmplitude`, `m_learningMaxFrequency`, `m_isLearningValid`) を追加。`Play()` で学習開始状態にリセットし、`Seek()` で不連続なデータになるため学習を無効化（`m_isLearningValid = false`）する安全機構を設けた。
- 音声処理コールバック（`ProcessAudioFrames`）内で、リアルタイムに全サンプルの最大振幅とFFT結果から有効高音域を算出し、学習変数を更新し続けるロジックを実装した。
- `Application::Run` における曲の再生終了検知（`IsAtEnd()` 発生時）に、自己学習データが有効な場合は既存の `TrackMetadata` の値と比較し、未解析か自己学習値が既存値を超えている場合に `PlaylistManager::UpdateScanData` を用いてTSVファイルを更新する処理を追加した。これにより、事前スキャンがOFFであっても1周再生するだけで自動的に完全なデータが学習されるハイブリッド解析システムが完成した。

### タスク4: Visualizerでの波形前処理の実装
- `Renderer::DrawVisualizer` および `Visualizer::Draw` のシグネチャを変更し、現在の曲のメタデータ（`peakAmplitude`, `maxFrequency`）を渡すよう改修した。
- `Visualizer::Draw` 内部にて、渡された `maxFrequency` を用いた高音域クリッピング、`peakAmplitude` を用いたノーマライズ、および `ConfigManager` から取得した5バンドEQゲインを線形補間（Lerp）して各周波数帯域へ適用するロジックを実装し、前処理済み波形を生成した。
- `IVisualizerStyle` (PrismBeat / HaloDust) には、前処理済みのデータを透過的に渡すよう変更し、描画ロジックと波形加工ロジックの分離を維持した。

### タスク5: 作業レポートおよびアーキテクチャドキュメントの更新
- 本RESファイルにタスク4までの詳細作業内容を追記した。
- `PROJECT_ARCHITECTURE.md` の「Visualizer クラス」に関する記述を更新し、波形前処理（ノーマライズ、高音域クリッピング、5バンドEQ）の責務を負うアーキテクチャへ進化した旨を追記した。

## 5. HOTFIX
- `Visualizer::Draw` における5バンドEQのLerp計算バグを修正。
  - 原因: 割合計算における境界判定の不備および自前補間計算のミスにより、ゲイン値が全帯域に波及する不具合が発生していた。また、インデックスの割合計算における整数除算の排除を徹底する必要があった。
  - 対応: インデックスの割合計算 (`ratio`) を `static_cast<float>` を用いて明示的に浮動小数点計算とした。区間判定を `<= 0.25f`, `<= 0.50f`, `<= 0.75f` に修正し、各区間の局所進行度 (`localRatio`) を算出した上で、`<cmath>` の `std::lerp` を用いて正しく線形補間を行うように修正した。

- 描画プラグインの旧ブースト補正の完全撤廃 (Phase 19-2)
  - 原因: 既に前処理で100%ノーマライズおよびEQ補正された0.0f〜1.0fのスペクトルデータが渡されているにもかかわらず、各描画プラグイン（PrismBeat, HaloDust）側に旧来の不自然なブースト定数やマジックナンバーが残存していたため、波形描画が飽和・破綻していた。
  - 対応:
    - `Visualizer_PrismBeat.cpp`: `AMPLITUDE_MULTIPLIER` や `HIGH_FREQ_BOOST`、および各帯域ごとの不要なブースト比率設定を削除。渡された値を純粋な割合として扱い、描画領域の高さに乗算する素直なマッピングへ変更。
    - `Visualizer_HaloDust.cpp`: 同様に `CIRCLE_AMPLITUDE_MULTIPLIER` 等の強制増幅定数を削除。0.0f〜1.0fの振幅値をそのままパーセンテージとして信頼し、円の半径やパーティクル・レーザー放出判定、振幅計算などに直接利用する純粋なロジックへ修正。

- ビジュアライザ描画領域のプラグイン別マッピング化 (Phase 19-2)
  - `ConfigManager` を拡張し、プラグインごとに描画領域の割合を定義する新規パラメータを追加。
    - `[Visualizer_PrismBeat]` : `MaxHeightRatio`
    - `[Visualizer_HaloDust]` : `InnerRadiusRatio`, `OuterRadiusRatio`
  - `IVisualizerStyle` に `SetConfig` を追加し、各プラグインで設定値を参照できるようにした。
  - `Visualizer_PrismBeat` では描画高さを `MaxHeightRatio` に制約。
  - `Visualizer_HaloDust` では、内径と外径をそれぞれ `InnerRadiusRatio`, `OuterRadiusRatio` の割合に基づいて算出し、0.0〜1.0 のスペクトル振幅が内径から外径までの間でマッピングされるよう描画計算式を修正した。

- ビジュアライザ(HaloDust)の描画領域マッピングの更なる最適化 (Phase 19-2 Hotfix)
  - `ConfigManager` における HaloDust の設定を `BaseRadiusRatio` (無音時基準円) と `GraphLengthRatio` (波形全体の長さ) に変更。
  - `Visualizer_HaloDust` の描画にて、振幅を基準円から内側・外側に等分して伸ばす計算（`baseRadius ± (graphLength / 2.0f) * spectrum[i]`）に修正。
  - パーティクルおよびレーザーの発生起点を外側の頂点（`baseRadius + ampPx`）に適合させた。

- FFTスケール不整合の修正とクランプ安全装置の追加 (Phase 19-2 Hotfix)
  - 原因: `peakAmplitude` にPCMサンプルのピーク値（最大1.0付近）を記録しており、それを用いてスケールの異なるFFT出力スペクトル（数十〜数百）をノーマライズしていたため、描画プラグインに1.0fを大きく超過する異常値が渡され、画面が破綻していた。
  - 対応:
    - `AudioPlayer.cpp` における自己学習（`AudioPlayerOnProcess`）および事前スキャン（`ScanAudioData`）にて、ピーク振幅の算出対象をPCMサンプルから、FFT計算後のスペクトル配列の最大値へ変更し、正しいスケールの値をTSVに記録するよう修正した。
    - `Visualizer.cpp` の `Visualizer::Draw` にて、ノーマライズおよび5バンドEQ等のすべての前処理を終えた直後（プラグインへ渡す直前）に、配列の全要素に対して `std::clamp(val, 0.0f, 1.0f)` を適用する物理的な安全装置（クリップ処理）を追加し、1.0fを超える値による描画の破綻を完全に防いだ。

- ドロップ直後の1曲目波形スキャン漏れの修正 (Phase 19-2 Hotfix)
  - 原因: 新規プレイリストにファイルをドロップした際、1曲目が即座に自動再生されることによって自己修復ロジック（`UpdateTrackMetadataIfNeeded`）が働き、タグ解析済みフラグ（`isLoaded = true`）が立ってしまう。その結果、バックグラウンドスレッド（`ParseThreadFunc`）が1曲目を「解析済み」と誤認し、波形の事前スキャン（`ScanAudioData`）ごとスキップしてしまうバグが発生していた。
  - 対応: `Application::ParseThreadFunc` 内の未解析トラックを処理するループにおいて、`isLoaded == true` であっても、波形スキャンデータが未取得（`peakAmplitude <= 1.0f` などフォールバック状態）であり、かつ `EnablePreScan` が有効な場合には、タグ解析はスキップしつつ波形の事前スキャンのみを実行しメタデータを更新するよう条件式を修正した。これにより、自動再生された1曲目であっても裏で高速に波形スキャンが完了し、正しいFFTピーク値が適用されるようになった。

- 5バンドEQカーブの対数(Log)マッピング化 (Phase 19-2 Hotfix)
  - 原因: 振幅のピュアリニア計算への回帰に伴い、5バンドEQのインデックス割合（`ratio`）計算も線形に戻ってしまっていたため、描画側の対数スケールとEQの帯域幅にズレが生じていた。
  - 対応: `Visualizer::Draw` のスペクトル加工ループにおいて、`std::log10` を用いて現在のインデックス `i` と `maxFrequency` の対数比率を計算し直す「対数（Log）マッピング化」を実装した。算出した新しい `ratio` を用いて4区間の線形補間（Lerp）を行うことで、振幅の計算はリニアのまま、EQの適用帯域のみを視覚表示に合わせた対数スケールに復元した。

- ピュアリニアのピンクノイズ補正とEQ対数アンカー化 (Phase 19-2 Hotfix)
  - 原因: キックなどの低音の最大音圧で全帯域をリニアにノーマライズすると、エネルギーが小さい中高音域が潰れてしまう1/fゆらぎの問題が発生していた。
  - 対応:
    - 5バンドEQのインデックス割合（`ratio`）計算を `std::log10` を用いた対数スケールで行うように修正（対数位置合わせの再適用）。
    - 算出した対数割合を用いて、高音域になるほどウェイトが上がるピンクノイズ補正（最大約16倍）を適用してからノーマライズを行うように変更。
    - 補正とノーマライズ後にクランプを通し、5バンドEQゲインを乗算して安全に処理結果を出力するように実装した。

- FFTスケール不整合の再修正 (Phase 19-2 Hotfix)
  - 原因: 巻き戻り等の影響により、`peakAmplitude` が再びPCMサンプルの最大値として記録される状態に戻ってしまっており、FFTスペクトル値とのスケール不整合による画面張り付きバグが再発していた。
  - 対応: `src/AudioPlayer.cpp` の自己学習 (`AudioPlayerOnProcess`) および事前スキャン (`ScanAudioData`) におけるピーク振幅算出処理を再修正し、記録・算出する対象を「PCMサンプルの最大値」から「FFT計算完了後のスペクトル配列の最大値」へ変更した。これにより、正しくノーマライズが行われる状態を復元した。

- 生データ直結とEQ適用後のクランプ修正 (Phase 19-2 Hotfix)
  - 原因: 計算結果を 0.0f 〜 1.0f にクランプ（制限）した後にEQ（`bandGain`）を乗算していたため、上限突破した波形がすべて「1.0 * bandGain」となり、画面に「EQカーブの形」がベタッと張り付いてしまうバグが発生していた。また、システム側のピンクノイズ補正やピーク除算がユーザーのマニュアルEQと衝突していた。
  - 対応: ピンクノイズ補正と `peakAmplitude` によるノーマライズを削除し、生のスペクトルデータに対して5バンドEQゲインを先に乗算するように修正。その後、最後に 0.0f 〜 1.0f のクランプを適用する純粋な構成とした。対数スケールの位置合わせ（`ratio`計算）ロジックはEQカーブのアンカー位置決定のために維持した。

- メタデータ自己修復によるスキャン結果上書きの防止 (Phase 19-2 Hotfix)
  - 原因: プレイリストが空の状態から1曲目を追加・即座に再生した際、`UpdateTrackMetadataIfNeeded` によるタグ情報の自己修復処理が走り、バックグラウンドでの波形スキャンが完了する前にタグ情報のみが更新される。この際、TSVへ保存するデータとして `TrackMetadata` を丸ごと上書きしてしまっていたため、スキャン用の初期値（`peakAmplitude = 1.0`, `maxFrequency = 2047` 等）が誤って上書き保存されてしまう非同期のすれ違いバグが発生していた。
  - 対応: `src/PlaylistManager.cpp` におけるメタデータ更新処理 `PlaylistManager::UpdateMetadata` のシグネチャを `TrackMetadata` 構造体を直接受け取る形へ変更し、古いデータを新しいメタデータで上書きする際、波形スキャンの結果パラメータ（`peakAmplitude`, `maxFrequency`）およびアートフレーミング設定については既存のプレイリストデータが持っている現在の値を退避・復元するように保護処理を追加した。これにより、タグ情報のみが安全に更新され、裏スレッドでのスキャン結果が初期化される問題を完全に防止した。
