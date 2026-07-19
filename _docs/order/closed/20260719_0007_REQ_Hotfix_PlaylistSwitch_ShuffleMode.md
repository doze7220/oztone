##### 作業指示書 REQ: プレイリスト切り替え時のシャッフル状態依存化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_PlaylistSwitch_ShuffleMode.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: プレイリスト切り替え時のキュー再構築の修正**
    *   `src/Application_Playlist.cpp` の `SwitchPlaylist` 等において、新しいプレイリストをロード（`LoadFromFile`等）した直後、曲の再生を開始する前に `m_playlistManager.RebuildQueue(m_config.GetShuffleMode());` を呼び出す処理を追加・修正する。
    *   これにより、プレイリスト切り替え時に現在のシャッフルモード（ON/OFF）に従って、ランダムまたは連番のキューが正しく再構築されるようにする。
*   **要件2: 初期化時のキュー再構築の確認と修正**
    *   `src/Application_Initialize.cpp` などの起動時プレイリスト読み込み箇所においても、同様に `RebuildQueue(m_config.GetShuffleMode())` が適用されるように修正する。
    *   `PlaylistManager::LoadFromFile` 内部に過去の強制シャッフルロジックが残存している場合はそれを削除し、明示的に `RebuildQueue` を呼び出して制御する設計へと純化させる。

#### 【絶対遵守ルール (Constraints)】
*   **状態の尊重**: キューの再構築は必ず `ConfigManager` が保持している現在の `ShuffleMode` を尊重し、ハードコードで `true` などを渡してはならない。
