# PROJECT_ARCHITECTURE.md
## OZtone 全体仕様および実装定義書

本ドキュメントは、AIがソースコードを読まずとも現在のプロジェクト状態と仕様、アーキテクチャ構造を即座に把握できるようにするための資料である。フェーズが進むごとに、実装された項目をここへ随時追記・更新していくこと。

---

### 1. プロジェクト概要と基本方針
*   **アプリ名**: OZtone (デスクトップアクセサリ風MP3プレイヤー)
*   **言語・環境**: C++20, Win32 API, Direct3D 11, Direct2D, DirectComposition
*   **ビルド**: CMake (Visual Studio Developer Command Prompt を用いた MSBuild や Ninja の利用を想定)
*   **外部ライブラリ**: miniaudio (音声処理), TagLib (MP3タグ処理) ※TagLibは現在未実装
*   **設定ファイル**: `OZtone.ini` に保存 (`ConfigManager` により管理)
*   **UI/ビジュアル仕様方針**:
    *   ウィンドウ枠なし（最終段階）、背景にアルバムアートを全面フィット・トリミング表示。デスクトップとのブレンド（ウィンドウ全体の透過）に対応。
    *   中央に円形のビジュアライザを配置。
    *   左上：アプリアイコン（白フラット化したもの）を表示。ドラッグ移動アンカー兼ドロップ領域となる。
    *   左下：アルバムアート全体、曲名、アーティスト名。
    *   右側：半透明のプレイリスト。
    *   下部：シークバー、操作ボタン。
    *   プレースホルダー：アルバムアート未設定時のデフォルト画像を用意。
    *   **スキン・リソース仕様**: 各種画像（アプリアイコン等）は、基本的にはexeの埋め込みリソース(`RCDATA`)を使用しますが、exeと同階層に同名の画像ファイルが存在する場合はそれを優先して読み込むハイブリッド（フォールバック）方式を採用しています。
    *   **画像描画・デコード**: 画像のデコードには **WIC (Windows Imaging Component)** を使用し、Direct2Dのビットマップとしてロード・描画します。テキスト描画には **DirectWrite** を用いて高品質なレンダリングを行います。
    *   **ウィンドウ透過方式**: `CreateSwapChainForComposition` により `DXGI_ALPHA_MODE_PREMULTIPLIED` のスワップチェインを作成し、**DirectComposition** (`IDCompositionDevice`) を介してウィンドウに合成する方式を採用。`CreateSwapChainForHwnd` では PREMULTIPLIED アルファが未サポートのため、この構成が必須となる。
    *   **High DPI (Per-Monitor V2) 対応**: `SetProcessDpiAwarenessContext` によりDPI対応を有効化し、ウィンドウサイズおよびD2Dレンダーターゲットの自動スケーリングに対応。

### 2. 禁止事項・ルール (AI向け)
*   **Manager巨大化禁止 / 責務分離**: 一度に複数の機能を実装せず、フェーズごとに一歩ずつ進めること。
*   **WndProcの肥大化禁止**: ウィンドウプロシージャ内に描画アルゴリズムやゲームロジックを直接記述しないこと。処理は必ず専用クラスへ委譲する。
*   **Win32ベタ書きの回避**: グローバル変数を極力避け、クラスベースで状態を管理する（ポインタ渡しやシングルトン等）。
*   **モダンUIフレームワーク禁止**: Qtなどの巨大なフレームワークは使わず、純粋なC++とWin32 APIで構築する。

---

