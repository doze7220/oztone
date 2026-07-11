##### 作業指示書 REQ: Hotfix 背景クリックによるUIの能動的フェードアウト (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポートを新規作成（例：`YYYYMMDD_HHMM_RES_Hotfix_ActiveFadeOut.md`）し、対応内容を記録すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
UI要素が何もない背景部分を左クリック、または右クリックした際に、ホバー展開中（離脱ディレイ中）のUI（プレイリスト、ロゴ拡張メニュー、下部コントロール等）の格納アニメーションを即座に開始させる。

*   **対象ファイル:** `src/Renderer.h` / `src/Renderer.cpp`
    *   **修正内容:** `void ForceClearHoverDelays();` メソッドを追加し、内部で展開ディレイ用のタイマー変数（例: `m_logoMenuLeaveTimer`, `m_playlistLeaveTimer`, `m_controlLeaveTimer` 等）を強制的に `0.0f` にリセットする処理を実装する。

*   **対象ファイル:** `src/Window.h` / `src/Window.cpp`
    *   **修正内容:**
        *   背景クリックを通知するためのコールバック（例: `std::function<void()> m_onBackgroundClickCallback;`）と、セッター `SetBackgroundClickCallback` を追加する。
        *   `WM_RBUTTONDOWN` 処理内（右ドラッグフレーミングの起点部分）にて、このコールバックを呼び出す処理を追加する。
        *   `WM_LBUTTONDOWN` 処理内にて、各UI要素のヒットテスト（プレイリスト、ロゴメニュー等）をすべてすり抜け、背景全体でのウィンドウドラッグ移動（`HTCAPTION` 送信）を開始する直前に、同様にこのコールバックを呼び出す処理を追加する。

*   **対象ファイル:** `src/Application.cpp`
    *   **修正内容:**
        *   `Application::Initialize` 内にて `m_window.SetBackgroundClickCallback` を登録し、ラムダ式の中で `m_renderer.ForceClearHoverDelays();` を実行するように連携処理を記述する。
