### 作業指示書 REQ: Hotfix プレイリストのトラックナンバー本来表示化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_Playlist_OriginalTrackNo.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「プレイリストのトラックナンバー本来表示化が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
シャッフルモード時においても、プレイリストに表示されるトラックナンバー（CD帯）を「現在のキュー順（i+1）」ではなく「プレイリストに登録された本来の行番号」で表示するように改修する。

*   **要件1: PlaylistManager へのゲッター追加**
    *   `src/PlaylistManager.h` および `src/PlaylistManager.cpp` を確認し、現在のシャッフル順序配列 `m_shuffleIndices` を取得するメソッド `const std::vector<size_t>& GetShuffleIndices() const;` が無ければ追加する。

*   **要件2: Renderer および WidgetContext へのデータ伝達ルート構築**
    *   `src/WidgetContext.h` の `WidgetContext` に `std::vector<size_t> shuffleIndices;` を追加する。
    *   `src/Renderer.h` にメンバ変数 `std::vector<size_t> m_shuffleIndices;` とセッター `void SetShuffleIndices(const std::vector<size_t>& indices);` を追加する。
    *   `src/Renderer_Setter.cpp` に `SetShuffleIndices` の実装を追加する。
    *   `src/Renderer_Context.cpp` 内でコンテキストを構築する際（`BuildRenderContext` 等）、`ctx.shuffleIndices = m_shuffleIndices;` を代入するよう修正する。

*   **要件3: Application からのデータ同期**
    *   `src/Application_Render.cpp` の `ForceRender`（または描画前処理ループ）にて、`m_renderer.Render` 等の描画・更新処理を呼び出す前に、`m_renderer.SetShuffleIndices(m_playlistManager.GetShuffleIndices());` を呼び出して最新のインデックス配列を同期する。

*   **要件4: Widget_Playlist_DrawItems での描画ロジック改修**
    *   `src/Widget_Playlist_DrawItems.cpp` 内の曲一覧描画ループ（TrackListモード）において、CD帯のトラックナンバー文字列を生成している箇所（`i + 1` などをフォーマットしている箇所）を改修する。
    *   `ctx.shuffleIndices` の要素数が十分にあることを確認した上で、`ctx.shuffleIndices[i] + 1` を本来のナンバーとして表示するように変更する。配列外アクセスの安全対策として `ctx.shuffleIndices.size() > i` のガードを入れておくこと。

#### 【絶対遵守ルール (Constraints)】
*   **Rendererの状態非保持原則**: Widget は Renderer を直接参照してはならない。必ず `WidgetContext` を経由してデータを取得すること。
*   **ファイル分割の厳守**: Renderer や Application が実装対象別に物理分割されていることを前提とし、必ず適切な .cpp ファイル（`Renderer_Setter.cpp`、`Application_Render.cpp` 等）に対して修正を行うこと。
