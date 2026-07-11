### 作業指示書 REQ: グランドフィナーレ前の不具合一掃 (Hotfix 2: プレイリスト切り替えバグの修正)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260707_2345_RES_Phase18-12_ArtFraming.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前の計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（`D:\ozlab\oztone\_docs\RES(Hotfix)_template.md`）を元に、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHSS_RES_Hotfix_RES_Phase18-12_ArtFraming.md`）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
*   **不具合の原因**:
    昨晩の実装にて、`Application::SwitchPlaylist` および `Application::CreateNewPlaylist` 内で `m_playlistManager.SaveToFile()` を呼び出すタイミングが「新しいパスへ切り替わった後」になってしまったため、切り替え前のプレイリスト情報が新しいファイルに上書き保存されるバグが発生している。また、AIの修正ミスにより `m_playlistManager.Clear()` の呼び出しが欠落し、リストの追加読み込み（追記）が発生している。
*   **修正内容 (src/Application.cpp)**:
    1.  **Application::SwitchPlaylist**:
        メソッドの冒頭（`m_config->SetDefaultPlaylistPath` でパスを変更する **前**）に、現在のパス `m_config->GetDefaultPlaylistPath()` を取得し、それに対して `m_playlistManager.SaveToFile(oldPath)` を呼び出して現在の状態を安全に保存すること。
        その後、パスを新しい `filepath` に更新し、**必ず `m_playlistManager.Clear()` を呼び出してメモリ上のリストを空にしてから**、`m_playlistManager.LoadFromFile(filepath)` を実行する正しい順序に直すこと。
    2.  **Application::CreateNewPlaylist**:
        ここでも同様に、新しいファイル名を生成して `SetDefaultPlaylistPath` を呼び出す **前** に、`m_playlistManager.SaveToFile(m_config->GetDefaultPlaylistPath())` を呼び出して現在の状態を保存すること。
    3.  **Application::ClearPlaylist**:
        このメソッドの役割は「現在のリストを空にして保存する」ことであるため、既存の `m_playlistManager.Clear()` の直後に `m_playlistManager.SaveToFile(m_config->GetDefaultPlaylistPath())` が呼ばれる正しい順序を維持（または修正）すること。
