# PROJECT_ARCHITECTURE.md
## OZtone 全体仕様および実装定義書

本ドキュメントは、AIがソースコードを読まずとも現在のプロジェクト状態と仕様、アーキテクチャ構造を即座に把握できるようにするための資料である。フェーズが進むごとに、実装された項目をここへ随時追記・更新していくこと。

---

### 1. プロジェクト概要と基本方針
*   **アプリ名**: OZtone (デスクトップアクセサリ風MP3プレイヤー)
*   **言語・環境**: C++20, Win32 API, Direct3D 11, Direct2D, DirectComposition
*   **ビルド**: CMake (Visual Studio Developer Command Prompt を用いた MSBuild や Ninja の利用を想定)
*   **外部ライブラリ**: miniaudio (音声処理), TagLib (MP3タグ処理)
*   **設定ファイル**: `OZtone.ini` に保存 (`ConfigManager` により管理)
*   **UI/ビジュアル仕様方針**:
    *   ウィンドウ枠なし、背景にアルバムアートを全面フィット・トリミング表示。デスクトップとのブレンド（ウィンドウ全体の透過）に対応。
    *   UIの視認性を確保するため、背景アルバムアートとUIの間にはダークオーバーレイ（黒半透明矩形）を挟み、各UI要素（テキストや画像）にはドロップシャドウを描画する。
    *   中央に円形のビジュアライザを配置。
    *   左上：アプリアイコン（白フラット化したもの）を表示。ドラッグ移動アンカー兼ドロップ領域となる。
    *   左下：アルバムアート全体、曲名、アーティスト名。
    *   右下：「次の曲」の極小アルバムアートと曲名・アーティスト名を先読みして表示（ロード中はプレースホルダーを表示）。※現在は設定(`EnableNextTrack`)による隠し機能となっており、デフォルトでは非表示。
    *   下部：シークバー、およびマウスをホバーさせた時のみフェードインして浮かび上がる再生コントロール（前の曲、再生/一時停止、次の曲）と音量コントロール（スピーカーアイコンとパーセンテージ表示）。ホバー領域に入るとシークバー本体は視認性向上のため50%減光する。また、マウスホイールで直感的な音量調整が可能。
    *   プレースホルダー：アルバムアート未設定時のデフォルト画像を用意。
    *   テキストトリミング：曲名、アーティスト名、時間表示などの各種テキストは、表示領域を超過した場合に「...」で省略表示（トリミング）される。
    *   **設定/システムトレイアイコン**: 設定はシステムトレイアイコンに集約させ、コンフィグウィンドウは限界まで作成しない。またシステムトレイアイコンからはアプリの終了、INIファイル・プレイリストのリセット機能などを入れる。画面サイズやレイアウトなどは、INIファイルを直接編集することでカスタマイズ可能とする。
    *   **スキン・リソース仕様**: 各種画像（アプリアイコン等）は、基本的にはexeの埋め込みリソース(`RCDATA` や `ICON`)を使用しますが、一部の画像はexeと同階層に同名の画像ファイルが存在する場合はそれを優先して読み込むハイブリッド（フォールバック）方式を採用しています。アプリアイコンには `app_icon.ico` が埋め込まれ、システムトレイおよびタスクバーで表示されます。
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
 │   └── Debug               # デバッグビルド出力先
 ├── src/                    # C++ソースコード
 │   ├── main.cpp            # エントリポイント(wWinMain)
 │   ├── Application.h/cpp   # アプリケーションライフサイクル・メインループ
 │   ├── AudioPlayer.h/cpp   # 音声再生エンジン (miniaudioラップ)
 │   ├── ConfigManager.h/cpp # 設定ファイル(OZtone.ini)の管理
 │   ├── miniaudio.h         # miniaudio シングルヘッダライブラリ
 │   ├── PlaylistManager.h/cpp # 再生待ちキュー（プレイリスト）管理
 │   ├── Renderer.h/cpp      # Direct3D11 / Direct2D ハイブリッド描画エンジン
 │   ├── Visualizer.h/cpp    # 7色ネオン心電図風スペクトラムビジュアライザ
 │   ├── resource.h          # リソースID定義ヘッダ
 │   ├── TagManager.h/cpp    # MP3メタデータ抽出クラス
 │   ├── Window.h/cpp        # Win32ウィンドウ管理・WndProc
 │   └── OZtone.rc           # 画像などを埋め込むリソーススクリプト
 ├── assets/                 # ボタン類などのリソース、テスト用音声ファイル
 ├── _docs/                  # 開発関連ドキュメント
 │   ├── logs/               # AIの実装レポート出力先 (YYYYMMDD_HHMM_タスク名.md)
 │   └── OzPlayer_Plan.txt   # 開発初期の要望メモ
 ├── PROJECT_ARCHITECTURE.md # 本ファイル（全体仕様・設計・アーキテクチャ）
 ├── PROJECT_CONSTITUTION.md # プロジェクト憲法（AI向け絶対ルール）
 └── README.md               # プロジェクト概要
