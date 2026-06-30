【AIへの指示（Phase 6 - Step 2 : システムトレイへの常駐と終了機能）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 2】**
アプリケーションをシステムトレイ（タスクバー通知領域）へ常駐させ、トレイアイコンの右クリックメニューからアプリケーションを終了できる機能を実装します。併せて、過去の仮実装をクリーンアップし、オリジナルのアプリアイコンを組み込みます。

**【実装要件】**
1. **アイコンリソースの追加**:
   - `assets/app_icon.ico` を読み込むため、`src/resource.h` に `IDI_APP_ICON` を定義し、`src/OZtone.rc` にアイコンリソースとして追加してください。
   - `Window::Initialize` におけるウィンドウクラス（`WNDCLASSEXW`）の登録時、`hIcon` および `hIconSm` に `LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON))` を指定し、EXEファイル自体のアイコンとしても適用されるようにしてください。

2. **Windowクラスの拡張 (トレイアイコンの登録と削除)**:
   - `Window.h` にカスタムメッセージ（例: `WM_TRAYICON = WM_APP + 1`）と、トレイアイコン操作用の `NOTIFYICONDATA` 構造体の管理を追加してください。
   - `Window::Initialize` の終盤で `Shell_NotifyIcon(NIM_ADD, &nid)` を呼び出し、システムトレイにアイコンを登録してください。アイコン画像には先ほどの `IDI_APP_ICON` を使用し、ツールチップテキストには "OZtone" と設定してください。
   - `WindowProc` の `WM_DESTROY` 処理内で `Shell_NotifyIcon(NIM_DELETE, &nid)` を呼び出し、終了時にアイコンがトレイに残るのを防ぐクリーンアップを確実に行ってください。

3. **コンテキストメニューの表示と終了処理**:
   - `WindowProc` にて `WM_TRAYICON` を捕捉し、`lParam` が `WM_RBUTTONUP` だった場合にコンテキストメニューを表示する処理を実装してください。
   - `CreatePopupMenu()` でメニューを作成し、「終了 (Exit)」という項目を追加してください。
   - `TrackPopupMenu()` でマウスポインタの位置にメニューを表示してください（※Win32の仕様上、メニュー表示直前に `SetForegroundWindow(hwnd)` を呼び出さないとメニューが消えなくなるバグがあるため注意）。
   - メニューから「終了」が選択されたら `WM_COMMAND` 経由で捕捉し、`PostMessage(hwnd, WM_CLOSE, 0, 0)` を発行して安全にアプリを終了させてください。

4. **仮実装（右クリック終了）の破棄**:
   - Phase 3-1で実装した「`WM_RBUTTONDOWN` 時にロゴ領域内ならアプリを終了させる」という仮実装を完全に削除してください。（ドラッグ移動のための左クリック判定は残します）

**【事後処理】**
実装完了後、ビルドを行ってください。起動時にシステムトレイとタスクバーにオリジナルアイコンが表示されること、アイコンの右クリックメニューから正常にアプリが終了できること、左上ロゴの右クリックでは終了しなくなったことを確認できたら、`PROJECT_ARCHITECTURE.md` を更新し、`_docs/logs/YYYYMMDD_HHMM_Phase6_Step2.md` へ実装レポートを出力してください。
