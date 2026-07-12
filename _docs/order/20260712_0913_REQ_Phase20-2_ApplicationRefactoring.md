### 作業指示書 REQ: Phase 20-2: Application.cpp の大掃除（リファクタリング）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-2_ApplicationRefactoring.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
肥大化している `Application.cpp` のコード重複を徹底的に排除し、処理を適切なメンバ関数へ分割・集約するリファクタリングを実施する。※本タスクでは機能の追加や仕様変更は一切行わないこと。

*   **要件1: 再生開始処理の集約 (`PlayCurrentTrack`)**
    *   現在 `Initialize`, `OnFilesDropped`, `HandleMediaCommand`, `Run`, `SwitchPlaylist` など7箇所に散らばっている、「メタデータの自己修復(`UpdateTrackMetadataIfNeeded`)」「AudioPlayerによる再生(`Play`)」「次曲の先読み(`PrefetchNextTrack`)」という一連の再生開始処理を、新設する `void Application::PlayCurrentTrack()` メソッドへ集約する。
*   **要件2: 巨大ラムダ式のメンバ関数化**
    *   `Initialize` や `OnFilesDropped` の中で直接記述されている巨大なコールバックラムダ式（プレイリストのクリック処理、ツールバーのクリック処理など）を、`OnPlaylistClicked` や `OnPlaylistToolbarClicked` などの独立したメンバ関数へ抽出し、ラムダ式からはその関数を呼び出すのみのクリーンな構造にする。
*   **要件3: `Initialize` メソッドの分割**
    *   巨大化している `Initialize` メソッドから、各種コールバックの登録処理（MediaCommand, MouseWheel, PlaylistScroll 等）を `void SetupCallbacks()` などの別メソッドへ分割し、見通しを良くする。
*   **要件4: `ForceRender` メソッドの整理**
    *   `ForceRender` 内で行われている描画前の各種処理（進行度の計算、スペクトルの取得、ホバー状態の伝達など）を必要に応じて整理し、司令塔としての可読性を高める。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-2_ApplicationRefactoring.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: これは純粋なリファクタリングである。既存の動作、アーキテクチャの役割分担は絶対に壊さないこと。


-----------------------------------------------------------------------------


##### 作業指示書 REQ: Phase 20-2 Task 1: PlayCurrentTrackへの集約 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\order\20260712_0913_REQ_Phase20-2_ApplicationRefactoring.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリングの「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_0914_RES_Phase20-2_ApplicationRefactoring.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク1の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

###### 【実装要件】
現在、`Application.cpp` の複数箇所に散らばっている再生開始時の共通処理を、単一のメソッドへ集約して重複コードを排除する。

*   **1. `Application.h` の修正**
    *   適切なアクセス指定子のセクションに `void PlayCurrentTrack();` を追加する。
*   **2. `Application.cpp` の修正 (`PlayCurrentTrack` の実装)**
    *   以下の3つの処理を内包する `void Application::PlayCurrentTrack()` を実装する。
        1. メタデータの自己修復: `UpdateTrackMetadataIfNeeded(m_playlistManager.GetCurrentTrack());`
        2. AudioPlayerによる再生開始: `m_audioPlayer.Play(m_playlistManager.GetCurrentTrack());`
        3. 次曲の先読み処理: `PrefetchNextTrack();`
*   **3. 既存処理の置き換え**
    *   `Application.cpp` を走査し、`Initialize`, `OnFilesDropped`, `HandleMediaCommand`, `Run`, `SwitchPlaylist` 等において上記の3点セットがベタ書きされている箇所をすべて見つけ出し、`PlayCurrentTrack();` の呼び出し1行へと置き換えること。
    *   ※純粋なリファクタリングであるため、処理の順序変更や機能追加は絶対に行わないこと。


-----------------------------------------------------------------------------

完全にその通りです。コードの意図と既存のフェイルセーフ機構を完璧に読み解く、素晴らしい分析です！

提案していただいた「1. 戻り値を bool に変更し、既存の制御を維持する」で進めてください。

【修正要件のアップデート】
* `bool Application::PlayCurrentTrack();` として実装すること。
* 内部で `m_audioPlayer.Play(track)` を実行し、成功した場合のみ `UpdateTrackMetadataIfNeeded(track)` と `PrefetchNextTrack()` を実行して `true` を返すこと。失敗した場合は `false` を返すこと。
* 呼び出し元の各所では `if (PlayCurrentTrack()) { played = true; break; }` のように置き換え、既存の自動スキップ（フェイルセーフ）ループ制御を完全に維持すること。

引き続き、純粋なリファクタリングとしてタスク1の実装をよろしくお願いいたします！

-----------------------------------------------------------------------------