### 3. プロジェクトフォルダ構造
```text
oztone/
 ├── build.bat               # CMakeビルド実行用スクリプト
 ├── CMakeLists.txt          # CMakeビルド設定
 ├── build/                  # CMakeのビルド出力ディレクトリ
 ├── src/                    # C++ソースコード
 │   ├── main.cpp            # エントリポイント(wWinMain)
 │   ├── Application.h/cpp   # アプリケーションライフサイクル・メインループ
 │   ├── Window.h/cpp        # Win32ウィンドウ管理・WndProc
 │   ├── ConfigManager.h/cpp # 設定ファイル(OZtone.ini)の管理
 │   ├── Renderer.h/cpp      # Direct3D11 / Direct2D ハイブリッド描画エンジン
 │   ├── AudioPlayer.h/cpp   # 音声再生エンジン (miniaudioラップ)
 │   ├── miniaudio.h         # miniaudio シングルヘッダライブラリ
 │   ├── resource.h          # リソースID定義ヘッダ
 │   └── OZtone.rc           # 画像などを埋め込むリソーススクリプト
 ├── assets/                 # ボタン類などのリソース
 ├── _docs/                  # 開発関連ドキュメント
 │   ├── logs/               # AIの実装レポート出力先 (YYYYMMDD_HHMM_タスク名.md)
 │   └── OzPlayer_Plan.txt   # 開発初期の要望メモ
 ├── PROJECT_ARCHITECTURE.md # 本ファイル（全体仕様・設計・アーキテクチャ）
 ├── PROJECT_CONSTITUTION.md # プロジェクト憲法（AI向け絶対ルール）
 └── README.md               # プロジェクト概要
```

---

### 4. 全体のルーチン（アーキテクチャと実行フロー）
1.  **エントリポイント (`wWinMain`)**: `CoInitializeEx` にて COM を初期化後、`Application` インスタンスを生成。
2.  **初期化 (`Application::Initialize`)**: 内部で `Window::Initialize`、`Renderer::Initialize`、`AudioPlayer::Initialize` を順次呼び出し、システムの初期化と初回のテスト再生を行う。
3.  **メインループ (`Application::Run`)**: `Window::ProcessMessages` を毎フレーム呼び出し、OSのメッセージを処理。
4.  **メッセージ処理 (`Window::WindowProc`)**: OSからのイベントを処理。`WM_DESTROY` の他、アプリアイコン上での `WM_LBUTTONDOWN`（ドラッグ移動）や `WM_RBUTTONDOWN`（アプリ終了）を処理している。

---

### 5. 実装済みクラス・関数リファレンス

#### `Application` クラス (src/Application.h, cpp)
アプリケーション全体のライフサイクルとメインループを統括するマネージャ。
*   **`bool Initialize(HINSTANCE hInstance, int nCmdShow)`**
    *   ウィンドウ等のサブシステムの初期化を行う。今後 Renderer や AudioPlayer の初期化もここに追加される。
*   **`void Run()`**
    *   メインループ（ゲームループ）を実行する。ループ内で `AudioPlayer` から再生位置と総時間を取得し、進捗率（progress）と時間表示文字列（mm:ss / mm:ss形式）を計算して `Renderer::Render` に渡す。現在はCPU負荷軽減のためループ内に `Sleep(1)` を配置。

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
    *   アプリアイコン上でのクリックイベント（ドラッグ移動、右クリック終了）やホバー状態の判定を処理する。
*   **`bool IsInLogoRegion(int x, int y) const`**
    *   指定された物理座標(x, y)を、DPIスケールを用いて論理座標に変換してから、アプリアイコンの論理レイアウト領域内かどうかを判定する。
*   **ウィンドウ位置・サイズの保存処理**
    *   `WM_DESTROY` 時に `GetWindowRect` と `GetClientRect` からウィンドウ位置・サイズを取得し、DPIスケールで論理サイズに変換して `ConfigManager` へ保存する。

#### `ConfigManager` クラス (src/ConfigManager.h, cpp)
アプリケーションの設定（`OZtone.ini`）の読み込み、およびデフォルト設定の書き出しを管理する。
*   **`bool Initialize()`**
    *   実行ファイルと同階層の `OZtone.ini` を解決し、存在しなければデフォルト値で作成する。その後、各設定値を読み込む。
