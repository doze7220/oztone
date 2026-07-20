##### 作業指示書 REQ: Hotfix: サムネイルDBの初期化順序の適正化 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトはサムネイルDBの初期化順序逆転バグに対する単独のHotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従ってソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを用いて、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbDB_InitOrder.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/Application_Initialize.cpp`
*   `Application::Initialize` メソッド内において、`m_thumbnailDatabase.Initialize()` および関連するサムネイル基盤の初期化処理を探す。
*   この初期化処理が、プレイリストの読み込み (`m_playlistManager.LoadFromFile`) や、それに伴う自動再生 (`PlayCurrentTrack()`) よりも **後** に記述されている場合、それらの処理よりも **前** （例えば `m_renderer.Initialize` や `m_audioPlayer.Initialize` の直後）に移動させること。
*   これにより、ドラム初期化（データ注入）が行われる際に、サムネイルデータベースが確実に復元・初期化済みであることを保証する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本タスクでは `Initialize` メソッド内での呼び出し順序の入れ替えのみを行うこと。
