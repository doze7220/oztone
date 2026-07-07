### 作業指示書 REQ: Phase 18-3: システムトレイの再構築
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは  **絶対にコードの修正（ファイルの書き換え）を行わない**  こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-3_SystemTrayRestructuring.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------
#### 【実装要件】
システムトレイの右クリックメニューを大掃除し、再生コントロールの追加、およびシステム設定系のネスト化（詳細設定化）を行って、安全でストイックな構成へと再構築する。

*   **要件1: 不要なメニューIDの削除と大掃除 (`src/Window.h/cpp`)**
    *  他のUIへ移行済みの以下の機能をシステムトレイメニューの構築およびコマンドハンドラから完全に削除する。
    *  `ID_TRAY_CLEAR_PLAYLIST`, `ID_TRAY_NEW_PLAYLIST`
    *  背景モード選択系 (`ID_TRAY_BG_NOWPLAYING` 等)
    *  ビジュアライザ選択系 (`ID_TRAY_VIS_PRISM` 等)
    *  `ID_TRAY_ENABLE_RESIZE`, `ID_TRAY_LOCK_WINDOW_POS`

*   **要件2: 再生コントロールの追加 (`src/Window.h/cpp`, `src/Application.cpp`)**
    *  メニューの最上部に以下の再生コントロールを追加する。
    *  `再生/一時停止` (`AudioPlayer::TogglePlayPause` と連動)
    *  `次の曲へ`, `前の曲へ` (`PlaylistManager` と連動した既存のスキップ処理)
    *  `音量` (サブメニューとし、100%, 75%, 50%, 25% の項目を用意。選択時に `AudioPlayer::SetVolume` および `ConfigManager::SetDefaultVolume` へ反映する)

*   **要件3: 「詳細設定」サブメニューの構築 (`src/Window.cpp`)**
    *  新たに「詳細設定」というサブメニューを作成し、既存のシステム系コマンドをこの中にネストして整理する。
    *  格納する項目: `画面の表示順序` (既存のZ-Orderサブメニューごとネスト), `終了時の位置・サイズを記憶`, `位置とサイズをリセット`, `設定を初期化`。

*   **要件4: 安全な設定初期化（バックアップ機能） (`src/ConfigManager.cpp`)**
    *  「設定を初期化」実行時に呼び出される `ConfigManager::SaveDefaultSettings()` （または専用の新メソッド）を改修する。
    *  現在の `OZtone.ini` が存在する場合、上書きする前に `OZtone_YYYYMMDD_HHMM.ini` (または `HHMMSS`) の形式でリネーム（またはコピー）してバックアップファイルを作成する安全装置を実装する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-3_SystemTrayRestructuring.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。
#### 【絶対遵守ルール (Constraints)】
*   **メモリリークの防止** : サブメニューを動的生成・アタッチする際は、親メニュー破棄時の再帰破棄を前提とするなどしてリソースリークを防ぐこと。
*   **スコープの厳守** : 既存のプレイリスト機能や、他のUIで使われている設定ロジックそのものを削除してしまわないよう、Windowのメニューハンドリング部のみを掃除すること。
