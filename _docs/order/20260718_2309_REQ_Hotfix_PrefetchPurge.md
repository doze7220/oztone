##### 作業指示書 REQ: 先読み機能（Prefetch）の完全パージ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_PrefetchPurge.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: Application層からの先読みメソッドと変数のパージ**
    *   `src/Application.h` から `PrefetchNextTrack()` の宣言、および関連変数（`m_prefetchThread`, `m_isPrefetchReady`, `m_prefetchedAlbumArt`, `m_prefetchedTitle`, `m_prefetchedArtist` 等）をすべて削除する。
    *   `src/Application_Playback.cpp` から `PrefetchNextTrack()` の実装を完全に削除する。
*   **要件2: 呼び出し元からの先読みトリガー・待機処理の削除**
    *   `src/Application_Render.cpp` の `Run` ループ内にある、「`m_isPrefetchReady` が true になるまで待機する（`continue`でスキップする）」ガード処理を完全に削除する。
    *   `PlayCurrentTrack` 内部や、ジャンプ時、プレイリスト追加時などの各所に残存している `PrefetchNextTrack()` の呼び出しをすべて削除する。
*   **要件3: 画像ロードのフリップ後オンデマンドへの一本化**
    *   先読みによる画像の事前キャッシュ処理を廃止し、曲が切り替わってフリップサイクルが回った直後に、「新しいカレントスロット」に対してのみ非同期で画像をロードする現在の正しいフローに完全に依存させる。

#### 【絶対遵守ルール (Constraints)】
*   **残骸の完全削除**: コメントアウトで残すのではなく、先読み機能に関する変数やメソッドは跡形もなく物理削除すること。
