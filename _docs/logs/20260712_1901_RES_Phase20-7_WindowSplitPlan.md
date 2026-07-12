# RES:実装計画・作業レポート Phase 20-7: Window.cpp 分割に向けた事前調査と整理・解体計画

## 1. 実装目的
現在1200行を超える `Window.cpp` を、AI-IDEのコンテキスト節約のため、実装のみを責務別の .cpp ファイルへ物理分割する。
分割後は「ある機能を修正する際に、対象となる.cppだけを読めば実装を理解できる」ことを目標とする。
本フェーズでは、コードの書き換えは行わず、高度なアーキテクチャ設計と実装・解体計画の立案のみを行う。

## 2. アーキテクチャ設計
### 要件1: WindowProcの薄いディスパッチャ化と抽出
    - 現在の巨大な `WindowProc` を薄いディスパッチャ（司令塔）として維持するため、各メッセージ処理を専用のメンバ関数 `HandleXXXX()` へ抽出する。
    - 抽出候補一覧:
        - `HandleHotkey(WPARAM)`
        - `HandleGetMinMaxInfo(LPARAM)`
        - `HandleSize(WPARAM, LPARAM)`
        - `HandleNcHitTest(LPARAM)`
        - `HandleWindowPosChanging(LPARAM)`
        - `HandleMouseActivate()`
        - `HandleMouseMove(WPARAM, LPARAM)`
        - `HandleMouseLeave()`
        - `HandleLButtonDown(WPARAM, LPARAM)`
        - `HandleLButtonUp()`
        - `HandleRButtonDown(WPARAM, LPARAM)`
        - `HandleRButtonUp()`
        - `HandleLButtonDblClk(WPARAM, LPARAM)`
        - `HandleMouseWheel(WPARAM, LPARAM)`
        - `HandleKeyDown(WPARAM)`
        - `HandleTrayIcon(LPARAM)`
        - `HandleCommand(WPARAM)`
        - `HandleAppMediaKey(WPARAM)`
        - `HandleCopyData(LPARAM)`
        - `HandleDestroy(HWND)`
    - `Window.h` にこれらの関数の宣言を private メンバ関数として追加する（公開インターフェースや変数追加は行わない）。

### 要件2: コンテキスト単位での解体計画
    - 抽出されたメンバ関数群と既存の関数を、以下の責務別ファイルへ物理分割するロードマップを策定した。
    - `Window_Proc.cpp` (薄いメッセージディスパッチの基盤)
        - `WindowProcStatic`, `WindowProc`
    - `Window_Mouse.cpp` (各UIの座標判定とマウスイベント処理)
        - `IsInLogoRegion`, `IsInLogoMenuRegion`, `GetLogoMenuButtonAt`
        - `IsInPlaybackControlRegion`, `IsInVolumeControlRegion`, `IsInPlaylistRegion`
        - `GetPlaylistToolbarButtonAt`, `IsPlaylistPinnedButtonAt`, `GetPlaybackButtonAt`
        - `HandleMouseMove`, `HandleMouseLeave`, `HandleLButtonDown`, `HandleLButtonUp`, `HandleRButtonDown`, `HandleRButtonUp`, `HandleLButtonDblClk`, `HandleMouseWheel`
    - `Window_TrayMenu.cpp` (トレイメニュー生成とコマンド処理)
        - 定数 `TRAY_MENU_ORDER`
        - `HandleTrayIcon` (右クリックによるメニュー構築)
        - `HandleCommand` (メニューからのコマンド実行処理)
    - `Window_DropTarget.cpp` (ファイルドロップ処理)
        - `DropTarget` クラスのメンバ関数すべて (`QueryInterface`, `AddRef`, `Release`, `DragEnter`, `DragOver`, `DragLeave`, `Drop`)
    - `Window_Initialize.cpp` (ウィンドウ生成・初期化)
        - `Window` コンストラクタ, デストラクタ
        - `Initialize`
        - `HandleDestroy` (終了時処理、位置保存など)
    - `Window_System.cpp` (フック・グローバルホットキー等のシステム処理)
        - `RegisterHotkeys`, `UnregisterHotkeys`
        - `LowLevelKeyboardProc` (低レベルキーボードフック)
        - `HandleHotkey`, `HandleAppMediaKey`, `HandleCopyData`
        - `HandleGetMinMaxInfo`, `HandleSize`, `HandleNcHitTest`, `HandleWindowPosChanging`, `HandleMouseActivate`, `HandleKeyDown`

### 要件3: ユーティリティファイルの作成禁止
    - 分割は完全に「責務単位」とし、機能横断的なファイル（`WindowUtility.cpp` や `WindowCommon.cpp` など）は作成しない。

## 3. 実装タスクリスト
[x] タスク1: `Window.h` の更新と `WindowProc` の整理
- [x] `Window.h` に `HandleXXX()` 関数の宣言を追加。
- [x] `Window.cpp` でメッセージハンドラを個別のメソッドに抽出。
- [x] `WindowProc` を純粋なディスパッチャに変更。
[x] タスク2: `Window_Mouse.cpp` の分離
    - `Window_Mouse.cpp` を新規作成し、座標判定（IsIn...系）やマウスイベント（HandleMouseMove, HandleLButtonDownなど）を移行する。
