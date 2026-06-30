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
 │   └── Debug               # デバッグビルド出力先
 │       └── playlists/      # (実行時生成) 現在のプレイリストなどのテキスト保存先
 ├── src/                    # C++ソースコード
 │   ├── main.cpp            # エントリポイント(wWinMain)
 │   ├── Application.h/cpp   # アプリケーションライフサイクル・メインループ
 │   ├── AudioPlayer.h/cpp   # 音声再生エンジン (miniaudioラップ)
 │   ├── ConfigManager.h/cpp # 設定ファイル(OZtone.ini)の管理
 │   ├── miniaudio.h         # miniaudio シングルヘッダライブラリ
 │   ├── PlaylistManager.h/cpp # 再生待ちキュー（プレイリスト）管理
 │   ├── Renderer.h/cpp      # Direct3D11 / Direct2D ハイブリッド描画エンジン
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
1.  **エントリポイント (`wWinMain`)**: `OleInitialize` にて COM を初期化後、`Application` インスタンスを生成。
2.  **初期化 (`Application::Initialize`)**: 内部で `Window::Initialize`、`Renderer::Initialize`、`AudioPlayer::Initialize` を順次呼び出しシステムの初期化を行う。
3.  **メインループ (`Application::Run`)**: `Window::ProcessMessages` を毎フレーム呼び出し、OSのメッセージを処理。
4.  **メッセージ処理 (`Window::WindowProc` / `DropTarget`)**: OSからのイベントを処理。`WM_DESTROY` の他、アプリアイコン上での `WM_LBUTTONDOWN`（ドラッグ移動）や `WM_RBUTTONDOWN`（アプリ終了）を処理。ファイルのドラッグ＆ドロップは `IDropTarget` を通じて処理される。

---

### 5. 実装済みクラス・関数リファレンス

#### `Application` クラス (src/Application.h, cpp)
アプリケーション全体のライフサイクルとメインループを統括するマネージャ。
*   **`bool Initialize(HINSTANCE hInstance, int nCmdShow)`**
    *   各サブシステム (`Window`, `Renderer`, `AudioPlayer`) の初期化を行う。
    *   起動時、`ConfigManager` からプレイリストのパスを取得し `PlaylistManager::LoadFromFile` でリストを復元。曲があれば自動で最初の曲の再生と次曲の先読みを開始し、無ければUIを空状態(`No Track`)で初期化する。再生に失敗した場合は即座に次の曲へと自動スキップし、全曲エラー時は安全に空状態で待機する。
*   **`void OnFilesDropped(const std::vector<std::wstring>& paths)`**
    *   アプリアイコン領域へファイルまたはフォルダがドロップされた際のコールバック処理。フォルダの場合は再帰的に中身を探索し、`.mp3` 拡張子とマジックナンバー(ID3 or 0xFF)検証を通過したファイルのみを `PlaylistManager` に追加する。
    *   内部でのパス処理は `std::wstring` と `std::filesystem::path` を用いて行い、外部C-API（miniaudio, TagLib）への受け渡し時にのみACPエンコード（`.string()`）に変換することで、Windows環境でのマルチバイト文字（日本語など）の文字化けを防いでいる。
    *   追加後にプレイリストをファイル（`DefaultPlaylistPath`）へ自動出力し、もしキューが新規追加によって空から1曲以上になった場合は、即座に最初の曲の再生と次曲の先読みを開始する。再生失敗時は自動で次の曲を試みるフェイルセーフが働く。
*   **`void PrefetchNextTrack()`**
    *   次の曲のタグ情報（曲名・アーティスト名）およびアルバムアート画像（WICデコード済み `ID2D1Bitmap`）を先読みし、メモリ上に保持する（プリフェッチ）。この処理はメインスレッドをブロックしないよう `std::thread` を用いてバックグラウンドで実行され、完了時に `m_isPrefetchReady` を `true` に更新する。対象ファイルが存在しない、またはタグロードに失敗した場合でも例外を投げず、安全なフォールバックデータ（ファイル名からタイトルを抽出）をキャッシュにセットしスレッドクラッシュを防止する。
