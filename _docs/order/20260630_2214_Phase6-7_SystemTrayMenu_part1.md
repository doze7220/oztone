現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 7 - part 1】**
システムトレイの右クリックメニューを大幅に拡充し、「コンフィグウィンドウを作らずにアプリを制御する」ストイックなアーキテクチャへ移行します。まずはウィンドウのZ-Order制御、座標保存トグル、位置リセット、設定初期化の4機能を実装します。

**【実装要件】**
1. **ConfigManager の拡張**:
   - `[Window]` セクションに以下を追加し、読み書き処理とゲッター/セッターを実装してください。
     - `ZOrder` (int型。0: 通常, 1: 最前面(TopMost), 2: 最背面(Bottom))
     - `SavePositionOnExit` (bool型。デフォルト1)
   - 既存の `WindowX`, `WindowY` について、`CW_USEDEFAULT` にリセットする機能（または特定のデフォルト座標へ書き換える処理）を追加してください。

2. **Window クラスの拡張 (トレイメニュー構築)**:
   - `Window.h` に、メニューアイテム用のリソースID定数（例: `ID_TRAY_ZORDER_NORMAL`, `ID_TRAY_ZORDER_TOPMOST`, `ID_TRAY_ZORDER_BOTTOM`, `ID_TRAY_SAVE_POS`, `ID_TRAY_RESET_POS`, `ID_TRAY_RESET_ALL`, `ID_TRAY_EXIT` 等）を定義してください。
   - `WindowProc` の `WM_TRAYICON` で右クリックされた際、`CreatePopupMenu()` を用いて上記のメニューを構築してください。
   - メニュー表示前に ConfigManager から設定値を取得し、`CheckMenuItem` または `CheckMenuRadioItem` を用いて、現在の `ZOrder` 状態や `SavePositionOnExit` の ON/OFF 状態にチェックマーク（✔）を付けて表示してください。

3. **Window クラスの拡張 (コマンド処理)**:
   - `WM_COMMAND` にて各メニューアイテム選択時の処理を実装します。
   - **Z-Order変更**: 選択されたモード（0, 1, 2）を ConfigManager に保存し、`SetWindowPos` を呼び出して `HWND_TOPMOST` / `HWND_BOTTOM` / `HWND_NOTOPMOST` を即座に適用してください。
   - **SavePositionOnExitトグル**: ConfigManager の値を反転させて保存してください。
   - **Reset Position & Size**: `SetWindowPos` を用いてウィンドウサイズを 1024x512 の論理サイズ（DPIスケーリング適用後）に戻し、位置を画面中央またはデフォルト位置へ移動してください。
   - **Reset All Settings**: ConfigManager の `SaveDefaultSettings()` を呼び出してINIを初期化後、`MessageBoxW` で「設定を初期化しました。アプリを再起動します。」と通知し、`WM_CLOSE` を発行して終了させてください。

4. **Window クラスの既存処理修正**:
   - `WM_DESTROY` 時に呼ばれる `SaveWindowPosition` について、`ConfigManager::GetSavePositionOnExit()` が `true` の場合のみ実行されるように `if` 条件で囲んでください。
   - `Window::Initialize` において、ウィンドウ生成後に ConfigManager の `ZOrder` を読み取り、`SetWindowPos` で初期のZ-Orderを適用してください。

**【追記事項】**
メニューの構築順序はハードコードせず、Window.cpp の先頭付近に constexpr UINT TRAY_MENU_ORDER[] = { ... } のような定数配列を定義し、それを for ループで回して AppendMenu を実行する動的構築の仕組みにしてください（区切り線は 0 などで表現してください）。これにより、将来ソースコード上で簡単に順番を入れ替えられるようにします。

**【事後処理】**
実装完了後、ビルドを行ってください。トレイアイコンの右クリックから各種メニューが機能し、最前面/最背面への移動、ウィンドウサイズのリセット、およびチェックマークの表示切替が正しく動作することを確認できたら、実装レポートを出力してください。
