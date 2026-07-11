##### 作業指示書 REQ: Hotfix 初回起動時のウィンドウ中央配置 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポートを新規作成（例：`YYYYMMDD_HHMM_RES_Hotfix_InitialWindowPosition.md`）し、対応内容を記録すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
INIファイルが存在しない初回起動時において、ウィンドウがOS任せの配置（`CW_USEDEFAULT`）となり画面外や不自然な位置へ表示される問題を防ぐため、以下の対応を行う。

*   **対象ファイル:** `src/Window.cpp`
*   **修正内容:**
    *   `Window::Initialize` メソッド内にて、`m_config` から取得しDPIスケーリングを適用した後の `x` および `y` （または元の設定値）が `CW_USEDEFAULT` と等しいかを判定するロジックを追加する。
    *   `x == CW_USEDEFAULT || y == CW_USEDEFAULT` の場合、`GetSystemMetrics(SM_CXSCREEN)` と `GetSystemMetrics(SM_CYSCREEN)` を用いてプライマリモニタの解像度を取得する。
    *   取得したモニタ解像度と、算出済みの物理ピクセルサイズ (`width`, `height`) を用いて、ウィンドウが画面中央に配置されるように `x` と `y` を計算して上書きする。
        （計算例： `x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;` `y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;`）
    *   この座標の上書き処理を、`CreateWindowExW` を呼び出してウィンドウを実際に生成する直前に行うこと。
