##### 作業指示書 REQ: サムネイル抽出のリトライ機構パージ (Hotfix 1/3)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、サムネイル工場（ThumbCacher）に一時的に組み込まれたファイルロックリトライ機構の完全削除（パージ）である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbRetryPurge.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/ThumbCacher.cpp`

*   **タスク1: ThumbCacherからのファイルロック・リトライ機構の削除**
    *  `ThumbCacher.cpp` の `WorkerLoop` 内において、`FileManager::ExtractAlbumArtBinary` による画像バイナリ抽出処理を囲んでいた一時的なリトライループ（最大10回などの `for` や `while` ループ）および `Sleep(50)` の待機処理を完全に削除する。
    *  リトライなしの1回勝負で `ExtractAlbumArtBinary` を呼び出す、本来の純粋なロジックへ復元する。
    *  抽出に失敗した場合（空バイナリが返るなど）は、直ちに発番を取り消す（`m_db->RollbackThumbId`）既存のフェイルセーフ処理へ移行する状態を維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **単一責務の厳守**: 今回は `ThumbCacher` から泥臭いリトライロジックを削ぎ落とすことのみを目的とする。抽出後のWICデコードやデータベースへの保存処理など、その他のロジックは一切変更しないこと。
