##### 作業指示書 REQ: Hotfix OSD表示の完全網羅とON/OFFオプション化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポートを新規作成（例：`YYYYMMDD_HHMM_RES_Hotfix_OSD_Completeness.md`）し、対応内容を記録すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
現在、主にグローバルホットキー等の画面外操作に限定されているOSD（フライテキスト）の表示を、UIからのマウスクリック操作にも完全対応させる。同時に、OSD表示を無効化できるオプションを追加する。

*   **要件1: OSDのON/OFFオプション追加**
    *   `src/ConfigManager.h` / `.cpp`: `[Layout_OSD]` セクションに `EnableOSD` (デフォルト1) を追加し、読み書きとゲッターを実装する。
    *   `src/Renderer.cpp`: `TriggerFlyText` および `TriggerVolumeOsd` メソッドの冒頭に `if (!m_config->GetEnableOSD()) return;` を追加し、無効時は完全に処理をスキップさせる。
    *   `src/Window.h` / `.cpp`: システムトレイメニューの「詳細設定 (Advanced)」内に「OSD表示 (Show OSD)」を追加し、トグル状態の表示と、クリック時に `m_config->SetEnableOSD(!m_config->GetEnableOSD())` を実行して状態を切り替える処理を実装する。

*   **要件2: ロゴ拡張メニュー操作時のOSDトリガー追加**
    *   `src/Window.cpp` (`WM_LBUTTONDOWN` 内のロゴメニュークリック処理) において、各アクション実行後に `m_renderer.TriggerFlyText` を呼び出す。
    *   ビジュアライザ切替: モードに応じて `L"VISUALIZER: PRISM BEAT"`, `L"VISUALIZER: HALO DUST"`, `L"VISUALIZER: OFF"`
    *   背景表示モード切替: モードに応じて `L"BACKGROUND: NOW PLAYING"`, `L"BACKGROUND: HIDDEN"`, `L"BACKGROUND: DEFAULT"`
    *   プレイリスト左右配置切替: `L"PLAYLIST POS: LEFT"`, `L"PLAYLIST POS: RIGHT"`
    *   リサイズモード切替: `L"RESIZE MODE: ON"`, `L"RESIZE MODE: OFF"`
    *   画面位置固定トグル: `L"WINDOW LOCK: ON"`, `L"WINDOW LOCK: OFF"`

*   **要件3: プレイリストのツールバー操作時のOSDトリガー追加**
    *   `src/Application.cpp` (`SetPlaylistToolbarClickCallback` 内の処理) において、各アクション実行後に `m_renderer.TriggerFlyText` を呼び出す。
    *   ピン留めトグル（※ピン留め処理が行われている適切な箇所に）: `L"PINNED: ON"`, `L"PINNED: OFF"`
    *   曲の削除: `L"TRACK REMOVED"`
    *   全曲削除（クリア）: `L"PLAYLIST CLEARED"`
    *   プレイリストの新規作成: `L"NEW PLAYLIST CREATED"`
    *   プレイリストの削除: `L"PLAYLIST DELETED"`

*   **要件4: 背景アートフレーミングのリセット操作時のOSDトリガー追加**
    *   `src/Application.cpp` (または `Window.cpp` からの該当コールバック内) のフレーミングリセット処理（マウス中ボタンクリック）実行時に、`m_renderer.TriggerFlyText(L"FRAMING RESET")` を呼び出す。
