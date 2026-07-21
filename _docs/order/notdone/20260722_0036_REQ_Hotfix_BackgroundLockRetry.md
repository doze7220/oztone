##### 作業指示書 REQ: BackgroundManager へのファイルロックリトライ機構の実装 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、ファイルロック競合による背景アート抽出失敗を回避するため、BackgroundManager へリトライ機構を導入する Hotfix である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_BackgroundLockRetry.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/BackgroundManager.cpp`
*   **タスク1: BackgroundManager へのファイルロック・リトライ機構の実装**
    *  `BackgroundManager::WorkerLoop` 内において、`FileManager::ExtractAlbumArtBinary` を呼び出して背景用画像バイナリを抽出する処理を改修する。
    *  抽出に失敗した場合（空のバイナリが返ってきた、あるいは例外が発生した場合）、直ちに諦めてプレースホルダ等の処理へ移行するのではなく、`Sleep(50)` を挟んで最大10回（計500ms）リトライするループ構造（`while` または `for`）を組み込むこと。
    *  リトライ上限に達してもバイナリが抽出できなかった場合のみ、既存の失敗時ロジック（プレースホルダ表示用画像のロードなど）へフォールバックするように堅牢化すること。

#### 【絶対遵守ルール (Constraints)】
*   **メインスレッドの無停止**: リトライのための `Sleep` 処理は、必ず `BackgroundManager` の裏スレッド（ワーカー内部）でのみ実行すること。絶対にメインスレッドをブロックしてはならない。