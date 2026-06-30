【AIへの指示（Phase 6 - Step 6 : 低レベルキーボードフック導入）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 6】**
前回のStep 4で実装したメディアキー対応が「ウィンドウ非アクティブ時に反応しない」という問題を解決するため、低レベルキーボードフック（WH_KEYBOARD_LL）を導入し、OSの最深部でメディアキーの入力を捕捉して処理するアーキテクチャに改修します。

**【実装要件】**
1. **Window クラスの拡張 (フックの準備)**:
   - `Window.h` に、フックハンドルを保持する `HHOOK m_keyboardHook;` と、静的コールバック内から自ウィンドウへメッセージを送るための静的変数 `static HWND s_hwnd;` を追加してください。
   - 低レベルキーボードフック用の静的コールバック関数 `static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);` を宣言してください。
   - `Window.h` にフックからの通知を受け取るためのカスタムメッセージ `WM_APP_MEDIAKEY` (`WM_APP + 2` など) を定義してください。

2. **Window クラスの実装 (フックの登録と解除)**:
   - `Window::Initialize` のウィンドウ生成成功直後に、`s_hwnd = m_hwnd;` としてHWNDを保持し、`m_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);` を呼び出してフックを登録してください。
   - `Window::~Window` または `WM_DESTROY` 処理内で、`m_keyboardHook` が有効な場合は `UnhookWindowsHookEx(m_keyboardHook);` を呼び出し、確実にフックを解除（クリーンアップ）してください。

3. **フックプロシージャの実装**:
   - `Window::LowLevelKeyboardProc` の実装内にて、`nCode == HC_ACTION` の場合、`wParam` が `WM_KEYDOWN` または `WM_SYSKEYDOWN` であれば、`lParam` を `KBDLLHOOKSTRUCT*` にキャストしてください。
   - `vkCode` が以下のいずれかである場合、`PostMessage(s_hwnd, WM_APP_MEDIAKEY, vkCode, 0);` を発行して自身のウィンドウへ通知してください。
     - `VK_MEDIA_PLAY_PAUSE`
     - `VK_MEDIA_STOP`
     - `VK_MEDIA_NEXT_TRACK`
     - `VK_MEDIA_PREV_TRACK`
   - メッセージの送信後も、他のアプリケーションの動作を阻害しないよう、必ず `CallNextHookEx(nullptr, nCode, wParam, lParam);` を返してください。

4. **メッセージ処理の置き換え**:
   - `WindowProc` にて、以前の `WM_APPCOMMAND` を捕捉する処理を**削除（または無効化）**してください。
   - 代わりに新設した `WM_APP_MEDIAKEY` を捕捉し、`wParam` (vkCode) に応じて、Step 4で実装した `m_onMediaCommand` コールバックを呼び出してください。
   - その際、Application側の既存実装を変更せずに済むよう、Window側で以下のように `APPCOMMAND` の定数へ変換してからコールバックへ渡してください。
     - `VK_MEDIA_PLAY_PAUSE` -> `APPCOMMAND_MEDIA_PLAY_PAUSE`
     - `VK_MEDIA_STOP` -> `APPCOMMAND_MEDIA_STOP`
     - `VK_MEDIA_NEXT_TRACK` -> `APPCOMMAND_MEDIA_NEXTTRACK`
     - `VK_MEDIA_PREV_TRACK` -> `APPCOMMAND_MEDIA_PREVIOUSTRACK`

**【事後処理】**
実装完了後、ビルドを行ってください。
ブラウザや他のアプリをアクティブにした状態（OZtoneがバックグラウンドにいる状態）でキーボードのメディアキーを押し、OZtoneが確実に再生/一時停止や曲送りなどの反応を示すことを確認できたら、実装レポートを出力してください。
