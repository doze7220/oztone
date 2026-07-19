### 作業指示書 REQ: Phase 22-5: サムネイルDBのID駆動復元とバグ修正 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-5_ThumbDB_Hotfix.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、タスクを安全な粒度（1タスクにつき1〜2ファイル程度）に細かく分割したタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
サムネイルインデックスファイル (`.idx`) における「IDとファイルパスの紐付け喪失」および「オフセットがすべて0で記録される」致命的欠陥、ファイルシークの不具合を修正し、堅牢なID駆動データベースを復元する。

*   **要件1: .idx ファイルへのファイルパス保存と紐付けマップの構築**
    *   `ThumbnailDatabase` に、ファイルパスからIDを逆引きするための `std::unordered_map<std::wstring, uint32_t> m_pathToId;` を追加する。
    *   `Initialize` での `.idx` ロード処理を改修し、フォーマットを `thumbId \t offset \t size \t filepath` として読み込み、`m_sectorMap` と同時に `m_pathToId` も構築する。
    *   `StoreCookedData` での `.idx` 書き込み処理を改修し、末尾に `\t filepath` も出力して永続化する。
*   **要件2: ID発番・エンキュー窓口の統合 (GetOrGenerateThumbId)**
    *   `uint32_t GetOrGenerateThumbId(const std::wstring& filepath)` を新設する。
    *   内部で `m_pathToId` を検索し、存在すればその `thumbId` を返す。存在しなければ `m_nextId` をインクリメントして発番・マップへ登録した上で、直ちに `m_thumbCacher.EnqueueTrack(thumbId, filepath)` を呼び出してキューへ流し込み、発行したIDを返す。
    *   これに伴い、`Application` 層から直接 `EnqueueTrack` を呼んでいた処理（Phase 22-4で追加したもの）を廃止し、すべてこの窓口経由で解決させる設計へ改修する。
*   **要件3: オフセット0バグの修正 (StoreCookedData)**
    *   パックファイルへのバイナリ追記時、`.img` ファイルへ書き込む直前に必ず `m_imgFile.seekp(0, std::ios::end);` を実行し、直後の `tellp()` で正しい末尾のオフセットを取得してから書き込むように修正する。
*   **要件4: ファイルシーク不具合の修正 (GetThumbnailBitmap 等)**
    *   `.img` ファイルからのバイナリ読み出し時、`seekg` を呼ぶ直前に必ず `m_imgFile.clear();` を呼び出し、EOF等のストリームエラー状態をリセットして確実に指定オフセットへシークできるようにする。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-5_ThumbDB_Hotfix.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **タスクの細分化**: 本修正は多岐にわたるため、AIのコンテキスト保護（ハルシネーション防止）の観点から、タスクをファイル単位や責務単位で十分に細分化して計画を立てること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-5 計画書のアーキテクチャ違反（循環参照）修正
* 対象ファイル: D:\ozlab\oztone\_docs\logs\20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md

#### 【作業手順（厳守事項）】
1. 対象ファイル（RES）の実装計画について、以下の【指摘事項と解決方針】を読み、アーキテクチャ上の責務違反（循環参照）を自己監査すること。
2. 指摘を理解・納得した場合、対象ファイルを直接編集し、正しいアーキテクチャに沿った確定仕様へとタスクリストをアップデートすること。
3. 作業完了後、チャットにて検討結果の所感と、計画書の更新が完了した旨を報告すること。

#### 【指摘事項と解決方針】
*   **アーキテクチャ違反の指摘:**
    *   タスク1およびタスク3にて、`ThumbnailDatabase` に `SetThumbCacher` でポインタを持たせ、内部から `EnqueueTrack` を呼び出す計画となっている。
    *   しかし、`ThumbCacher` は保存のために `ThumbnailDatabase::StoreCookedData` を呼ぶため、この設計では「循環参照（相互依存）」が発生し、責務分離の原則に明確に違反する。データベースは外部モジュール（ワーカースレッド）の稼働を直接管理すべきではない。
*   **正しい設計方針 (アンサー):**
    *   **タスク1＆3の修正**: `ThumbnailDatabase` から `ThumbCacher` への依存（ポインタ保持や内部呼び出し）を完全にパージする。`GetOrGenerateThumbId` のシグネチャを `uint32_t GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew)` のようにし、新規発番されたかどうかを呼び出し元へ返す純粋な辞書管理メソッドとする。
    *   **タスク5の修正**: `Application` 層（Initialize, Playlist, FileDrop 等の曲が追加される箇所）が司令塔となり、`GetOrGenerateThumbId` を呼び出し、`out_isNew == true` が返ってきた場合のみ、自ら `m_thumbCacher.EnqueueTrack(thumbId, filepath)` を呼び出して非同期生成を発注する設計とする。

