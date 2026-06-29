# PROJECT_ARCHITECTURE.md
## OZtone 全体仕様および実装定義書

本ドキュメントは、AIがソースコードを読まずとも現在のプロジェクト状態と仕様、アーキテクチャ構造を即座に把握できるようにするための資料である。フェーズが進むごとに、実装された項目をここへ随時追記・更新していくこと。

---

### 1. プロジェクト概要と基本方針
*   **アプリ名**: OZtone (デスクトップアクセサリ風MP3プレイヤー)
*   **言語・環境**: C++20, Win32 API
*   **ビルド**: CMake
*   **外部ライブラリ**: miniaudio (音声処理), TagLib (MP3タグ処理) ※現在未実装
*   **設定ファイル**: `OZtone.ini` に保存 ※現在未実装
*   **UI/ビジュアル仕様方針**:
    *   ウィンドウ枠なし（最終段階）、背景にアルバムアートを全面フィット・トリミング表示。半透明化はしない。
    *   中央に円形のビジュアライザを配置。
    *   左下：アルバムアート全体、曲名、アーティスト名。ドラッグ移動アンカー兼ドロップ領域となる。
    *   右側：半透明のプレイリスト。
    *   下部：シークバー、操作ボタン。
    *   プレースホルダー：アルバムアート未設定時のデフォルト画像を用意。

### 2. 禁止事項・ルール (AI向け)
*   **Manager巨大化禁止 / 責務分離**: 一度に複数の機能を実装せず、フェーズごとに一歩ずつ進めること。
*   **WndProcの肥大化禁止**: ウィンドウプロシージャ内に描画アルゴリズムやゲームロジックを直接記述しないこと。処理は必ず専用クラスへ委譲する。
*   **Win32ベタ書きの回避**: グローバル変数を極力避け、クラスベースで状態を管理する（ポインタ渡しやシングルトン等）。
*   **モダンUIフレームワーク禁止**: Qtなどの巨大なフレームワークは使わず、純粋なC++とWin32 APIで構築する。

---

### 3. プロジェクトフォルダ構造
現在のディレクトリ・ファイル構成は以下の通り（Phase 1 Step 1 時点）。

```text
oztone/
 ├── CMakeLists.txt          # CMakeビルド設定
 ├── src/                    # C++ソースコード
 │   ├── main.cpp            # エントリポイント(wWinMain)
 │   ├── Application.h/cpp   # アプリケーションライフサイクル・メインループ
 │   └── Window.h/cpp        # Win32ウィンドウ管理・WndProc
 ├── _docs/                  # 開発関連ドキュメント
 │   ├── logs/               # AIの実装レポート出力先 (YYYYMMDD_HHMM_タスク名.md)
 │   └── OzPlayer_Plan.txt   # 開発初期の要望メモ
 ├── PROJECT_ARCHITECTURE.md # 本ファイル（全体仕様・設計・アーキテクチャ）
 ├── PROJECT_CONSTITUTION.md # プロジェクト憲法（AI向け絶対ルール）
 └── README.md               # プロジェクト概要
```

---

### 4. 全体のルーチン（アーキテクチャと実行フロー）
**【現在の実行フロー（Phase 1 Step 1 時点）】**
1.  **エントリポイント (`wWinMain`)**: `Application` インスタンスを生成。
2.  **初期化 (`Application::Initialize`)**: 内部で `Window::Initialize` を呼び出し、Win32 APIを介してウィンドウを生成・表示。
3.  **メインループ (`Application::Run`)**: `Window::ProcessMessages` を毎フレーム呼び出し、OSのメッセージを処理。
4.  **メッセージ処理 (`Window::WindowProc`)**: OSからのイベントを処理。現在は `WM_DESTROY` のみ捕捉してアプリを終了させている。

---

### 5. 実装済みクラス・関数リファレンス

#### `Application` クラス (src/Application.h, cpp)
アプリケーション全体のライフサイクルとメインループを統括するマネージャ。
*   **`bool Initialize(HINSTANCE hInstance, int nCmdShow)`**
    *   ウィンドウ等のサブシステムの初期化を行う。今後 Renderer や AudioPlayer の初期化もここに追加される。
*   **`void Run()`**
    *   メインループ（ゲームループ）を実行する。現在はCPU負荷軽減のためループ内に `Sleep(1)` を配置。

#### `Window` クラス (src/Window.h, cpp)
Win32 APIのウィンドウ生成・破棄、メッセージディスパッチ処理を隠蔽・カプセル化するクラス。
*   **`bool Initialize(HINSTANCE hInstance, int nCmdShow)`**
    *   ウィンドウクラス (`WNDCLASSEXW`) の登録と、`CreateWindowExW` によるウィンドウ生成。
*   **`bool ProcessMessages()`**
    *   `PeekMessage` を使用したノンブロッキングなメッセージ処理。`WM_QUIT` 受信で false を返し、メインループを終了させる。
*   **`LRESULT CALLBACK WindowProcStatic(HWND, UINT, WPARAM, LPARAM)`**
    *   Win32 APIから直接呼び出される静的コールバック関数。`WM_NCCREATE` 時に `GWLP_USERDATA` に自身の `this` ポインタを保存し、以降のメッセージではポインタを復元して非静的な `WindowProc` へ処理を転送する。
*   **`LRESULT WindowProc(HWND, UINT, WPARAM, LPARAM)`**
    *   実際のメッセージハンドラ。責務分離のため、ここには描画ロジックなどを書かない。

---

### 6. 定数・設定値 (実装済みデータ)
*   **ウィンドウ初期サイズ**: 1024 x 512 px (現在は `Window::Initialize` 内で `RECT` としてハードコード定義)
*   **ウィンドウスタイル**: `WS_OVERLAPPEDWINDOW` (※開発初期段階のデバッグ用。今後のフェーズで枠なし `WS_POPUP` 等に変更予定)
*   **ウィンドウクラス名**: `L"OZtoneWindowClass"`
*   **ウィンドウタイトル**: `L"OZtone"`
