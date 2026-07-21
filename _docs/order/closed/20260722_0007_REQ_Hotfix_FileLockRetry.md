##### 作業指示書 REQ: ファイルロックリトライ機構の実装 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは、ファイルロック競合による画像・波形抽出失敗を回避するためのリトライ機構の導入である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_FileLockRetry.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/TrackAnalyzer.cpp`, `src/ThumbCacher.cpp`

*   **タスク1: TrackAnalyzer へのファイルロック・リトライ機構**
    *  `TrackAnalyzer.cpp` の `ParseThreadFunc` 内において、ファイルを読み込む処理で例外やエラーが発生した場合、直ちに諦めるのではなく `Sleep(50)` を挟み、最大10回（計500ms）リトライするループを組み込む。

*   **タスク2: ThumbCacher へのファイルロック・リトライ機構**
    *  `ThumbCacher.cpp` の `WorkerLoop` 内において、`FileManager::ExtractAlbumArtBinary` 等の読み込み処理が失敗した場合も同様に `Sleep(50)` を挟んで最大10回リトライするループを組み込み、それでもダメな場合のみ発番を取り消す（`m_db->RollbackThumbId`）ように堅牢化する。

#### 【絶対遵守ルール (Constraints)】
*   **メインスレッドの無停止**: リトライのための `Sleep` 処理は、必ず裏スレッドのワーカー（`TrackAnalyzer` および `ThumbCacher`）内部でのみ実行すること。絶対にメインスレッドをブロックしてはならない。
*   **単一責務の厳守**: 今回はリトライ機構の導入のみを行う。UIの描画やフェード等の変更は一切行わないこと。
こ