#### 【ファイルの修正要件】
*   対象ファイル内の「タスク1」「タスク3」「タスク5」の記述を、上記の方針に基づく設計へと断定的に書き換えること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-5 Task 1-2 : サムネイルDBのID駆動復元とバグ修正（第1撃：ヘッダ定義の更新）
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルDB修正の第1フェーズである。直ちに以下の【実装要件】に従ってコードの修正を実行すること。実装ファイル（.cpp）のロジックやタスク3以降は後続の指示で行うため、今は絶対に手を触れないこと。
2. 作業完了後、実装したコードと既存の作業レポート（20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md）のタスク1およびタスク2の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスク1」および「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「タスク1とタスク2が完了しました。ビルドエラーが出る状態ですが、想定通りです。次の指示をお願いします」と報告すること。

#### 【実装要件】
*   **タスク1: ThumbnailDatabase ヘッダおよび基本定義の改修**
    *   対象ファイル: `src/ThumbnailDatabase.h`
    *   `std::unordered_map<std::wstring, uint32_t> m_pathToId;` を追加する。
    *   `uint32_t GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew);` を新設し、不要となる `GetThumbnailId` 等があれば削除/置換する。
    *   `StoreCookedData` の引数に `const std::wstring& filepath` を追加する。
*   **タスク2: ThumbCacher キュー構造の改修**
    *   対象ファイル: `src/ThumbCacher.h`
    *   タスクキューを `std::queue<std::pair<uint32_t, std::wstring>> m_taskQueue;` に変更する。
    *   `EnqueueTrack` の引数を `(uint32_t thumbId, const std::wstring& filepath)` に変更する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本REQの目的は「ヘッダファイルのシグネチャとメンバ変数の更新」のみである。`.cpp` ファイルの実装は絶対に書き換えないこと（意図的にビルドエラーになる状態で完了とする）。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-5 Task 3-4 : サムネイルDBのID駆動復元とバグ修正（第2撃：バックエンドの実装）
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルDB修正の第2フェーズである。直ちに以下の【実装要件】に従ってコードの修正を実行すること。タスク5以降は後続の指示で行うため絶対に手を触れないこと。
2. 作業完了後、実装したコードと既存の作業レポート（20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md）のタスク3およびタスク4の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスク3」および「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「タスク3とタスク4が完了しました。Application層が未接続のためビルドエラーが出ますが、想定通りです。次の指示をお願いします」と報告すること。

#### 【実装要件】
*   **タスク3: ThumbnailDatabase.cpp の不具合修正と要件実装**
    *   対象ファイル: `src/ThumbnailDatabase.cpp`
    *   `Initialize` メソッドでの `.idx` ロード処理を改修し、フォーマットを `thumbId \t offset \t size \t filepath` として読み込み、`m_sectorMap` への登録と同時に `m_pathToId[filepath] = thumbId;` を構築する。
    *   `GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew)` を実装する。内部で `m_pathToId` を検索し、存在すれば `out_isNew = false` とともにそのIDを返す。存在しなければ `m_nextId` を発番してマップへ登録し、`out_isNew = true` とともに新しいIDを返す（ここではエンキュー等、外部の呼び出しは絶対に行わない）。
    *   `StoreCookedData` の引数に `filepath` を追加。`.img` ファイルへ書き込む直前に必ず `m_imgFile.seekp(0, std::ios::end);` と `tellp()` を実行して正しい末尾オフセットを取得するバグ修正を行う。さらに `.idx` ファイルへの書き込みフォーマットの末尾に `\t filepath` を追加して永続化する。
    *   `.img` ファイルからのバイナリ読み出し時（`GetThumbnailBitmap` 等）、`seekg` を呼ぶ直前に必ず `m_imgFile.clear();` を呼び出し、EOF等のストリームエラー状態をリセットする修正を適用する。
