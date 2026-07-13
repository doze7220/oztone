### 作業指示書 REQ: Hotfix: DefaultIniのSSOT化と最新パラメータ同期 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DefaultIni_SSOT.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
**[背景と大目的]**
現在、新規INI生成用のマスターデータである `ConfigManager_DefaultIni.h` (`DEFAULT_INI_CONTENT`) に最新機能のパラメータが追記されておらず、さらにC++コード内の初期値（コンストラクタでの直接代入など）と二重管理状態になっている。
本Hotfixでは、漏れている最新パラメータ群を完全に追記し、さらにC++側のハードコード初期値を撤廃。`DEFAULT_INI_CONTENT` を Single Source of Truth (真実の単一情報源) として内部でパース・利用するアーキテクチャへリファクタリングする。

*   **要件1: 最新パラメータ群の `DEFAULT_INI_CONTENT` への完全同期**
    *   Phase 19等のアップデートで追加されたパラメータ群を `ConfigManager_DefaultIni.h` 内の `DEFAULT_INI_CONTENT` へ漏れなく追記する。
    *   （対象例：HighFreqNoiseThreshold, EnablePreScan, BandGain0〜100, HaloDustのThickness, Speed, SpawnRate, LifeTime, PrismBeatのPrismLineThickness等）
*   **要件2: 自前パース関数の新設**
    *   `ConfigManager` に、メモリ上の `DEFAULT_INI_CONTENT` テキストを1行ずつ解析し、指定された `[Section]` と `Key` からデフォルト値（文字列）を抽出するテキスト解析ヘルパー関数（例: `GetDefaultIniValue(section, key)` 等）を新設する。
*   **要件3: 初期値のSSOT化（二重管理のパージ）**
    *   `ConfigManager` のコンストラクタやメンバ変数定義に散らばっているハードコードされた初期値をすべてパージ（削除）する。
    *   コンストラクタでの初期化時、`ResetToDefaults()` 実行時、およびINIファイルに値がなかった場合のオートフィル時に、新設したパース関数を用いて **必ず `DEFAULT_INI_CONTENT` からデフォルト値を引っ張ってくる** ようにロジックを一元化する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DefaultIni_SSOT.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、SSOT化の仕様変更に関する記述を更新する（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は設定値の管理アーキテクチャの変更であり、UIの挙動やビジュアライザの見え方を変更するものではない。
*   **計画のみの実行**: 重ねて指示するが、本プロンプトにおいてソースコードの直接修正は絶対に行ってはならない。

---------------------------------------------------------------

### 作業指示書 REQ: Hotfix: DefaultIniのSSOT化と最新パラメータ同期 (タスク1〜4 実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260713_0922_RES_Hotfix_DefaultIni_SSOT.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfix（DefaultIniのSSOT化）の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260713_0922_RES_Hotfix_DefaultIni_SSOT.md）の「タスク1」〜「タスク4」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「DefaultIniのSSOT化(タスク1〜4)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: DEFAULT_INI_CONTENT の完全同期 (タスク1)**
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` に、Phase 19周辺で追加された最新パラメータ（`HighFreqNoiseThreshold`, `EnablePreScan`, 5バンドEQゲイン, PrismBeatパラメータ, HaloDustパラメータ等）を漏れなく追記し、マスターデータを完全な状態にする。
*   **要件2: パース関数の実装とフェイルファスト (タスク2)**
    *   `src/ConfigManager.h / .cpp` に `GetDefaultIniValue(section, key)` のようなパース関数を新設し、メモリ上の `DEFAULT_INI_CONTENT` 文字列から値を抽出するロジックを実装する。
    *   **【重要】抽出時に指定された Section や Key が見つからなかった場合は、空文字などを返して隠蔽するのではなく、`assert(false && "Missing key in DEFAULT_INI_CONTENT");` 等を用いて意図的にエラー（クラッシュ）を発生させる「フェイルファスト（Fail-Fast）」設計とすること。これにより実装漏れを即座に検知する。**
*   **要件3: ハードコードのパージと委譲 (タスク3)**
    *   `LoadOrWriteInt`, `LoadOrWriteFloat`, `LoadOrWriteString` のシグネチャからデフォルト値の引数を削除する。
    *   内部で `GetDefaultIniValue` を呼び出してデフォルト値を取得するように改修する。
    *   `ConfigManager_*.cpp` に分散している各 `Load*Settings` メソッド内の呼び出しから、ハードコードされた第3引数（デフォルト値）をすべて削除する。
*   **要件4: ドキュメントの更新 (タスク4)**
    *   `PROJECT_ARCHITECTURE.md` の ConfigManager の説明部分を更新し、初期値管理が `DEFAULT_INI_CONTENT` にSSOT化された旨とフェイルファストの仕様を追記する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は設定管理アーキテクチャのリファクタリングであり、既存のビジュアライザやUIの挙動自体を変更しないこと。
