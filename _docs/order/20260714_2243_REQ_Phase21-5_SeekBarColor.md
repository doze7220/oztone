### 作業指示書 REQ: Phase 21-5: シークバーのオーバーレイ化と完全データ駆動化 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-5_SeekBarEvolution.md）として新規作成すること。
3. レポートのフォーマットは、`PROJECT_CONSTITUTION.md` の指示に従い、テンプレートファイル `D:\ozlab\oztone\_docs\RES(ImpretonPlan)_template.md` を参照し、それに完全に準拠して出力すること。細かなタスクリストを含めること。
4. チャットにて「Phase 21-5 の計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告し、待機すること。

#### 【目的】
現在シークバー右に表示されている時間表示が10分以上になると改行してしまう。
だがそのために時間表示テキストボックスを拡張すると、普段は使わない場合にシークバーとの隙間が大きくなる。
また100分以上のMP3が出た時に、同様に対応できなくなる。
その根本的な解決を行う。

#### 【実装要件】
*   **要件1: タイムカウントのオーバーレイ化とシークバーの最大拡張**
    *   `src/LayoutCalculator.cpp` のシークバーレイアウト計算を改修する。
    *   シークバーの幅計算から時間表示用幅（`TimeAreaWidth`）の引き算を削除し、左右の `SeekBarMargin` いっぱいに最大まで広げる。
    *   時間テキストは、シークバー本体の上に重ねて（オーバーレイして）右寄せ描画する構成へ変更する。
    *   `ConfigManager` に `TimeMarginRight`（デフォルト: 5.0f等）を追加し、右端のマージンから少し離して時間テキストを描画できるようにする。
*   **要件2: シークバーのカラー＆アルファの完全データ駆動化**
    *   背景と同化するのを防ぐための明度調整や、背景アートを透けさせるステルスUI化を実現するため、カラーとアルファをINIから制御可能にする。
    *   `src/ConfigManager.h` および `ConfigManager_Playback.cpp` に、`[Layout_SeekBar]` セクションの `SeekBarFgColor`（デフォルト: "#E0E0E0" 等）、`SeekBarBgColor`（デフォルト: "#FFFFFF"）、および `FgOpacity`（デフォルト: 0.8f 等。前景の不透明度）を追加し、ゲッターおよびINI入出力を実装する。（既存の `BgOpacity` はそのまま維持・活用する）
    *   `ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` にも追記し、SSOTを維持する。
    *   `src/Widget_SeekBar.cpp` のブラシ生成処理にて、ハードコードの白から `ConfigManager` の値（`WidgetCommon::HexToColorF` 使用）へ置き換え、描画時に `FgOpacity` および `BgOpacity` を適用する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本作業はシークバーのレイアウト計算の改修とカラー・アルファ設定の追加のみに留め、不必要な他ファイルへの干渉を行わないこと。