[x] タスク3: `Window_TrayMenu.cpp` の分離
    - `Window_TrayMenu.cpp` を新規作成し、トレイアイコンやメニュー構築（TRAY_MENU_ORDER）、メニューからのコマンド処理（HandleTrayIcon, HandleCommandなど）を移行する。
[x] タスク4: `Window_DropTarget.cpp` の分離
    - `Window_DropTarget.cpp` を新規作成し、OLE Drag and Drop（IDropTargetの実装）に関する処理を完全に移行する。
[x] タスク5: `Window_System.cpp` の分離
    - `Window_System.cpp` を新規作成し、グローバルホットキー、低レベルキーボードフック、WM_COPYDATA（多重起動防止）などのシステム連携処理を移行する。
[x] タスク6: `Window_Initialize.cpp` の分離
    - `Window_Initialize.cpp` を新規作成し、ウィンドウの登録や生成（Initializeなど）、破棄（HandleDestroyなど）を移行する。
[x] タスク7: `Window_Proc.cpp` の作成と最終クリーンアップ
    - 大元に残った薄いディスパッチャ（WindowProc）を `Window_Proc.cpp` へ移行する。
    - `CMakeLists.txt` に新設した6つのファイルを追加し、古い `Window.cpp` を物理削除してビルドを通す。
[x] タスク8: ビルドと動作確認
    - プロジェクトが正常にコンパイル・リンクできることを確認する。
    - ウィンドウ生成、マウス操作、トレイメニュー表示、ホットキー等の機能が分割前と完全に一致して動作することを確認する。

## 4. 詳細作業内容
### タスク1: `Window.h` の更新
    - `HandleMouseMove`, `HandleLButtonDown`, `HandleRButtonDown`, `HandleMouseWheel`, `HandleTrayIcon`, `HandleCommand`, `HandleDestroy` などの20個のハンドラメソッドを `Window.cpp` に追加しました。
    - `WindowProc` の `switch` 文内のロジックを各ハンドラメソッドへ移動し、`WindowProc` 自身はディスパッチ専用関数としてスッキリさせました。
    - 機能の変更や物理的なファイル分割は行っていません。
### タスク2: `Window_Mouse.cpp` の分離
    - `src/Window_Mouse.cpp` を新規作成し、`Window.cpp` からマウス入力ハンドラ(`HandleMouseMove`, `HandleLButtonDown`など)および領域・座標判定メソッド(`IsInLogoRegion`, `GetPlaybackButtonAt`など)を完全に移行しました。
    - `CMakeLists.txt` に新設した `src/Window_Mouse.cpp` をコンパイル対象として追加しました。
### タスク3: `Window_TrayMenu.cpp` の分離
    - `src/Window_TrayMenu.cpp` を新規作成し、トレイアイコン関連(`TRAY_MENU_ORDER`, `HandleTrayIcon`)やメニューからのコマンド処理(`HandleCommand`)を完全に移行しました。
    - `CMakeLists.txt` に新設した `src/Window_TrayMenu.cpp` をコンパイル対象として追加しました。
### タスク4: `Window_DropTarget.cpp` の分離
    - `src/Window_DropTarget.cpp` を新規作成し、`Window.cpp` から `DropTarget` クラスのメンバ関数すべて（`QueryInterface`, `AddRef`, `Release`, `DragEnter`, `DragOver`, `DragLeave`, `Drop`）を完全に移行しました。
    - `CMakeLists.txt` に新設した `src/Window_DropTarget.cpp` をコンパイル対象として追加しました。
### タスク5: `Window_System.cpp` の分離
    - `src/Window_System.cpp` を新規作成し、`Window.cpp` からシステム連携処理（`RegisterHotkeys`, `LowLevelKeyboardProc`, `HandleHotkey`, `HandleAppMediaKey`, `HandleCopyData`, `HandleGetMinMaxInfo`, `HandleSize`, `HandleNcHitTest`, `HandleWindowPosChanging`, `HandleKeyDown` 等）を完全に移行しました。
    - `CMakeLists.txt` に新設した `src/Window_System.cpp` をコンパイル対象として追加しました。
### タスク6: `Window_Initialize.cpp` の分離
    - `src/Window_Initialize.cpp` を新規作成し、`Window.cpp` からウィンドウ生成、初期化処理（`Initialize`）、および破棄のイベントハンドラ（`HandleDestroy`）、コンストラクタ・デストラクタを完全に移行しました。
    - `CMakeLists.txt` に新設した `src/Window_Initialize.cpp` をコンパイル対象として追加しました。
### タスク7: `Window_Proc.cpp` の作成と最終クリーンアップ
    - `src/Window_Proc.cpp` を新規作成し、`Window.cpp` に残存していた純粋なディスパッチャ（`WindowProcStatic`, `WindowProc`など）を完全に移行しました。
    - `src/Window.cpp` をファイルシステムから物理的に削除し、`CMakeLists.txt` を更新して `src/Window.cpp` の代わりに `src/Window_Proc.cpp` を登録しました。これにより `Window.cpp` の完全解体が完了しました。
### タスク8: ビルドと動作確認
    - ユーザー様にビルドと動作確認（ウィンドウ生成、マウス操作、トレイメニュー表示、ホットキー等の機能が分割前と完全に一致して動作すること）を依頼しました。
    - ユーザー様によるビルドと動作確認が完了したことを確認しました。
    - アーキテクチャ資料 (`PROJECT_ARCHITECTURE.md`) へ Window クラスの分割情報を追記し、Phase 20-7 の全タスクを完了としました。