*   **`void Run()`**
    *   メインループ（ゲームループ）を実行する。ループ内で `AudioPlayer` の再生終了（`IsAtEnd`）を検知した場合、かつ `m_isPrefetchReady` が `true` になっていることを確認した上で、プレイリストを進め、先読みデータを `Renderer` に即時反映し、次の曲を再生しつつ更に次の曲を先読みする。これにより、重いWICデコード等による曲間移行時のUIブロックを防いでいる。
    *   曲の移行時に `AudioPlayer::Play` が失敗した場合は自動的に次の曲へスキップする。スキップが連続してプレイリストの総曲数に達した場合（無限ループ防止）は即座に再生処理を中止し、UIを空状態に戻して安全な待機状態に移行する。
    *   進捗率（progress）と時間表示文字列も計算して `Renderer::Render` に渡す。現在はCPU負荷軽減のためループ内に `Sleep(1)` を配置。

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
    *   アプリアイコン上でのクリックイベント（ドラッグ移動、右クリック終了）や通常のホバー状態の判定を処理する。
*   **`DropTarget` クラス (IDropTarget 実装)**
    *   OLE Drag and Drop によるファイルドロップを受け付けるクラス。`DragEnter` や `DragOver` を処理し、ドラッグ中であってもロゴ領域内にマウスがある場合は `Window` 側のホバー状態を強制的にオンにすることで、ドラッグ中の視覚的フィードバック（ハイライト）を実現している。
*   **`bool IsInLogoRegion(int x, int y) const`**
    *   指定された物理座標(x, y)を、DPIスケールを用いて論理座標に変換してから、アプリアイコンの論理レイアウト領域内かどうかを判定する。
*   **ウィンドウ位置・サイズの保存処理**
    *   `WM_DESTROY` 時に `GetWindowRect` と `GetClientRect` からウィンドウ位置・サイズを取得し、DPIスケールで論理サイズに変換して `ConfigManager` へ保存する。

#### `ConfigManager` クラス (src/ConfigManager.h, cpp)
アプリケーションの設定（`OZtone.ini`）の読み込み、およびデフォルト設定の書き出しを管理する。
*   **`bool Initialize()`**
    *   実行ファイルと同階層の `OZtone.ini` を解決し、存在しなければデフォルト値で作成する。その後、各設定値を読み込む。
*   ウィンドウ表示フラグ (`GetShowTitleBar` 等) やウィンドウの論理位置・サイズ (`GetWindowX` 等)、アプリアイコンやアルバムアートのレイアウト情報 (`GetLogoX` 等, `GetBaseX` 等, `GetBgOpacity` 等, `GetFallbackArtOpacity` 等)、およびシークバーのレイアウトと文字間隔設定（`GetSeekBarWidthRatio`, `GetSeekBarTimeLetterSpacing` 等）のゲッターを提供する。
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
*   **`void SetTrackInfo(const std::wstring& title, const std::wstring& artist)`**, **`void SetAlbumArt(ID2D1Bitmap* bitmap)`**
    *   外部から曲名・アーティスト名およびアルバムアート画像を設定する。`bitmap` に `nullptr` を渡すと正規画像が未設定状態となる。
*   **`void Render(bool isHovered, float progress, const std::wstring& timeString)`**
    *   毎フレーム呼び出され、描画処理を実行する。`BeginDraw()` の直後に `SetTransform` によって `m_dpiScale` を適用し、以降の描画はすべて論理ピクセルのまま物理ピクセルへ自動拡大される。
    *   完全透明でクリアした後、背景アルバムアート、アプリアイコン、曲情報テキスト(曲名・アーティスト名)、および進行度(`progress`)に応じたシークバーと現在時間(`timeString`)を描画する。
    *   **フォールバック処理**: 正規のアルバムアート画像が未設定の場合、背景にはプレースホルダー画像(`IDI_PLACEHOLDER_ART`)が描画されるが、左下のサムネイル領域にはプレースホルダーを描画せず、`FallbackArtOpacity` のアルファ値を持つ黒い板（`FillRectangle`）を描画する。時間文字列の描画には `IDWriteTextLayout` を使用し、ConfigManager から取得した文字間隔を `SetCharacterSpacing` で適用している。

