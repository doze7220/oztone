# RES:実装計画・作業レポート Phase 21-1: 背景アルバムアート操作の拡充

## 1. 実装目的
背景アルバムアートのフレーミング操作を拡張し、キーボード（SHIFTキー）を使わずともマウスの右クリックとホイール操作の組み合わせのみで直感的に完結できるようにする。さらに誤爆しやすかった中ボタンクリックによるリセット処理を、「右クリックを押しながら」という条件へ変更することで操作の統一感と安全性を向上させる。

## 2. アーキテクチャ設計
### 要件1: 画像拡縮操作の拡張
- `src/Window_Mouse.cpp` の `HandleMouseWheel` における判定を改修する。
- 既存の「SHIFTキーが押されている」という条件 (`GetAsyncKeyState(VK_SHIFT) & 0x8000`) に加え、「右ボタンが押されている」という条件 (`wParam & MK_RBUTTON`) を OR で追加する。これにより、どちらの操作でも `m_onArtFramingScroll` が呼び出されるようになる。

### 要件2: フレーミングリセット操作の変更
- プロジェクトのコードベースを調査した結果、現在 `WM_MBUTTONDOWN` のハンドリング自体が未実装（または一時的に抜け落ちている）状態であることが判明したため、メッセージディスパッチおよびマウスハンドリング基盤にミドルクリックの処理を新設する。
- `Window_Proc.cpp` の `WindowProc` に `WM_MBUTTONDOWN` の分岐を追加し、`HandleMButtonDown` へディスパッチする。
- `Window.h` に `bool HandleMButtonDown(WPARAM wParam, LPARAM lParam);` のシグネチャを追加。
- `Window_Mouse.cpp` に `HandleMButtonDown` の実装を追加し、`wParam & MK_RBUTTON`（右クリックが押されている）が真である場合のみ `m_onArtFramingReset()` を呼び出すようにする。

## 3. 実装タスクリスト
- `[x]` タスク1: 画像拡縮操作（マウスホイール）のフック条件拡張
- `[x]` タスク2: メッセージディスパッチ基盤への `WM_MBUTTONDOWN` 追加
- `[x]` タスク3: `HandleMButtonDown` の実装と右クリック併用条件の追加
- `[x]` タスク4: アーキテクチャ資料の更新

## 4. 詳細作業内容
### タスク1: 画像拡縮操作（マウスホイール）のフック条件拡張
- **対象ファイル**: `src/Window_Mouse.cpp`
- **作業内容**:
  - `HandleMouseWheel` 関数内のフレーミング拡縮条件を `if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (wParam & MK_RBUTTON))` に変更する。

### タスク2: メッセージディスパッチ基盤への `WM_MBUTTONDOWN` 追加
- **対象ファイル**: `src/Window.h`, `src/Window_Proc.cpp`
- **作業内容**:
  - `Window.h` の private メソッド定義部に `bool HandleMButtonDown(WPARAM wParam, LPARAM lParam);` を追加。
  - `Window_Proc.cpp` の `WindowProc` 内の `switch (uMsg)` に `case WM_MBUTTONDOWN:` を追加し、`if (HandleMButtonDown(wParam, lParam)) return 0; break;` の処理を記述する。

### タスク3: `HandleMButtonDown` の実装と右クリック併用条件の追加
- **対象ファイル**: `src/Window_Mouse.cpp`
- **作業内容**:
  - `Window::HandleMButtonDown` 関数を実装する。
  - 関数内で `if (wParam & MK_RBUTTON)` を判定し、真の場合に `if (m_onArtFramingReset) m_onArtFramingReset();` を実行して `true` を返す。それ以外は `false` を返す。

### タスク4: アーキテクチャ資料の更新
- **対象ファイル**: `PROJECT_ARCHITECTURE.md`
- **作業内容**:
  - `UI/ビジュアル仕様方針` の「ウィンドウのドラッグ移動」の項目を更新し、右ドラッグでの移動に加え、「右クリックを押しっぱなし＋マウスホイールで画像の拡縮」「右クリックを押しっぱなし＋中ボタンクリックでリセット」が可能になり、キーボードを使わずにマウス（片手）のみでフレーミングの全操作が完結する旨を追記した。
  - `Window クラス` の「背景フレーミングの入力フック」の項目を更新し、新規追加された `HandleMButtonDown` メソッドによるディスパッチ処理と、右クリック併用によるリセット条件について追記した。

### 作業完了報告
Phase 21-1 の各タスクの要件に従って、指定されたソースファイルの変更およびアーキテクチャ資料の更新を実施完了。
