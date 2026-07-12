### 作業指示書 REQ: Hotfix 終了時のプレイリストパスINI保存漏れ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの調査および修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_SavePlaylistOnExit.md）を新規作成し、原因と対応内容を追記すること。
3. チャットにて「Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
アプリケーション終了時に、現在アクティブなプレイリストのパスが INI ファイル（OZtone.ini の DefaultPlaylistPath）に保存されず、次回起動時に復元されない不具合を修正する。

*   **要件1: アプリケーション終了時のパス保存処理の追加**
    *   `Application` クラスのデストラクタ (`Application::~Application()`)、または終了処理が行われる適切な箇所を確認する。
    *   終了処理のシーケンス（各マネージャの破棄・保存の前）において、`m_config.SetDefaultPlaylistPath(m_config.GetDefaultPlaylistPath());` のような呼び出し、あるいは明示的に `ConfigManager` に現在のプレイリストパスを INI の `[Playlist]` セクションへ保存（`WritePrivateProfileStringW` 相当）させる処理を追加する。
    *   （※もし `ConfigManager` の `SetDefaultPlaylistPath` が INI への保存ロジックを内包していない場合は、保存するよう `ConfigManager_Playlist.cpp` 等を修正すること。）

#### 【絶対遵守ルール (Constraints)】
*   機能追加は行わず、対象バグ（終了時のINI保存漏れ）の修正のみに留めること。