#### `AudioPlayer` クラス (src/AudioPlayer.h, cpp)
音声処理ライブラリ `miniaudio` をラップし、バックグラウンドでのMP3再生を管理するクラス。
*   **`bool Initialize()`**
    *   `ma_engine` をインスタンス化し、`ma_engine_init` で初期化する。
*   **`void Uninitialize()`**
    *   ロード済みのサウンド（`ma_sound`）を破棄し、初期化済みであれば `ma_engine_uninit` を呼び出してエンジンリソースを解放する。
*   **`bool Play(const std::string& filepath)`**
    *   指定されたMP3ファイルを `ma_sound_init_from_file` でロードし、`ma_sound_start` にて再生する。既にロード済みの場合は一度破棄してから再ロードする。再生開始直前に `ma_sound_get_length_in_seconds` を呼び出して曲長をキャッシュする。
*   **`float GetPositionSeconds()`**
    *   現在の再生位置（秒）を取得して返す。
*   **`float GetLengthSeconds()`**
    *   曲の総時間（秒）を取得して返す。毎フレーム取得による内部デコード競合（クラッシュ）を防ぐため、`Play` 時に取得したキャッシュされた値を返す。
*   **`bool IsPlaying()`**
    *   現在再生中かどうかを返す。
*   **`bool IsAtEnd()`**
    *   現在の音声が最後まで再生されたか(EOF)を返す。

#### `TagManager` クラス (src/TagManager.h, cpp)
`TagLib` をラップし、MP3などの音声ファイルからID3タグやメタデータを抽出する機能を提供するクラス。
*   **`bool Load(const std::string& filepath)`**
    *   指定されたファイルパスのメタデータを `TagLib::MPEG::File` を用いて読み込む。`MPEG::File` 一本でタイトル・アーティスト名（`tag()`経由）とアルバムアート（`ID3v2Tag()`→`APIC`フレーム経由）を同時に取得する。
    *   **設計上の注意**: Windows環境では同一ファイルを `TagLib::FileRef` と `TagLib::MPEG::File` で二重にオープンするとファイルロック競合が発生し、2回目のオープンが失敗する。そのため、汎用の `FileRef` は使用せず `MPEG::File` に統一している。
*   **`std::wstring GetTitle() const`, `std::wstring GetArtist() const`**
    *   ロードされたメタデータから曲名、アーティスト名を取得して返す。
*   **`const std::vector<uint8_t>& GetAlbumArtBytes() const`**
    *   ID3v2の `APIC` (Attached Picture Frame) からアルバムアートのバイナリデータを取得して返す。画像が存在しない場合は空のベクターを返す。

#### `PlaylistManager` クラス (src/PlaylistManager.h, cpp)
再生待ちキュー（プレイリスト）を管理するクラス。
*   **`bool Add(const std::string& filepath)`**
    *   指定されたファイルパス（絶対パス）をプレイリストに追加する。すでにリストに存在する場合は追加をスキップ（重複排除）し `false` を返す。成功時は `true` を返す。
*   **`std::string GetCurrentTrack() const`**
    *   現在のインデックスに該当する曲のファイルパスを取得して返す。
*   **`size_t GetCount() const`**
    *   現在のプレイリストの全曲数を取得する。
*   **`void Advance()`**
    *   キューを次の曲へ進める。リスト末尾の場合は先頭へループする。
*   **`std::string GetNextTrack() const`**
    *   次に再生される予定のファイルパスを取得して返す。
*   **`void SaveToFile(const std::string& outPath) const`**
    *   現在のプレイリストの全パスを指定されたテキストファイルに出力する。
*   **`void LoadFromFile(const std::string& inPath)`**
    *   テキストファイルからプレイリストを読み込み、ファイルが実在するものだけをキューに復元する。ファイルが存在しない・破損している場合は単に空として扱いクラッシュを防ぐ。
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

*   （現在、特筆すべき仮実装項目はありません）
