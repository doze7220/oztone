##### 作業指示書 REQ: サムネイル発注のゾンビ化解消（ロールバック＆自律リトライ機能） (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトはサムネイル発注のゾンビ化解消（ロールバック＆自律リトライ機能）のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbRollback.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/ThumbnailDatabase.h`, `src/ThumbnailDatabase.cpp`, `src/ThumbCacher.cpp`
*  ファイルロック競合等の理由で `ThumbCacher` がサムネイルの抽出・保存（`StoreCookedData`）を完遂できなかった場合、メモリ上に発番情報だけが残り二度とエンキューされなくなるバグを修正する。
*   **タスク1: ロールバックメソッドの追加**
    *  `ThumbnailDatabase.h` および `cpp` に `void RollbackThumbId(const std::wstring& filepath)` を追加する。
    *  内部で `std::lock_guard<std::mutex> lock(m_mutex);` で保護し、メモリ上の辞書 `m_pathToId` から該当の `filepath` のエントリを削除（`erase`）する処理を実装する。これにより該当パスは未発番状態へ戻る。
*   **タスク2: 例外・失敗時のロールバック呼び出し**
    *  `ThumbCacher.cpp` の `WorkerLoop` において、画像抽出およびWICデコード処理を囲んでいる `try-catch` ブロックの `catch` セクションにて、`m_db->RollbackThumbId(filepath);` を呼び出す処理を追加する。
    *  さらに、`ExtractAlbumArtBinary` で取得したバイナリが空だったなど、正当な理由なく `m_db->StoreCookedData` に到達せずにループを `continue` または抜けるようなパスが存在する場合は、同様に `RollbackThumbId` を呼んで発番を取り消すこと。（※もし「画像無し」として空データを意図的に `StoreCookedData` へ渡して永続化している仕様であれば、それは正常終了とみなす。あくまで「途中で処理を放棄した」場合のみロールバックする）

#### 【絶対遵守ルール (Constraints)】
*   **状態の整合性厳守** : `ThumbCacher` がタスクを正常に完遂できなかった場合は必ず `RollbackThumbId` を呼び、データベースの「発番済み」状態と「実際にクックされたか」の状態の乖離を絶対に防ぐこと。これによりUI側からの自律的なリトライループが完成する。
