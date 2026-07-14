### 作業指示書 REQ: Phase 21-3: プレイリスト一覧復帰時のフォーカス復元 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-3_PlaylistFocusRestore.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
プレイリストのUIにおいて、曲一覧モードから「上の階層へ（プレイリスト一覧）」へ戻った際、現在再生中（アクティブ）のプレイリストが自動的に「選択状態（フォーカス）」となるように改修し、そのままリスト削除などの操作をスムーズに行えるようにする。

*   **要件1: フォーカス復元ロジックの追加 (Application_Playlist.cpp)**
    *   `src/Application_Playlist.cpp` 内の `OnPlaylistToolbarClick` メソッドを改修する。
    *   `!m_isPlaylistListViewMode` (曲一覧モード) かつ `buttonIndex == 0` (上の階層へ戻る) の処理ブロックにおいて、単に `m_isPlaylistListViewMode = true;` とするだけでなく、フォーカス状態 (`m_focusedPlaylistIndex`) を復元するロジックを追加する。
*   **要件2: 現在のプレイリストのインデックス検索**
    *   `m_config.GetDefaultPlaylistPath()` を用いて現在のプレイリストのパスを取得する。
    *   `m_config.GetAvailablePlaylists()` を呼び出してプレイリストの一覧を取得する。
    *   一覧の各パス（またはファイル名）と現在のパスを比較し、一致する要素のインデックスを特定する。
    *   特定したインデックスを `m_focusedPlaylistIndex` に設定する（見つからない場合は既存のデフォルト動作とするか、0等にフォールバックする）。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の極小化**: 本タスクは `Application_Playlist.cpp` 内のモード遷移時のフォーカス設定のみを追加するものであり、プレイリストの描画ロジック（`Widget_Playlist.cpp`）やリストの削除ロジック等、他の機能は一切破壊しないこと。
-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 21-3: プレイリスト一覧復帰時のフォーカス復元 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_2133_RES_Phase21-3_PlaylistFocusRestore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 21-3の「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260714_2133_RES_Phase21-3_PlaylistFocusRestore.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「Phase 21-3の実装がすべて完了しました！ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: Application_Playlist.cppのフォーカス復元ロジック追加**
    *   `src/Application_Playlist.cpp` の `Application::OnPlaylistToolbarClick` メソッドを改修する。
    *   `!m_isPlaylistListViewMode` かつ `buttonIndex == 0` （曲一覧からプレイリスト一覧へ戻る）のブロックにおいて、以下の処理を実装する。
        1. `m_config.GetDefaultPlaylistPath()` を用いて現在のプレイリストパスを取得する。
        2. `m_config.GetAvailablePlaylists()` を用いて利用可能なプレイリスト一覧を取得する。
        3. 一覧をループまたは `std::find` 等で検索し、現在のパスと一致する（またはファイル名が一致する）要素のインデックスを特定する。
        4. 一致するものが見つかった場合、そのインデックスを `m_focusedPlaylistIndex` に代入する（見つからなければ既存のフォーカスのまま、あるいは0等にする）。
        5. 最後に `m_isPlaylistListViewMode = true;` を設定してモードを切り替える（既存処理の維持）。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の極小化**: 本タスクは `Application_Playlist.cpp` 内のモード遷移時のフォーカス設定のみを追加するものであり、プレイリストの描画ロジック（`Widget_Playlist.cpp`）やリストの削除ロジック等、他の機能は一切破壊しないこと。
