##### 作業指示書 REQ: PlayCurrentTrackのロック優先度最適化 (Hotfix)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、ファイルロック競合による初回再生スキップバグのHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_PlayCurrentTrackPriority.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Application_Playback.cpp`

*   **タスク1: `Application::PlayCurrentTrack` 内の処理順序の適正化**
    *  関数の先頭付近で行われている `m_thumbnailManager.GetOrGenerateThumbId` に伴うサムネイル関連の情報構築・発注処理、および `m_backgroundManager.Load` による背景画像ロード発注の処理ブロックを、**`m_audioManager.Play(path)` の呼び出しの「直後」** に移動する。
    *  これにより、メインスレッドの音声再生エンジンが確実に一番最初にファイルロックを取得し、後から発動する非同期ワーカーたちが `FileManager` のリトライクッションで安全に待機する正しいロック優先順位を確立する。
    *  ※TrackMetadata の自己修復（`UpdateTrackMetadataIfNeeded`）はメインスレッドの同期処理でありファイルをすぐ閉じるため、`Play` の前に残しておいても問題ない（既存の位置で可）。

#### 【絶対遵守ルール (Constraints)】
*   **処理内容の維持**: あくまでメソッド内の呼び出し順序を入れ替えるのみとし、他のロジックや変数の操作は一切変更しないこと。
