【AIへの指示（Phase 6 - Step 5 : コマンドライン引数対応と多重起動防止）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 5】**
「送る (SendTo)」メニュー等からのコマンドライン引数起動に対応し、多重起動防止（プロセス間通信）を実装します。すでに起動しているOZtoneがある場合は、新しいプロセスは引数のファイルパスを既存プロセスに送り、自身は終了します。

**【実装要件】**
1. **Window クラスの拡張 (WM_COPYDATA 受信)**:
   - `Window.h` に、文字列（ファイルパス）を受信した際のコールバック関数（例: `std::function<void(const std::wstring&)> m_onCopyDataCallback;`）と、それをセットする `SetCopyDataCallback` メソッドを追加してください。
   - `WindowProc` で `WM_COPYDATA` を捕捉し、`COPYDATASTRUCT::lpData` からファイルパス文字列（`LPCWSTR`）を取り出し、コールバックへ渡して発火させてください。

2. **Application クラスの連携**:
   - `Application::Initialize` 内で `m_window.SetCopyDataCallback` を用いて、受信したパス文字列を `std::vector<std::wstring>` に詰め、既存の `m_application.OnFilesDropped()` へ流し込む処理を登録してください。
   - `Application` クラスに、起動直後のコマンドライン引数を処理するためのメソッド（例: `ProcessCommandLineArgs(int argc, LPWSTR* argv)`）を追加し、内部で引数パスを集めて `OnFilesDropped` を呼び出すように実装してください。

3. **エントリポイント (main.cpp) の拡張 (多重起動防止と送信)**:
   - `wWinMain` の先頭で `CreateMutexW` を用いて名前付きミューテックス（例: `L"OZtone_Mutex"`）を作成し、`GetLastError() == ERROR_ALREADY_EXISTS` で多重起動を検知してください。
   - `CommandLineToArgvW` を用いてコマンドライン引数を取得してください。
   - **多重起動だった場合**:
     - 引数がある場合（`argc > 1`）、`FindWindowW` で既存のOZtoneウィンドウ（クラス名 `L"OZtoneWindowClass"`）を検索してください。
     - 見つかった場合、2番目以降の引数（ファイルパス）をループで回し、`WM_COPYDATA` を用いて `COPYDATASTRUCT` にパス文字列を詰めて既存ウィンドウへ送信（`SendMessage`）し、最後にプロセスを終了してください。
   - **初回起動の場合**:
     - 通常通り `Application::Initialize` を呼び出します。
     - 初期化完了後、引数がある場合（`argc > 1`）は `m_application.ProcessCommandLineArgs` を呼び出して起動直後に読み込ませてください。

**【事後処理】**
実装完了後、ビルドを行ってください。
1. OZtoneが起動していない状態で、MP3ファイルを OZtone.exe に直接ドラッグ＆ドロップ（引数起動）し、正常に再生開始されること。
2. 既にOZtoneが起動している状態で、別のMP3ファイルを OZtone.exe にドラッグ＆ドロップした際、新しいウィンドウは開かず、既存のOZtoneのプレイリストに曲が追加されること。
を確認できたら、`PROJECT_ARCHITECTURE.md` を更新し、実装レポートを出力してください。
