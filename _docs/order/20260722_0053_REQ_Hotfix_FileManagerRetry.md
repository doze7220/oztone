##### 作業指示書 REQ: FileManagerへのI/Oリトライ機構の一元化 (Hotfix 3/3)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは、ファイルI/Oの統括クラスである FileManager へファイルロック競合時のリトライ機構を仕込む、本命のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_FileManagerRetry.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/FileManager.cpp`

*   **タスク1: メタデータ抽出処理へのリトライ機構の実装**
    *  `FileManager::ExtractTextMetadata` 内において、`TagLib::MPEG::File` 等を用いてファイルを開く処理を行う際、ファイルロック競合によりオープンに失敗した（例: `isValid()` が false になった、または例外が発生した）場合、直ちに諦めるのではなく、`Sleep(50)` を挟んで最大10回（計500ms）リトライするループ構造を実装する。
    *  10回リトライしてもダメだった場合のみ、フォールバック（ファイル名からのタイトル抽出など）の既存の失敗時処理へ移行する。

*   **タスク2: アルバムアート抽出処理へのリトライ機構の実装**
    *  `FileManager::ExtractAlbumArtBinary` 内においても同様に、`TagLib::MPEG::File` 等のオープン処理で失敗（`isValid() == false` または例外）した場合に `Sleep(50)` を挟んで最大10回リトライするループを実装する。
    *  リトライ上限に達した場合のみ、空のバイナリを返すなど既存の失敗時処理へ移行する。

#### 【絶対遵守ルール (Constraints)】
*   **完全なカプセル化**: リトライロジックは必ず `FileManager.cpp` の内部に閉じ込めること。外部の呼び出し元（`TrackAnalyzer` や `ThumbCacher`、`BackgroundManager`）に対しては、一切このリトライの事情を意識させない（純粋にパスを渡せば結果が返ってくるだけの状態を維持する）こと。