---------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 21-5: シークバーのオーバーレイ化と完全データ駆動化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_2303_RES_Phase21-5_SeekBarEvolution.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 21-5の「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260714_2303_RES_Phase21-5_SeekBarEvolution.md）のタスクリストのチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「Phase 21-5のシークバー改修が完了しました。ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: ConfigManagerへの設定値追加とINIデフォルト更新**
    *   `src/ConfigManager.h` に `SeekBarFgColor`, `SeekBarBgColor`, `FgOpacity`, `TimeMarginRight` を追加。
    *   `src/ConfigManager_Playback.cpp` でINIの読み書きロジック（Getter追加、LoadFromIni、SaveToIni修正）を実装。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` にデフォルト設定を追記し、SSOTを維持すること。
*   **タスク2: LayoutCalculator レイアウト計算の改修 (最大拡張・オーバーレイ化)**
    *   `src/LayoutCalculator.cpp` のシークバーレイアウト計算において、幅計算から `TimeAreaWidth` 分の引き算を削除し、左右のマージンいっぱいに最大拡張する。
    *   時間テキスト描画用の矩形を、シークバー本体と重なるよう（オーバーレイ）に配置し、右端から `TimeMarginRight` の余白を空けて右寄せ描画できる矩形として算出する。
*   **タスク3: SeekBarWidget カラー・アルファのデータ駆動化**
    *   `src/Widget_SeekBar.cpp` の描画処理（ブラシ生成等）において、ハードコードされた白色を排除する。
    *   ConfigManagerから取得した `SeekBarFgColor` および `SeekBarBgColor` を `WidgetCommon::HexToColorF` で変換し、描画時にそれぞれ `FgOpacity` および `BgOpacity` を適用して描画する構造へ変更する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本作業はシークバーのレイアウト計算の改修とカラー・アルファ設定の追加のみに留め、不必要な他ファイルへの干渉を行わないこと。
---------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 21-5 Hotfix: シークバー時間テキストの独立カラー化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260714_2303_RES_Phase21-5_SeekBarEvolution.md）の末尾に「HOTFIX」の項目を追加し、原因と対応内容を追記すること。
3. チャットにて「時間テキストカラーの独立化Hotfixが完了しました。ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: ConfigManagerへのテキストカラー設定追加**
    *   `src/ConfigManager.h` に時間テキスト用の設定値 `TimeTextColor` (デフォルト: "#FFFFFF") と `TimeTextOpacity` (デフォルト: 1.0f) を追加し、ゲッターを実装する。
    *   `src/ConfigManager_Playback.cpp` の `[Layout_SeekBar]` 読み書きロジックにて、`TextColor` と `TextOpacity` のパースおよびINI保存処理を追加する。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` の `[Layout_SeekBar]` セクションに `TextColor=#FFFFFF` と `TextOpacity=1.0` を追記し、SSOTを維持する。
*   **タスク2: SeekBarWidget でのテキスト専用ブラシの適用**
    *   `src/Widget_SeekBar.cpp` の時間テキスト描画処理を改修する。
    *   テキスト描画用のブラシ色として、シークバー本体の `FgColor` を流用するのではなく、ConfigManagerから取得した `TimeTextColor` と `TimeTextOpacity` を適用すること（必要に応じて `WidgetCommon::HexToColorF` で変換し、ブラシのColorとOpacityにセットして描画する）。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本作業は時間テキストの色・アルファ設定の分離のみに留めること。
---------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 21-5 Hotfix: TimeAreaWidthの完全パージ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260714_2303_RES_Phase21-5_SeekBarEvolution.md）の末尾の「HOTFIX」項目に、原因（オーバーレイ化に伴う不要パラメータの削除）と対応内容を追記すること。
3. チャットにて「TimeAreaWidthの完全パージが完了しました。ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: ConfigManagerからの完全削除**
    *   `src/ConfigManager.h` から `m_timeAreaWidth` の変数宣言およびゲッター（`GetTimeAreaWidth`）を削除する。
    *   `src/ConfigManager_Playback.cpp` の `[Layout_SeekBar]` 読み書きロジックから `TimeAreaWidth` に関するパース処理およびINI保存処理を完全に削除する。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` に記載されている `TimeAreaWidth=110`（または設定値）の行を削除し、SSOTをクリーンアップする。
*   **タスク2: LayoutCalculatorのクリーンアップ**
    *   `src/LayoutCalculator.cpp` のシークバーレイアウト計算処理を確認し、もし時間テキストの描画矩形（`timeRect` 等）の算出に `GetTimeAreaWidth()` が残存している場合はそれを削除する。
    *   時間テキストは右寄せ描画（`DWRITE_TEXT_ALIGNMENT_TRAILING`）されるため、`timeRect` の横幅はシークバー本体と同じ幅（または十分な幅）を持たせておけば問題なく右寄せで機能する。それに合わせたクリーンな矩形計算へ修正すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本作業は `TimeAreaWidth` の削除および関連するクリーンアップのみに留めること。
---------------------------------------------------------------------------------
