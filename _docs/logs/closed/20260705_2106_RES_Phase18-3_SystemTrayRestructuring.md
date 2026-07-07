# RES:実装計画・作業レポート Phase 18-3: システムトレイの再構築

## 1. 実装目的
システムトレイの右クリックメニューを大掃除し、再生コントロールの追加、およびシステム設定系のネスト化（詳細設定化）を行って、安全でストイックな構成へと再構築する。

## 2. アーキテクチャ設計
### 要件1: 不要なメニューIDの削除と大掃除
    - src/Window.h, src/Window.cpp内から、他のUIへ移行済みのシステムトレイメニュー機能（プレイリストクリア・新規作成、背景モード選択、ビジュアライザ選択、リサイズ有効化、位置ロック）のIDおよび関連するメニュー構築・コマンドハンドリング処理を完全に削除する。

### 要件2: 再生コントロールの追加
    - src/Window.h, src/Window.cppに、新たなメニューID（再生/一時停止、前の曲へ、次の曲へ、音量設定：100%, 75%, 50%, 25%）を定義する。
    - トレイメニューの最上部にこれらのコントロール項目を追加する。音量はサブメニューとしてネストする。
    - src/Application.cpp（またはWindowからのイベントコールバック経由）でこれらのコマンドを受け取り、AudioPlayerやPlaylistManagerと連動して再生操作や音量設定(`ConfigManager::SetDefaultVolume`および`AudioPlayer::SetVolume`)を行うように実装する。

### 要件3: 「詳細設定」サブメニューの構築
    - src/Window.cppにて、トレイメニューに新たに「詳細設定」サブメニューを作成する。
    - 既存の「画面の表示順序（Z-Order）」、「終了時の位置・サイズを記憶」、「位置とサイズをリセット」、「設定を初期化」をこのサブメニュー内に格納する。
    - サブメニュー生成時のメモリリークを防ぐため、親メニュー破棄時の再帰破棄を前提とした設計（または適切な破棄処理）とする。

### 要件4: 安全な設定初期化（バックアップ機能）
    - src/ConfigManager.cppの「設定を初期化」処理において、既存の`OZtone.ini`が存在する場合、上書きする前に日時ベースのファイル名（例：`OZtone_YYYYMMDD_HHMM.ini` または `HHMMSS`）へコピー・リネームしてバックアップを生成するロジックを追加する。

## 3. 実装タスクリスト
- [x] タスク1: 不要なメニューIDの削除 (`src/Window.h`, `src/Window.cpp`)
    - `ID_TRAY_CLEAR_PLAYLIST`, `ID_TRAY_NEW_PLAYLIST`, 背景・ビジュアライザ選択系, リサイズ・位置ロック系のマクロ・定数を削除。
    - メニュー構築コードおよび `WM_COMMAND` 処理から該当項目を削除。
- [x] タスク2: 再生コントロールと音量サブメニューの追加 (`src/Window.h`, `src/Window.cpp`, `src/Application.cpp`)
    - メニューID（再生/一時停止、前/次の曲、音量4段階）の追加。
    - メニュー構築処理を修正し、最上部に項目を追加。
    - `WM_COMMAND`でのイベント検知と `Application` クラス側への連携実装。
- [x] タスク3: 「詳細設定」サブメニューの構築 (`src/Window.cpp`)
    - 「詳細設定」用のサブメニューを作成し、表示順序、位置サイズ記憶、リセット、初期化の各項目をネスト。
- [x] タスク4: 設定初期化時のバックアップ機能実装 (`src/ConfigManager.cpp`)
    - `ConfigManager::SaveDefaultSettings` 等を修正し、`std::filesystem` を用いて既存の `OZtone.ini` をバックアップ保存してから初期化を行う安全装置を実装。
- [x] タスク5: アーキテクチャドキュメントの更新 (`PROJECT_ARCHITECTURE.md`)
    - メニュー構成の変更、バックアップ機能の追加についてドキュメントを更新する。

## 4. 詳細作業内容
### タスク1: 不要なメニューIDの削除
    - `Window.h` から `ID_TRAY_CLEAR_PLAYLIST`、`ID_TRAY_NEW_PLAYLIST`、背景・ビジュアライザ選択、リサイズ、ロック用IDを削除。
    - `Window.cpp` の `TRAY_MENU_ORDER` および `WM_COMMAND` から対象のID処理を完全に撤去し大掃除を実施。

### タスク2: 再生コントロールと音量サブメニューの追加
    - `Window.h` に再生コントロール用IDと、音量メニュー（100%, 75%, 50%, 25%）のIDを追加。
    - `Window.cpp` にて、トレイメニュー最上部に再生コントロールを追加し、「音量 (Volume)」サブメニューを追加。
    - `Window.h` に `SetVolumeSetCallback` を追加し、`Application.cpp` 側で `m_audioPlayer.SetVolume` および `m_config.SetDefaultVolume` を呼び出す連携処理を実装。
    - `WM_COMMAND` にてメディアキーイベントへのマッピングを実装。

### タスク3: 「詳細設定」サブメニューの構築
    - `Window.cpp` の `WM_TRAYICON` ハンドラ内にて、「詳細設定 (Advanced)」のポップアップメニューを動的に生成し、既存のZオーダー、位置保存・リセット、全設定初期化等の項目をネストして配置。
    - `CheckMenuRadioItem` および `CheckMenuItem` の対象を `hAdvMenu` に変更し、正常な状態表示を維持。

### タスク4: 設定初期化時のバックアップ機能実装
    - `ConfigManager.cpp` の `SaveDefaultSettings()` メソッドにて、既存の `OZtone.ini` ファイルが存在する場合に `std::time` および `std::put_time` を用いて日時付きのバックアップファイル (`.bak`) を生成してからデフォルト内容で上書きする安全装置を実装。

### タスク5: アーキテクチャドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` を編集し、トレイメニューの構造変更（再生コントロール・音量の追加、詳細設定のサブメニュー化）および `SaveDefaultSettings()` でのバックアップ作成仕様を追記。

## 5. HOTFIX1
### 原因・理由: 
    - メニュー構築(`WM_TRAYICON`)のループスコープ置換において余分な `}` が残留し、直後の `WM_COMMAND` 等が `switch` 外と判定されるコンパイルエラーが発生した。
    - `ConfigManager.cpp` でのバックアップ処理にて、`std::localtime` による非推奨の警告(`warning C4996`)が発生した。

### 対応: 
    - `Window.cpp` の不要な `}` を削除し、スコープ構造を修正してコンパイルエラーを解消。
    - `ConfigManager.cpp` の日時取得処理をセキュアな `localtime_s` へ変更し、警告を解消。
