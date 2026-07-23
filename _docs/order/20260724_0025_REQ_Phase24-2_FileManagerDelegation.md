##### 作業指示書 REQ: Phase 24-2: ConfigManagerのファイルI/O責務移管 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase24-2_FileManagerDelegation.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
ConfigManagerに残存しているファイルシステム操作のロジックを、本来の責務を持つFileManagerへ移管し、ConfigManagerを純粋な設定コンテナへ純化する。

* **要件1: ConfigManagerからのファイルI/O処理の削除**
  * `ConfigManager` に残存しているディレクトリ走査（`GetAvailablePlaylists`）およびスナップショット監視（`CheckPlaylistSnapshotChanged`とメンバ変数）の宣言と実装を削除する。
* **要件2: FileManagerへの処理移管**
  * 削除したディレクトリ内のプレイリストファイルの一覧取得処理と、スナップショットによる更新検知処理を `FileManager` に新規の静的メソッド（または適切な実装）として移植する。
* **要件3: Application層の配線修正**
  * `Application` クラス（`Application_Playlist.cpp` や `Application_Render.cpp` など）の呼び出し元を修正し、`ConfigManager` ではなく `FileManager` からプレイリストの一覧や更新状態を取得するように結線し直す。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase24-2_FileManagerDelegation.md）に、詳細作業内容を記載する（タスクリストに含める）こと。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
* **スコープの厳守**: `GetAvailablePlaylists` と `CheckPlaylistSnapshotChanged` の移管とそれに伴う呼び出し元の修正のみに留めること。
