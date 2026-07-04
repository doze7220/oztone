### REQ: Phase 12-3: ロゴ拡張メニューへの「プレイリスト左右配置トグル」の追加 (計画立案)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase12-3_LogoMenuPlaylistPos.md`）として新規作成すること。
3. 作業計画を細かなタスクリストに分割し、レポート末尾に記載すること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
本タスクの目的は、左上ロゴホバー時に展開する「ロゴ拡張メニュー」の第6のボタンとして、「プレイリストの左右配置（PlaylistPosition）」を動的に切り替えるトグルボタンを追加することである。

* **要件1: メニュー項目（配列）への追加 (Window)**
  * `Window::Window` コンストラクタ内の `m_logoMenuItems` 配列の末尾に、6つ目のボタンを追加する。
  * アイコンには左右矢印（例: `↔️` や `⇄`）のUNICODE絵文字を使用する。
  * 表示テキストは`プレイリストの配置場所切り替え`とする。
* **要件2: ホバー説明テキストの動的表示 (LogoMenuWidget)**
  * 追加した6番目のアイコンにホバーした際、メニュー上部の説明テキストが現在の設定値（`ConfigManager::GetPlaylistPosition()`）に応じて動的に切り替わるようにする。
  * 左配置（0）の場合: `PLAYLIST POS: LEFT`
  * 右配置（1）の場合: `PLAYLIST POS: RIGHT`
* **要件3: クリック時の機能実行とINI保存 (Window / ConfigManager)**
  * `Window::WindowProc` のロゴメニュークリック処理（`WM_LBUTTONDOWN`）にて、追加したボタンのクリックを検知する。
  * クリックされた場合、現在の `PlaylistPosition` の値（0または1）を反転させ、`ConfigManager::SetPlaylistPosition`（新設）を通じて即座にINIファイルへ保存・適用する処理を実装する。
  * ※既存の `ConfigManager` には `PlaylistPosition` のゲッターしか存在しない場合があるため、セッター（`SetPlaylistPosition`）と `WritePrivateProfileStringW` による保存処理の追加も行うこと。

#### 【絶対遵守ルール (Constraints)】
* **アーキテクチャの厳守**: `LogoMenuItem` の配列管理という既存の拡張しやすい仕組みを活かすこと。描画処理は `LogoMenuWidget`、状態変更と保存は `Window` と `ConfigManager` という責務分離を絶対に崩さないこと。
