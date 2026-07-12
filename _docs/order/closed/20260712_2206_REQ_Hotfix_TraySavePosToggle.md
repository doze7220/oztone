### 作業指示書 REQ: Hotfix トレイメニューの位置記憶トグル修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの調査および修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_TraySavePosToggle.md）を新規作成し、原因と対応内容を追記すること。
3. チャットにて「Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
システムトレイメニューの「終了時にサイズと位置を記憶する」設定において、チェックマークの表示状態およびトグル操作が正常に機能しないバグを修正する。

*   **要件1: メニュー構築時のチェック状態同期 (`WM_TRAYICON`)**
    *   `src/Window_TrayMenu.cpp` 内のメニュー生成処理（`HandleTrayIcon` 等）を確認する。
    *   対象メニュー項目（`ID_TRAY_SAVE_POS`）をメニューに追加する際、`m_config->GetSavePositionOnExit()` の戻り値が `true` なら `MF_CHECKED` を、`false` なら `MF_UNCHECKED` を設定するロジックが存在するか確認し、なければ追加・修正すること。
*   **要件2: クリック時のトグル処理の確保 (`WM_COMMAND`)**
    *   `src/Window_TrayMenu.cpp` 内のコマンド処理（`HandleCommand` 等）を確認する。
    *   `ID_TRAY_SAVE_POS` が選択された際の分岐（`case ID_TRAY_SAVE_POS:`）が存在するか確認し、存在しない場合は以下を実装すること。
        *   `bool currentState = m_config->GetSavePositionOnExit();`
        *   `m_config->SetSavePositionOnExit(!currentState);`

#### 【絶対遵守ルール (Constraints)】
*   機能追加は行わず、対象バグの修正のみに留めること。
