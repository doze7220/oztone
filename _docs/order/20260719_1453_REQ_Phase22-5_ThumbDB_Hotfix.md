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
