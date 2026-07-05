# Phase 16-1: ウィンドウドラッグ移動の全体化と移動ロック 実装計画書

## 実装の背景（目的）
現在の OZtone では、ウィンドウのドラッグ移動が左上のロゴアイコン領域に限定されており、操作性が制限されている。
本フェーズでは、ウィンドウの操作性を向上させるため、どのUI要素の判定にも引っかからない「背景部分全体」でドラッグ移動ができるように機能を拡張する。
同時に、誤操作によるウィンドウ移動を防ぐため、システムトレイメニューから「画面固定（移動ロック）」を可能にする設定を追加する。

## タスクリスト

### 1. `ConfigManager` クラスの修正
* `ConfigManager.h` に `m_lockWindowPosition` (bool) を追加し、ゲッター (`GetLockWindowPosition`) とセッター (`SetLockWindowPosition`) を定義する。
* `ConfigManager.cpp` のコンストラクタでデフォルト値を `false` に初期化する。
* `LoadSettings` にて INIファイル (例: `[Window]` セクションの `LockWindowPosition`) からの読み込み処理を追加する。
* `SetLockWindowPosition` にて変数を更新し、INIファイルへ書き込む処理を実装する。
* `SaveDefaultSettings` にデフォルト値の書き出し処理を追加する。

### 2. `Window` クラスの修正
* `Window.h` のシステムトレイメニューの enum に `ID_TRAY_LOCK_WINDOW_POS` を追加する。
* `Window.cpp` の `TRAY_MENU_ORDER` 配列の適切な位置（例: リサイズ設定等の付近）に `ID_TRAY_LOCK_WINDOW_POS` を挿入する。
* `WM_TRAYICON` のメニュー構築処理にて、`LockWindowPosition` が `true` の場合はチェックマーク (`MF_CHECKED`) を付与して「画面固定 (Lock Window Position)」を追加する。
* `WM_COMMAND` にて `ID_TRAY_LOCK_WINDOW_POS` が選択された際、`m_config->SetLockWindowPosition(!m_config->GetLockWindowPosition());` を呼び出し設定をトグルする処理を追加する。
* `WM_LBUTTONDOWN` 内のドラッグ移動処理 (`IsInLogoRegion(xPos, yPos)` の判定部分) を変更し、特定のUI領域（プレイリスト、ロゴメニュー、再生コントロールなど）のクリック処理の**最後のフォールバック**として、`m_config->GetLockWindowPosition() == false` であれば `SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0)` を発動するよう修正する。

### 3. ドキュメント `PROJECT_ARCHITECTURE.md` の更新
* 「1. プロジェクト概要と基本方針」の「UI/ビジュアル仕様方針」における「左上：アプリアイコン…ドラッグ移動アンカー兼ドロップ領域となる」という記述を、「UI要素がない背景部分全体でドラッグ移動可能」という仕様に修正する。
* 「設定/システムトレイアイコン」の項目に「画面固定（移動ロック）」機能が追加された旨を追記する。

## 作業報告

### タスク1: `ConfigManager` クラスの修正
* `ConfigManager.h` に `m_lockWindowPosition` 変数および対応するゲッター・セッターを追加しました。
* `ConfigManager.cpp` にて、コンストラクタでの初期化（`false`）、INIファイルからの読み込み処理 (`LoadSettings`)、および保存処理 (`SetLockWindowPosition`) を実装しました。
* また、INIファイルのデフォルト設定内容にも `LockWindowPosition=0` を追加しました。

### タスク2: `Window` クラスの修正
* `Window.h` のシステムトレイメニュー用の enum に `ID_TRAY_LOCK_WINDOW_POS` を追加しました。
* `Window.cpp` の `TRAY_MENU_ORDER` 配列の適切な位置に `ID_TRAY_LOCK_WINDOW_POS` を挿入しました。
* `WM_TRAYICON` にて、`LockWindowPosition` の状態に応じてトレイメニューに「画面固定 (Lock Window Position)」をチェック付き/なしで追加する処理を実装しました。
* `WM_COMMAND` にて、メニュークリック時に `SetLockWindowPosition` を呼び出し設定をトグルする処理を実装しました。
* `WM_LBUTTONDOWN` 内のドラッグ移動処理（`HTCAPTION` の発動条件）を変更し、既存のすべてのUI判定（プレイリスト、ロゴメニュー等）を通過した最後のフォールバックとして、`LockWindowPosition == false` の場合に背景全体からドラッグ移動できるように修正しました。
### タスク3: ドキュメント `PROJECT_ARCHITECTURE.md` の更新
* 「1. プロジェクト概要と基本方針」の「UI/ビジュアル仕様方針」におけるアプリアイコンの記述から「ドラッグ移動アンカー兼」を削除しました。
* 新たに「ウィンドウのドラッグ移動」の項目を追加し、特定のUI要素がない背景部分全体でドラッグ可能になる仕様を追記しました。
* 「設定/システムトレイアイコン」の項目に「画面固定（移動ロック）」機能を追加した旨を追記しました。