*   **タスク4: ThumbCacher.cpp のワーカー処理改修**
    *   対象ファイル: `src/ThumbCacher.cpp`
    *   ワーカースレッドのループ内でキューから `std::pair<uint32_t, std::wstring>` を取り出すよう修正する。
    *   内部で `ThumbnailDatabase` へIDを問い合わせる処理（不要となった依存）を完全に削除し、キューから直接受け取った `thumbId` と `filepath` を用いて画像処理を行う。
    *   処理完了後、`m_db->StoreCookedData` を呼び出す際に、第4引数として `filepath` も正しく渡すように修正する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本REQの目的は「バックエンド層の実装（cpp）」のみである。表側である `Application` 層への結線（エンキュー呼び出し等）はまだ絶対に行わないこと。
*   **依存関係の純化**: `ThumbnailDatabase.cpp` 内から `m_thumbCacher` にアクセスしたり、工場のメソッドを呼び出したりしないこと（循環参照の禁止）。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-5 Task 5-6 : サムネイルDBのID駆動復元とバグ修正（第3撃：Application層の司令塔化とドキュメント更新）
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルDB修正の最終フェーズである。直ちに以下の【実装要件】に従ってコードおよびドキュメントの修正を実行すること。
2. 作業完了後、実装したコードと既存の作業レポート（20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md）の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスク5」および「タスク6」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「Phase 22-5 の全タスクが完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **タスク5: Application 層の呼び出し統合（司令塔化）**
    *   対象ファイル: `src/Application_Initialize.cpp`, `src/Application_Playlist.cpp`, `src/Application_FileDrop.cpp` 等の楽曲追加処理部分
    *   楽曲がプレイリストに追加されるタイミングで、`bool isNew = false; uint32_t thumbId = m_thumbnailDatabase.GetOrGenerateThumbId(filepath, isNew);` を呼び出す。
    *   `isNew == true` が返ってきた場合**のみ**、`m_thumbCacher.EnqueueTrack(thumbId, filepath)` を呼び出して非同期生成を発注する。
    *   これにより、Phase 22-4等で記述されていた直接的な `EnqueueTrack` 呼び出しや不要な発番処理を統合し、`Application` を「DBへ確認し、無ければ工場へ発注する司令塔」として純化させる。
*   **タスク6: ドキュメント（PROJECT_ARCHITECTURE.md）の更新**
    *   対象ファイル: `PROJECT_ARCHITECTURE.md`
    *   `Application` クラスの説明に、ThumbnailDatabaseとThumbCacherを仲介する司令塔としての責務（単方向依存で発注管理を行う）を追記する。
    *   `ThumbnailDatabase` クラスの説明を、IDとファイルパスの紐付け管理を行う「純粋な辞書管理・窓口」に更新する。
    *   `ThumbCacher` クラスの説明を、外部からの発注（IDとパスのペア）に従って黙々とサムネイルを生成する「純粋な工場（ワーカー）」に更新する。

#### 【絶対遵守ルール (Constraints)】
*   **依存の単方向性の維持**: `Application` は `ThumbnailDatabase` と `ThumbCacher` の両方を知っていてよいが、`ThumbnailDatabase` が `ThumbCacher` を知るようなコード（依存の逆流）は絶対に書かないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-5 Hotfix 3: Renderer_Contextの古いメソッド呼び出し修正
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1453_RES_Phase22-5_ThumbDB_Hotfix.md）の末尾に「HOTFIX」として作業内容を追記すること。
3. チャットにて「Renderer_Contextの呼び出し修正が完了しました。再度ビルドをお願いします」と報告すること。

#### 【実装要件】
`src/Renderer_Context.cpp` において、`GetThumbnailId` が `ThumbnailDatabase` のメンバーではないというコンパイルエラー (C2039) が発生している。これは Phase 22-5 のヘッダ改修により該当メソッドが `GetOrGenerateThumbId` へ置換されたが、UI呼び出し側が古いまま残っているためである。直ちに以下を修正する。

*   **要件1: メソッド呼び出しの置換 (Renderer_Context.cpp)**
    *   118行目付近の `m_thumbnailDatabase.GetThumbnailId(filepath)` 等となっている呼び出し箇所を修正する。
    *   以下のように、ダミーの `bool` 変数を用意した上で、新しい `GetOrGenerateThumbId` を呼び出してIDを取得する形へと書き換えること。
        ```cpp
        bool isNew = false;
        uint32_t thumbId = m_thumbnailDatabase.GetOrGenerateThumbId(filepath, isNew);
        ```
    *   ※描画構築層（Renderer_Context）は発注の司令塔ではないため、ここで万が一 `isNew == true` が返ってきたとしても `ThumbCacher::EnqueueTrack` 等の工場への発注処理は絶対に記述しないこと（発注はApplication層が楽曲追加時に行っているため、ここではただIDを取得するだけでよい）。