*   ウィンドウ表示フラグ (`GetShowTitleBar` 等) やウィンドウの論理位置・サイズ (`GetWindowX` 等)、アプリアイコンやアルバムアートのレイアウト情報 (`GetLogoX` 等, `GetBaseX` 等, `GetBgOpacity` 等)、およびシークバーのレイアウトと文字間隔設定（`GetSeekBarWidthRatio`, `GetSeekBarTimeLetterSpacing` 等）のゲッターを提供する。
*   **`void SaveWindowPosition(int x, int y, int width, int height)`**
    *   ウィンドウの表示位置・サイズを ini ファイルに保存する。

#### `Renderer` クラス (src/Renderer.h, cpp)
Direct3D 11 と Direct2D を用いたハイブリッド描画エンジン。画像デコード用に WIC、テキスト描画用に DirectWrite を内包する。ウィンドウ透過にはDirectCompositionを使用し、アルファブレンドされたスワップチェインを合成する。
また、**内部ロジックを論理ピクセル（96DPI基準）に統一し、描画の根元で `SetTransform` を用いて一括スケールする** というDPIスケーリングアーキテクチャを採用している。
*   **`bool Initialize(HWND hwnd, const ConfigManager& config)`**
    *   D3D11デバイス、`CreateSwapChainForComposition` による透過対応スワップチェイン、DirectCompositionデバイスの作成とウィンドウへのバインド、D2D1ファクトリ等を作成。WICファクトリを初期化し、初期アセット等を読み込む。
    *   ウィンドウのDPIを取得して `m_dpiScale` を算出し保持する。また D2D ターゲットのDPI設定は論理ピクセル基準(96.0f)に固定する。
*   **`bool LoadBitmapResource(const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap)`**
    *   指定されたファイル名がローカルに存在すればWICを用いてファイルからデコードし、無ければメモリリソース(`RCDATA`)からストリームを作成してデコードするフォールバック処理を行う。
*   **`void Render(bool isHovered, float progress, const std::wstring& timeString)`**
    *   毎フレーム呼び出され、描画処理を実行する。`BeginDraw()` の直後に `SetTransform` によって `m_dpiScale` を適用し、以降の描画はすべて論理ピクセルのまま物理ピクセルへ自動拡大される。
    *   完全透明でクリアした後、背景アルバムアート、アプリアイコン、ダミー曲情報テキスト、および進行度(`progress`)に応じたシークバーと現在時間(`timeString`)を描画する。時間文字列の描画には `IDWriteTextLayout` を使用し、ConfigManager から取得した文字間隔を `SetCharacterSpacing` で適用している。

#### `AudioPlayer` クラス (src/AudioPlayer.h, cpp)
音声処理ライブラリ `miniaudio` をラップし、バックグラウンドでのMP3再生を管理するクラス。
*   **`bool Initialize()`**
    *   `ma_engine` をインスタンス化し、`ma_engine_init` で初期化する。
*   **`void Uninitialize()`**
    *   ロード済みのサウンド（`ma_sound`）を破棄し、初期化済みであれば `ma_engine_uninit` を呼び出してエンジンリソースを解放する。
*   **`bool Play(const std::string& filepath)`**
    *   指定されたMP3ファイルを `ma_sound_init_from_file` でロードし、`ma_sound_start` にて再生する。既にロード済みの場合は一度破棄してから再ロードする。
*   **`float GetPositionSeconds()`**
    *   現在の再生位置（秒）を取得して返す。
*   **`float GetLengthSeconds()`**
    *   曲の総時間（秒）を取得して返す。
*   **`bool IsPlaying()`**
    *   現在再生中かどうかを返す。

---

### 6. 定数・設定値 (実装済みデータ)
*   **ウィンドウ初期サイズ**: `ConfigManager` (INIファイル) から `GetWindowWidth()`, `GetWindowHeight()` を通じて論理サイズを取得し、システムDPIでスケーリングして物理サイズを決定。
*   **ウィンドウスタイル**: `WS_POPUP | WS_EX_LAYERED` (完全枠なし・レイヤードウィンドウ。ConfigManager の設定に応じて動的に決定)
*   **ウィンドウクラス名**: `L"OZtoneWindowClass"`
*   **ウィンドウタイトル**: `L"OZtone"`
