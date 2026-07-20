##### 作業指示書 REQ: Hotfix: ThumbnailDatabaseのID発番修正と復元漏れ対応 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトはThumbnailDatabaseの起動時ID初期化およびID「0」の特殊化に関する単独のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbnailDatabase_IDFix.md）として新規作成すること。作業レポートに原因と対応内容を記載すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/ThumbnailDatabase.h`, `src/ThumbnailDatabase.cpp`
*   `ThumbnailDatabase.h` のクラス定義内に `static constexpr uint32_t NO_ART_THUMB_ID = 0;` を追加定義すること。
*   `ThumbnailDatabase.h` の `m_nextId` の初期値を `1` に変更すること。
*   `ThumbnailDatabase.cpp` の `Initialize` メソッド内において、インデックスファイル（.idx）をパースするループの前に `uint32_t maxId = 0;` を宣言し、ループ内で読み込んだ `thumbId` を比較して最大のIDを追跡・記録する処理を追加すること。
*   同メソッドのループ終了直後に、`m_nextId = std::max(1u, maxId + 1);` を設定し、次回の発番が必ず既存の最大IDの次（最低でも1）から始まるように修正すること。
*   `ThumbnailDatabase.cpp` の `GetThumbnailBitmap` や関連する画像取得ロジックにおいて、要求された `thumbId` が `NO_ART_THUMB_ID` (つまり `0`) であった場合は、ファイルシークやデコード処理を行わずに即座に `nullptr` (画像なし) を返すガード句を追加すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本タスクでは `ThumbnailDatabase` におけるIDの発番管理と、ID「0」の特殊化（アートなし扱い）の対応のみを行うこと。他クラスへの影響は出さないこと。