```

---

### 4. 全体のルーチン（アーキテクチャと実行フロー）
1.  **エントリポイント (`wWinMain`)**:
    *   `CreateMutexW` を用いて多重起動を防止。すでにプロセスが存在する場合は、コマンドライン引数のパスを `WM_COPYDATA` で既存プロセスへ送信して自身は終了する。
    *   `OleInitialize` にて COM を初期化後、`Application` インスタンスを生成。初回起動で引数がある場合は `ProcessCommandLineArgs` にて読み込ませる。
2.  **初期化 (`Application::Initialize`)**: 内部で `Window::Initialize`、`Renderer::Initialize`、`AudioPlayer::Initialize` を順次呼び出しシステムの初期化を行う。
3.  **メインループ (`Application::Run`)**: `Window::ProcessMessages` を毎フレーム呼び出し、OSのメッセージを処理。
4.  **メッセージ処理 (`Window::WindowProc` / `DropTarget`)**: OSからのイベントを処理。`WM_DESTROY` の他、アプリアイコン上での `WM_LBUTTONDOWN`（ドラッグ移動）を処理。ファイルのドラッグ＆ドロップは `IDropTarget` を通じて処理される。

---

### 5. 実装済みクラス・関数リファレンス

#### `Application` クラス (src/Application.h, cpp)
アプリケーション全体のライフサイクルとメインループを統括するマネージャ。
*   **`bool Initialize(HINSTANCE hInstance, int nCmdShow)`**
    *   各サブシステム (`Window`, `Renderer`, `AudioPlayer`) の初期化を行う。また、`Window` からのリサイズコールバックを受け取り、`Renderer::Resize` へ伝達するルーティングを設定する。
    *   起動時、`ConfigManager` からプレイリストのパスを取得し `PlaylistManager::LoadFromFile` でリストを復元。曲があれば自動で最初の曲の再生と次曲の先読みを開始し、無ければUIを空状態(`No Track`)で初期化する。再生に失敗した場合は即座に次の曲へと自動スキップし、全曲エラー時は安全に空状態で待機する。
    *   初期化時に `ConfigManager` から `DefaultVolume` を読み込み `AudioPlayer` へ適用する。
*   **`void OnFilesDropped(const std::vector<std::wstring>& paths)`**
    *   アプリアイコン領域へファイルまたはフォルダがドロップされた際のコールバック処理。フォルダの場合は再帰的に中身を探索し、サポート対象の拡張子（`.mp3`, `.flac`, `.wav`, `.ogg`）および各フォーマットに応じたマジックナンバー検証を通過したファイルのみを `PlaylistManager` に追加する。また、実験的対応（`[EXPERIMENTAL]`）として動画コンテナ形式（`.mp4`, `.m4a`）も、マジックナンバー検証をスキップして通過させる。
    *   内部でのパス処理は `std::wstring`と `std::filesystem::path` を用いて行い、外部C-API（miniaudio, TagLib）への受け渡しもワイド文字列のまま行うことで、Unicode文字（Shift-JIS非対応文字など）を含むファイルパスに完全対応している。プレイリスト（.lst）の保存・読み込み時にはWin32 APIを用いてUTF-8へ相互変換を行うことで安全なファイルI/Oを維持している。
    *   追加後にプレイリストをファイル（`DefaultPlaylistPath`）へ自動出力し、もしキューが新規追加によって空から1曲以上になった場合は、即座に最初の曲の再生と次曲の先読みを開始する。再生失敗時は自動で次の曲を試みるフェイルセーフが働く。
*   **`void PrefetchNextTrack()`**
    *   次の曲のタグ情報（曲名・アーティスト名）およびアルバムアート画像（WICデコード済み `ID2D1Bitmap`）を先読みし、メモリ上に保持する（プリフェッチ）。この処理はメインスレッドをブロックしないよう `std::thread` を用いてバックグラウンドで実行され、完了時に `m_isPrefetchReady` を `true` に更新する。対象ファイルが存在しない、またはタグロードに失敗した場合でも例外を投げず、安全なフォールバックデータ（ファイル名からタイトルを抽出）をキャッシュにセットしスレッドクラッシュを防止する。
*   **`void Run()`**
    *   メインループ（ゲームループ）を実行する。ループ内で毎フレーム、先読み状態 (`m_isPrefetchReady`) とデータを `Renderer::SetNextTrackInfo` に渡し、UIに「次の曲」情報を表示させる。
    *   `AudioPlayer` の再生終了（`IsAtEnd`）を検知した場合、かつ `m_isPrefetchReady` が `true` になっていることを確認した上で、プレイリストを進め、先読みデータを `Renderer` の「現在の曲」情報として即時反映し、次の曲を再生しつつ更に次の曲を先読みする。これにより、重いWICデコード等による曲間移行時のUIブロックを防いでいる。
    *   ループ内では後述の `ForceRender` を毎フレーム呼び出し描画を行う。現在はCPU負荷軽減のためループ内に `Sleep(1)` を配置。
*   **`void ForceRender()`**
    *   現在時間やスペクトルデータの取得、UIホバー状態（プレイリストホバー状態も含む）の収集を行い、強制的に1フレーム分の描画処理 (`Renderer::Render`) を実行する。この際、`PlaylistManager` から現在のインデックス、総曲数、シャッフルリスト一覧を取得してRendererに渡し、状態を連携する。ウィンドウリサイズ時など、OSのモーダルメッセージループによってメインループ (`Run`) がブロックされている最中に、リアルタイムで描画を追従させるためにも呼び出される。
    *   **メディアキー連携**: `Window::SetMediaCommandCallback` を用いてコールバックを登録しており、`APPCOMMAND_MEDIA_PLAY_PAUSE`、`APPCOMMAND_MEDIA_STOP`、`APPCOMMAND_MEDIA_NEXTTRACK`、`APPCOMMAND_MEDIA_PREVIOUSTRACK` に応じて再生コントロール（再生/一時停止切替、停止と巻き戻し、前後の曲への移行）を行う。キーの入力検知は `Window` クラス内の低レベルキーボードフックでグローバルに行われる。
    *   **音量調整連携**: `Window::SetMouseWheelCallback` により、ホバー領域内でのマウスホイール回転（`WM_MOUSEWHEEL`）をフックし、`AudioPlayer` を通じた音量の増減（5%刻み）および設定（`DefaultVolume`）の自動保存を行う。
*   **`void ClearPlaylist()`**
    *   プレイリストのキューを完全に空にし、現在の再生を即座に停止してUIを初期状態（"No Track"）にリセットする。また、クリア後の空の状態をデフォルトプレイリストファイル（`oztone_playlist.lst`）に上書き保存し、次回起動時も空の状態から始まるように同期する。
*   **`void ProcessCommandLineArgs(int argc, LPWSTR* argv)`**
    *   起動時のコマンドライン引数（ファイルパス等）をパースし、有効なファイルを `OnFilesDropped` 経由でプレイリストに追加する。

#### `Window` クラス (src/Window.h, cpp)
Win32 APIのウィンドウ生成・破棄、メッセージディスパッチ処理を隠蔽・カプセル化するクラス。
*   **`bool Initialize(HINSTANCE hInstance, int nCmdShow)`**
    *   ウィンドウクラス (`WNDCLASSEXW`) の登録と、`CreateWindowExW` によるウィンドウ生成。`hIcon` および `hIconSm` に `IDI_APP_ICON` を指定し、EXEとタスクバーのアイコンを設定。
    *   **DirectComposition と レイヤードウィンドウの挙動対策**: `WS_EX_LAYERED` が指定されている場合、ウィンドウ生成直後に `SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA)` を呼び出している。これは、DComp使用時にOSがウィンドウの入力判定領域（Hit-test region）を初期サイズのまま自動更新しなくなるOSの既知の挙動を回避し、リサイズ後も右下グリップや下部UI等の領域で正常にマウスイベントを受け取るためのワークアラウンドである。
    *   `Shell_NotifyIcon` を用いてシステムトレイ（通知領域）にアプリのアイコンを常駐させる。
*   **`bool ProcessMessages()`**
    *   `PeekMessage` を使用したノンブロッキングなメッセージ処理。`WM_QUIT` 受信で false を返し、メインループを終了させる。
*   **`LRESULT CALLBACK WindowProcStatic(HWND, UINT, WPARAM, LPARAM)`**
    *   Win32 APIから直接呼び出される静的コールバック関数。`WM_NCCREATE` 時に `GWLP_USERDATA` に自身の `this` ポインタを保存し、以降のメッセージではポインタを復元して非静的な `WindowProc` へ処理を転送する。
*   **`LRESULT WindowProc(HWND, UINT, WPARAM, LPARAM)`**
    *   `WM_PAINT` 時は `Renderer::Render` を呼び出す。
    *   `WM_MOUSEMOVE`、`WM_MOUSELEAVE` 等でマウス位置を追跡し、ロゴ領域や再生コントロール領域へのホバー状態（フェード用）を管理する。
    *   **プレイリストのホバー制御と排他制御**: `WM_MOUSEMOVE` 時に `IsInPlaylistRegion` を用いてプレイリスト領域へのホバーを判定する。プレイリスト展開中は、ホバー判定幅を `PlaylistHoverWidth` からリスト全体の幅に動的拡張するとともに、画面下部UIとの干渉回避用Y座標制限を無効化し、リスト全域でホバー状態を維持する。また、プレイリストホバー時は、背後の再生コントロールや音量調整の判定をスキップする排他制御（`WM_LBUTTONDOWN`, `WM_MOUSEWHEEL` での早期リターンやホバー状態の強制無効化）を行い、誤動作を防ぐ。
    *   **リサイズモードと最小サイズ制限**: `WM_NCHITTEST` を捕捉し、設定（`EnableResize`）が有効な場合はウィンドウ右下隅でリサイズカーソル（`HTBOTTOMRIGHT`）を返す。また、`WM_SIZE` でリアルタイムなサイズ変更イベントをコールバックで `Application` へ通知し、スワップチェインのリサイズ直後に強制再描画（`ForceRender`）を呼び出すことで、リサイズドラッグ中も描画を追従させる。また、`WM_GETMINMAXINFO` メッセージを捕捉し、現在のDPIに基づいてスケーリングされた論理限界サイズ（Width: 495, Height: 260）を下回らないようにOSレベルで縮小を制限している。
    *   システムトレイのアイコンからのメッセージ (`WM_TRAYICON`) を処理し、右クリック時にコンテキストメニューを表示する。メニューの定義（構築順序）は `Window.cpp` 先頭の `TRAY_MENU_ORDER` 配列で行われており、この配列を編集することで容易に項目の追加・入れ替えが可能となっている。背景の表示モードやZオーダーの切り替え、リサイズモードの有効化、プレイリストのクリア等もここから行う。
    *   **Zオーダーとフォーカス制御**: `WM_WINDOWPOSCHANGING` を捕捉し、Zオーダーが「最背面 (Bottom)」に設定されている場合は `HWND_BOTTOM` を強制して手前への移動を防いでいる。同時に `WM_MOUSEACTIVATE` で `MA_NOACTIVATE` を返すことで、クリックによるフォーカス奪取や意図しないアクティブ化を防止している。
    *   **メディアキーの処理 (低レベルキーボードフック)**: `WH_KEYBOARD_LL` を用いた低レベルキーボードフック (`LowLevelKeyboardProc`) により、アプリケーションが非アクティブな状態でもメディアキーの入力をグローバルに捕捉する。捕捉したキーイベントは `WM_APP_MEDIAKEY` カスタムメッセージとして変換・送信され、登録されたコールバック関数へ委譲される。
    *   **多重起動防止とプロセス間通信**: `WM_COPYDATA` を捕捉し、外部プロセス（2つ目以降の起動インスタンス）から送信されたファイルパス文字列を受け取り、登録されたコールバック(`SetCopyDataCallback`)を通じて本体のプレイリストへ追加する。
*   **`DropTarget` クラス (IDropTarget 実装)**
    *   OLE Drag and Drop によるファイルドロップを受け付けるクラス。`DragEnter` や `DragOver` を処理し、ドラッグ中であってもロゴ領域内にマウスがある場合は `Window` 側のホバー状態を強制的にオンにすることで、ドラッグ中の視覚的フィードバック（ハイライト）を実現している。
    *   **`bool IsInLogoRegion(int x, int y) const`**, **`bool IsInPlaybackControlRegion(int x, int y) const`**, **`bool IsInVolumeControlRegion(int x, int y) const`**, **`int GetPlaybackButtonAt(int x, int y) const`**
    *   指定された物理座標(x, y)を、DPIスケールを用いて論理座標に変換してから各領域内かどうかを判定する。画面下部のUI領域（再生・音量コントロール等）や右下リサイズグリップの判定では、固定の初期サイズではなく `GetClientRect` で現在の動的なクライアント領域のサイズを取得し、それを基準に相対的な領域計算を行うことで、リサイズ後も正確な座標判定を維持している。
*   **ウィンドウ位置・サイズの保存処理**
    *   `WM_DESTROY` 時に `GetWindowRect` と `GetClientRect` からウィンドウ位置・サイズを取得し、DPIスケールで論理サイズに変換して `ConfigManager` へ保存する。

#### `ConfigManager` クラス (src/ConfigManager.h, cpp)
アプリケーションの設定（`OZtone.ini`）の読み込み、およびデフォルト設定の書き出しを管理する。
*   **`bool Initialize()`**
    *   実行ファイルと同階層の `OZtone.ini` を解決し、存在しなければデフォルト値で作成する。その後、各設定値を読み込む。
*   ウィンドウ表示フラグ (`GetShowTitleBar` 等) やウィンドウの論理位置・サイズ (`GetWindowX` 等)、アプリ全体の視覚効果 (`GetEnableShadow`, `GetBgDarkenOpacity`, `GetBgOpacity` 等)、リサイズモード (`GetEnableResize` 等)、背景の表示モード（`GetBackgroundArtMode` 等）、ビジュアライザのモード (`GetVisualizerMode` 等)、および各種UIの表示フラグ（`GetShowAppLogo`, `GetShowNowPlaying`, `GetShowNextTrack`, `GetShowSeekBar`, `GetShowPlaybackControls`, `GetShowVolumeControl` 等）、アプリアイコンやアルバムアートのレイアウト情報 (`GetLogoX` 等, `GetBaseX` 等, `GetBaseBottomOffset` 等, `GetFallbackArtOpacity` 等)、再生コントロールのレイアウト設定 (`GetPlaybackBaseBottomOffset`, `ControlHoverHeight` 等)、音量コントロールの設定 (`GetVolumeIconSize`, `TextOffsetX`, `TextLetterSpacing`, 影設定等)、プレイリストUIの各テキスト（タイトル・アーティスト名・再生時間）の独立したフォント・色・オフセット設定およびスライドイン等のレイアウト設定 (`GetPlaylistTimeOffsetX`, `GetPlaylistWidth`, `PlaylistHoverWidth`, `PlaylistArtistColor`, `PlaylistTimeColor` 等)、各テキストのフォントやレイアウト（`GetTitleFontFamily` 等）、シークバーのレイアウトと文字間隔設定（`GetSeekBarWidthRatio`, `GetSeekBarTimeLetterSpacing` 等）、および「次の曲」表示のレイアウト設定・隠し機能フラグ (`GetNextBaseRightOffset`, `GetEnableNextTrack` 等) のゲッターとセッターを提供する。
*   **`int GetZOrder() const`, `void SetZOrder(int zOrder)`**
    *   ウィンドウのZオーダー設定（0: 通常, 1: 最前面, 2: 最背面）の取得と更新を行う。
*   **`bool GetSavePositionOnExit() const`, `void SetSavePositionOnExit(bool save)`**
    *   終了時にウィンドウ位置とサイズを自動保存するかどうかのフラグを取得・更新する。
*   **`void SaveDefaultSettings()`**
    *   初期（デフォルト）の設定値で `OZtone.ini` を上書き保存する。設定の完全初期化などで使用される。
*   **`std::wstring GetDefaultPlaylistPath() const`**
    *   起動時に読み込むデフォルトのプレイリストファイルのフルパスを取得する。設定が存在しない場合、初期値（`実行ファイルと同階層/oztone_playlist.lst`）を生成し、直ちにINIファイルへ書き出してユーザーが確認・編集できるようにする。
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
*   **`bool LoadBitmapFromMemory(const std::vector<uint8_t>& data, ID2D1Bitmap** ppBitmap)`**
    *   `TagManager` から取得したバイナリデータ等から動的にD2Dビットマップを生成する。
*   **`void SetTrackInfo(const std::wstring& title, const std::wstring& artist)`**, **`void SetAlbumArt(ID2D1Bitmap* bitmap)`**, **`void SetNextTrackInfo(bool isReady, ID2D1Bitmap* art, const std::wstring& title, const std::wstring& artist)`**
    *   外部から曲名・アーティスト名およびアルバムアート画像を設定する。また、先読みされた「次の曲」の情報も設定する。
*   **`void Resize(UINT width, UINT height)`**
    *   ウィンドウのリサイズに伴い、DirectComposition のターゲット指定を解除し、バックバッファを解放した上でスワップチェインの `ResizeBuffers` を実行し、D2Dターゲットを安全に再構築する。
*   **`void Render(bool isHovered, bool isControlHovered, bool isPlaylistHovered, bool isPlaying, float progress, const std::wstring& timeString, const std::vector<float>& spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<std::wstring>& shuffleList)`**
    *   毎フレーム呼び出され、描画処理を実行する。`BeginDraw()` の直後に `SetTransform` によって `m_dpiScale` を適用し、以降の描画はすべて論理ピクセルのまま物理ピクセルへ自動拡大される。
    *   完全透明でクリアした後、`BackgroundArtMode` の設定値（0:再生中, 1:非表示, 2:デフォルト固定）に従って背景アルバムアートを描画し、設定に応じてダークオーバーレイ（全画面の黒半透明矩形）を描画して視認性を確保する。
    *   その後、`Visualizer` クラスを用いて、背景アートとUIの間にビジュアライザを描画する（`VisualizerMode` が `0` 以外の場合のみ呼び出し、`0` 時は負荷ゼロでスキップする）。
    *   手前にアプリアイコン、曲情報テキスト(曲名・アーティスト名)、進行度(`progress`)に応じたシークバーと現在時間(`timeString`)、および右下の「次の曲」情報（アート、曲名、アーティスト名、Next Trackラベル）を描画する。各UI要素は `ConfigManager` から取得した `Visibility` フラグが有効な場合のみ描画され、無効時は完全に処理がスキップされる。また、各テキストは領域超過時に「...」でトリミングされる。「次の曲」表示は隠しフラグ (`EnableNextTrack`) が有効な場合のみ描画され、非表示時は現在の曲情報が画面右端まで広く描画される。シークバーと時間テキストは、UIホバー状態（`m_controlAlpha`）に応じて輝度が最大50%減衰する。各UI要素には設定によりドロップシャドウが付与される。
    *   画面下部全体へのマウスホバー時、フェードアニメーションとともに `ID2D1PathGeometry` を用いて白のベクターアイコンで再生コントロール（前の曲、再生/一時停止、次の曲）および音量コントロール（スピーカーアイコンとテキスト）を描画する。再生・音量UIもそれぞれ個別の表示フラグにより無効時は描画をスキップする。音量UIにもオフセットや文字間隔調整、ドロップシャドウが適用される。
    *   画面右下の「TRACK XXX/XXX」の表示領域にマウスホバーした際、右端からシャッフル再生順リスト（プレイリスト）がスライドインするアニメーション描画を行う。スクロール制御や現在曲のハイライト処理を伴い、再生コントロール領域等のホバー判定より優先してポップアップ表示される。プレイリスト内の各項目は、タイトル、アーティスト名、再生時間それぞれのテキストフォーマットとカラー（`PlaylistArtistColor`, `PlaylistTimeColor`）が独立して管理・適用される。TRACK数表示および再生時間は右寄せ（`DWRITE_TEXT_ALIGNMENT_TRAILING`）で描画され、TRACK数の影には専用のプロパティ（`TrackCountShadowOpacity`等）が適用される。特に再生時間はアイテム右下を基準とした動的な座標計算と文字間隔適用が行われる。
    *   リサイズモード (`EnableResize`) が有効な場合のみ、ウィンドウ右下隅に視覚的な手がかりとして半透明なリサイズグリップ（◢）を描画する。
    *   **フォールバック処理**: 正規のアルバムアート画像が未設定の場合、背景にはプレースホルダー画像(`IDI_PLACEHOLDER_ART`)が描画されるが、左下のサムネイル領域にはプレースホルダーを描画せず、`FallbackArtOpacity` のアルファ値を持つ黒い板（`FillRectangle`）を描画する。時間文字列の描画には `IDWriteTextLayout` を使用し、ConfigManager から取得した文字間隔を `SetCharacterSpacing` で適用している。

#### `Visualizer` クラス (src/Visualizer.h, cpp)
スペクトルデータを元に、「7色ネオンの心電図風」または「円形パーティクル」のビジュアライザを描画するモジュール。
*   **`bool Initialize(ID2D1DeviceContext* context)`**
    *   7色のネオン用半透明ブラシ（赤、オレンジ、黄、緑、水色、青、紫）および芯用の白色ブラシを初期化する。
*   **`void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist)`**
    *   設定 (`ConfigManager::GetVisualizerMode()`) の値（1: PrismBeat, 2: Halo Dust）に応じて、内部で `DrawPrismBeat` または `DrawCircleParticle` に描画処理を委譲する。
    *   起動直後の「No Track」状態など、引数のスペクトルデータが空あるいは要素数が少ない場合（`spectrum.size() < 2`）は、配列外アクセスやアサーションエラーによるクラッシュを防ぐために各描画関数の先頭で即座にリターン（描画スキップ）する安全対策が施されている。
*   **`void DrawPrismBeat(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect)`**
    *   8192サイズの高解像度スペクトルデータを、人間の耳の特性に合わせた対数スケール（上限10000Hz）でマッピングする直線型のオシロスコープ表現。
    *   `m_smoothedAmplitudes` を用いて「アタック（即時跳ね上がり）とディケイ（15%減衰）」による重力スムージングを行い、音楽的なビート感と余韻を表現する。
    *   `WAVE_STEP_X` でX座標を間引きながら、隣接する頂点のY座標を交互に反転(上・下)させて心電図の脈打ちを作る。
    *   色の割り当ては周波数ではなく「画面X座標の均等7分割」で行い、低音域から高音域まで美しいカラーバランスを維持。
    *   左右5%のマージン領域に対しても連続して波形計算を行い、画面端に向かって振幅とアナログノイズを滑らかにフェードアウト（`fade * fade`）させ、一直線へ自然に収束させるダミー表現を組み込む。
    *   `ID2D1PathGeometry` を用いて一筆書き(`D2D1_FIGURE_BEGIN_HOLLOW` / `D2D1_FIGURE_END_OPEN`)でパスを構築し、太い線(グロー)と細い線(芯)の二重描画でネオンのような発光効果を実現する。
*   **`void DrawCircleParticle(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist)`**
    *   中央に配置される円形ビジュアライザ表現（名称：Halo Dust）。曲名とアーティスト名の文字列ハッシュから固有のテーマカラー（HSVベース）を生成し、ベースカラー（#888888）と加算合成（クランプ）することで、白く眩しく発光する「真のネオン質感」を実現している。
    *   全64バンドの周波数帯に対してバーを放射状に描画。低音域の減衰や高音域の強調補正を行い、`ID2D1PathGeometry` で台形を描画して2パス（半透明のグローと白のコア）で縁取る。
    *   ビート（振幅の瞬間的な跳ね上がり）を検知すると、中心から放射状に飛散するレーザー(`LaserRay`)や3D回転する破片パーティクル(`Particle`)を全方位(360度)のランダムな角度に向けて放出する。
    *   パーティクルはCPU上でロドリゲスの回転公式を用いて各頂点の3D座標計算を行い、2Dに投影して描画している。
    *   `Visualizer.cpp` の先頭に専用の「ビジュアル設定」ブロックを設け、波形の振幅や円のサイズ、レーザー/パーティクルの速度・寿命、透明度などのパラメータを定数として集約し、容易に調整可能な構造としている。

#### `AudioPlayer` クラス (src/AudioPlayer.h, cpp)
音声処理ライブラリ `miniaudio` をラップし、バックグラウンドでのMP3再生を管理するクラス。
*   **`bool Initialize()`**
    *   `ma_engine` をインスタンス化し、`ma_engine_init` で初期化する。
*   **`void Uninitialize()`**
    *   ロード済みのサウンド（`ma_sound`）を破棄し、初期化済みであれば `ma_engine_uninit` を呼び出してエンジンリソースを解放する。
*   **`bool Play(const std::wstring& filepath)`**
    *   指定されたMP3ファイルを `ma_sound_init_from_file` でロードし、`ma_sound_start` にて再生する。既にロード済みの場合は一度破棄してから再ロードする。再生開始直前に `ma_sound_get_length_in_seconds` を呼び出して曲長をキャッシュする。
*   **`void SetVolume(float volume)`, `float GetVolume() const`**
    *   `ma_engine_set_volume` を用いてエンジン全体の音量を設定・取得する。
*   **`float GetPositionSeconds()`**
    *   現在の再生位置（秒）を取得して返す。
*   **`float GetLengthSeconds()`**
    *   曲の総時間（秒）を取得して返す。毎フレーム取得による内部デコード競合（クラッシュ）を防ぐため、`Play` 時に取得したキャッシュされた値を返す。
*   **`void TogglePlayPause()`**
    *   現在の再生状態を反転（再生中なら一時停止、一時停止中なら再生）させる。
*   **`void Stop()`**
    *   再生を停止し、シーク位置を曲の先頭に巻き戻す。
*   **`bool IsPlaying()`**
    *   現在再生中かどうかを返す。
*   **`bool IsAtEnd()`**
    *   現在の音声が最後まで再生されたか(EOF)を返す。
*   **`void ProcessAudioFrames(const float* pFrames, ma_uint64 frameCount, ma_uint32 channels)`**, **`void GetSpectrumData(std::vector<float>& outSpectrum)`**
    *   無名名前空間に定義したコールバック関数 `AudioPlayerOnProcess` を `ma_engine_config` の `onProcess` に登録してミックス後のPCMオーディオデータをキャプチャし、内部でC++ネイティブ実装の高速フーリエ変換（FFT）処理を行って周波数スペクトルデータを生成・保持する。
    *   **高解像度化**: FFTサイズは `8192` を採用しており、低音域においても十分な解像度を確保している。
    *   生成されたデータは `GetSpectrumData` を通じてメインスレッドからスレッドセーフに取得される。
#### `TagManager` クラス (src/TagManager.h, cpp)
`TagLib` をラップし、MP3などの音声ファイルからID3タグやメタデータを抽出する機能を提供するクラス。
*   **`bool Load(const std::wstring& filepath)`**
    *   指定されたファイルパスのメタデータを `TagLib::MPEG::File` を用いて読み込む。`MPEG::File` 一本でタイトル・アーティスト名（`tag()`経由）とアルバムアート（`ID3v2Tag()`→`APIC`フレーム経由）を同時に取得する。
    *   **設計上の注意**: Windows環境では同一ファイルを `TagLib::FileRef` と `TagLib::MPEG::File` で二重にオープンするとファイルロック競合が発生し、2回目のオープンが失敗する。そのため、汎用の `FileRef` は使用せず `MPEG::File` に統一している。
*   **`std::wstring GetTitle() const`, `std::wstring GetArtist() const`**
    *   ロードされたメタデータから曲名、アーティスト名を取得して返す。
*   **`const std::vector<uint8_t>& GetAlbumArtBytes() const`**
    *   ID3v2の `APIC` (Attached Picture Frame) からアルバムアートのバイナリデータを取得して返す。画像が存在しない場合は空のベクターを返す。

#### `PlaylistManager` クラス (src/PlaylistManager.h, cpp)
再生待ちキュー（プレイリスト）を管理するクラス。**常時シャッフル再生**および**ダブルバッファリングによるシームレスなループ**をサポートする。
*   **`size_t GetCurrentIndex() const`, `std::vector<std::wstring> GetShuffleList() const`**
    *   現在のシャッフルインデックスと、次に再生される順番のプレイリスト全体を取得する。
*   **`bool Add(const std::wstring& filepath)`**
    *   指定されたファイルパス（絶対パス）をプレイリストに追加する。すでに存在する場合はスキップする。
    *   追加時、現在のシャッフルループおよび次回のシャッフルループの末尾に、シャッフルせずにそのまま追加する。これにより、再生中のキューにドロップした曲が即座に末尾に積まれる。
*   **`std::wstring GetCurrentTrack() const`**
    *   現在のシャッフルインデックスが指す曲のファイルパスを返す。
*   **`void Advance()`**
    *   キューを次の曲へ進める。現在のシャッフルリストの末尾まで到達した場合は、次周用のシャッフルリスト (`m_nextShuffleIndices`) を現在のリストに昇格し、次周リストを新たに再生成してループする。
*   **`void Previous()`**
    *   キューを1つ前の曲へ戻す。現在のリストの先頭の場合は、リストの末尾（要素数 - 1）へループする。
*   **`std::wstring GetNextTrack() const`**
    *   次に再生される予定のファイルパスを取得して返す。現在のループの末尾にいる場合は、次周のシャッフルリストの先頭曲を返すことで、先読み処理との完全な整合性を保つ。
*   **`void SaveToFile(const std::wstring& outPath) const`**
    *   現在のプレイリストの実体をテキストファイルに出力する。
*   **`void LoadFromFile(const std::wstring& inPath)`**
    *   ファイルからプレイリストを読み込む。完了後、現在および次周のシャッフルリストを再生成 (`InitializeShuffle`) し、完全にシャッフルされた状態にする。
*   **`void ShuffleNextLoop()`**
    *   次回のループ順序のみをシャッフルし直す。D&Dで曲が追加された直後などに呼ばれ、現在の再生順序を乱さずに以降のランダム性を確保する。
*   **`void Clear()`**
    *   内部のプレイリスト（曲リスト、重複チェック用セット、シャッフルインデックスなど）を全て空に初期化する。
*   **`bool IsEmpty() const`**
    *   プレイリストが空かどうかを返す。

---

### 6. 定数・設定値 (実装済みデータ)
*   **ウィンドウ初期サイズ**: `ConfigManager` (INIファイル) から `GetWindowWidth()`, `GetWindowHeight()` を通じて論理サイズを取得し、システムDPIでスケーリングして物理サイズを決定。
*   **ウィンドウスタイル**: `WS_POPUP | WS_EX_LAYERED` (完全枠なし・レイヤードウィンドウ。ConfigManager の設定に応じて動的に決定)
*   **ウィンドウクラス名**: `L"OZtoneWindowClass"`
*   **ウィンドウタイトル**: `L"OZtone"`

---

### 7. 現在の仮実装項目（要リファクタリング・置換対象）
後続のステップで本格的な機能に置き換えられた際に、以下の項目は削除または大幅にリファクタリングされる想定です。作業前にここをチェックしてください。

*   **`.mp4` / `.m4a` フォーマットの実験的対応**: `Application::OnFilesDropped` において、これらの動画コンテナフォーマットに対しては複雑なマジックナンバー検証をスキップし、無条件でプレイリストへ通過させる暫定処理を組み込んでいます。将来的には専用の検証ロジックを実装するか、より堅牢なメディア解析処理へ置き換えられる想定です。(`// [EXPERIMENTAL] MP4/M4A Support` コメントにて隔離済み)
