### 作業指示書 REQ: Hotfix プレイリストインデックスのスレッドセーフ化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_Playlist_IndicesThreadSafe.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「スレッドセーフな値返しへの修正が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
マルチスレッド環境下におけるスレッド競合とヒープ破壊によるハングを防ぐため、`PlaylistManager` からシャッフルインデックスを取得するメソッドを「参照返し」から「値返し（コピー）」へと修正する。

*   **要件1: PlaylistManager のゲッターのシグネチャ変更**
    *   `src/PlaylistManager.h` 内の `GetShuffleIndices` メソッドの宣言を、`const std::vector<size_t>& GetShuffleIndices() const;` から `std::vector<size_t> GetShuffleIndices() const;` （値返し）に変更する。
    *   `src/PlaylistManager.cpp` 内の実装も同様に戻り値を変更し、内部で `std::lock_guard<std::mutex> lock(m_mutex);` によるロックを取得した状態で `return m_shuffleIndices;` と返すようにする。これにより、ロックの保護下で安全にコンテナのコピーが生成される。

#### 【絶対遵守ルール (Constraints)】
*   **Renderer側のシグネチャ維持**: `Renderer` 側のセッター `SetShuffleIndices(const std::vector<size_t>& indices)` のシグネチャは変更しないこと。値返しされた一時オブジェクトが const 参照として安全にバインドされるため、呼び出し側のコード（`SetShuffleIndices(m_playlistManager.GetShuffleIndices())`）は一切変更せずとも合法に動作する。
