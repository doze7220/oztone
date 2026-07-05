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
    *   左上：アプリアイコン（白フラット化したもの）を表示。ドロップ領域となる。また、ホバー時に右側へアニメーション展開する「ロゴ拡張メニュー」が備わっており、白単色の絵文字アイコンで構成される。トグルOFF時には半透明化＋赤い斜線で表現される（シャッフル切替ボタン等、一部動的にアイコンやテキストが切り替わるものもある）。
    *   **ウィンドウのドラッグ移動**: 特定のUI要素がない背景部分全体をドラッグすることでウィンドウを移動できる。
    *   ロゴ拡張メニューの機能：アプリ終了、プレイリスト固定トグル、ビジュアライザ3ステート切替（PrismBeat, Halo Dust, OFF）、シャッフル切替（「🔀」と「➡️」で動的切替）、プレイリスト左右配置切替。ホバー時には背景画像のフェードおよびグロー効果が付与され、上部に機能説明テキストが動的に表示される。
    *   左下：アルバムアート全体、曲名、アーティスト名。
    *   右下：「次の曲」の極小アルバムアートと曲名・アーティスト名を先読みして表示（ロード中はプレースホルダーを表示）。※現在は設定(`EnableNextTrack`)による隠し機能となっており、デフォルトでは非表示。
    *   下部：シークバー、およびマウスをホバーさせた時のみフェードインして浮かび上がる再生コントロール（前の曲、再生/一時停止、次の曲）と音量コントロール（スピーカーアイコンとパーセンテージ表示）。ホバー領域に入るとシークバー本体は視認性向上のため50%減光する。また、マウスホイールで直感的な音量調整が可能。
    *   **プレイリスト**: 画面左端または右端へのホバーで展開。領域は「曲一覧モード」と「プレイリスト一覧モード」の2階層構造を持つ。上部には固定ツールバーが配置され、モードに応じて「上の階層へ」「曲削除」「全曲削除」または「新規作成」「リスト削除」の操作が可能。ツールバーは上段にアイコン、下段に日本語の説明テキストが動的に表示される上下2段レイアウトである。
    *   プレースホルダー：アルバムアート未設定時のデフォルト画像を用意。
    *   テキストトリミング：曲名、アーティスト名、時間表示などの各種テキストは、表示領域を超過した場合に「...」で省略表示（トリミング）される。
    *   **設定/システムトレイアイコン**: 設定はシステムトレイアイコンに集約させ、コンフィグウィンドウは限界まで作成しない。またシステムトレイアイコンからはアプリの終了、画面固定（移動ロック）、INIファイル・プレイリストのリセット機能などを入れる。画面サイズやレイアウトなどは、INIファイルを直接編集することでカスタマイズ可能とする。
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
 │   ├── LayoutCalculator.h/cpp # 各UI要素のレイアウト（描画座標・矩形）計算クラス
 │   ├── miniaudio.h         # miniaudio シングルヘッダライブラリ
 │   ├── PlaylistManager.h/cpp # 再生待ちキュー（プレイリスト）管理
 │   ├── Renderer.h/cpp      # Direct3D11 / Direct2D ハイブリッド描画エンジン
 │   ├── Visualizer.h/cpp    # 7色ネオン心電図風スペクトラムビジュアライザ
 │   ├── resource.h          # リソースID定義ヘッダ
 │   ├── TagManager.h/cpp    # MP3メタデータ抽出クラス
 │   ├── Widget.h            # Widgetコアインターフェース・Context定義
 │   ├── Widgets.h/cpp       # 各種UI要素の具体Widget実装クラス
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
    *   追加後、`GetAsyncKeyState(VK_SHIFT)` によるキーボード状態の判定を行う。SHIFTキー押下時（BGM維持モード）は再生中の曲を止めずにキューをシャッフル更新（`ShuffleNextLoop`）するのみに留める。未押下時（即時再生モード）は、現在のシャッフル状態に応じたキューの再構築（`RebuildQueue`）を行い、最初に追加された曲へワープ（`WarpToTrack`）して即座に再生を開始する。
    *   また追加された未解析のトラックはバックグラウンド解析キュー（`m_parseQueue`）に積み、専用スレッド（`m_parseThread`）を起床させて非同期でのタグ解析を開始する。もしキューが新規追加によって空から1曲以上になった場合は、即座に最初の曲の再生と次曲の先読みを開始する。再生失敗時は自動で次の曲を試みるフェイルセーフが働く。
*   **`void PrefetchNextTrack()`**
    *   次の曲のタグ情報（曲名・アーティスト名）およびアルバムアート画像（WICデコード済み `ID2D1Bitmap`）を先読みし、メモリ上に保持する（プリフェッチ）。この処理はメインスレッドをブロックしないよう `std::thread` を用いてバックグラウンドで実行され、完了時に `m_isPrefetchReady` を `true` に更新する。対象ファイルが存在しない、またはタグロードに失敗した場合でも例外を投げず、安全なフォールバックデータ（ファイル名からタイトルを抽出）をキャッシュにセットしスレッドクラッシュを防止する。
*   **`void UpdateTrackMetadataIfNeeded(const std::wstring& filepath)`**
    *   対象曲のタグ情報を取得し、`PlaylistManager` 内のキャッシュメタデータ（`TrackMetadata`）と比較して自己修復を行うロジックが共通化されている。未解析（`isLoaded == false`）であるか、既存情報との齟齬があった場合、プレイリスト側の情報を上書き更新し、自動的にTSVキャッシュファイルへ保存する。`PrefetchNextTrack` での先読み時だけでなく、現在の曲の再生開始時などにも呼び出されるようになり、最新のタグ情報との自動同期をバックグラウンドで行う。
*   **`void ParseThreadFunc()` (バックグラウンド解析スレッド)**
    *   起動時やファイルドロップ時に追加された未解析トラックを専用キューから順次取り出し、スレッドローカルな `TagManager` で解析して `PlaylistManager::UpdateMetadata` により情報を補完する。これにより大量のファイル追加時にもUIスレッドをブロックしない。
*   **`void Run()`**
    *   メインループ（ゲームループ）を実行する。ループ内で毎フレーム、先読み状態 (`m_isPrefetchReady`) とデータを `Renderer::SetNextTrackInfo` に渡し、UIに「次の曲」情報を表示させる。
    *   `AudioPlayer` の再生終了（`IsAtEnd`）を検知した場合、かつ `m_isPrefetchReady` が `true` になっていることを確認した上で、プレイリストを進め、先読みデータを `Renderer` の「現在の曲」情報として即時反映し、次の曲を再生しつつ更に次の曲を先読みする。これにより、重いWICデコード等による曲間移行時のUIブロックを防いでいる。
    *   ループ内では後述の `ForceRender` を毎フレーム呼び出し描画を行う。現在はCPU負荷軽減のためループ内に `Sleep(1)` を配置。
    *   また、メインループ内で1秒間隔（1000ms）ごとに INIファイルの定期監視として `ConfigManager::CheckForUpdates` をポーリングし、設定ファイルの変更（ホットリロード）を検知した場合は `Renderer::ReloadResources` を呼び出してUIを動的に再構築するトップダウンのデータフローを構築している。
*   **`void ForceRender()`**
    *   現在時間やスペクトルデータの取得、UIホバー状態（プレイリストホバー状態も含む）の収集を行い、`Renderer::UpdateAnimation` と `Renderer::UpdateTextLayouts` を呼び出してアニメーション状態および変動テキストのレイアウトキャッシュを更新した後、強制的に1フレーム分の描画処理 (`Renderer::Render`) を実行する。この際、`PlaylistManager` から現在のインデックス、総曲数、シャッフルメタデータリスト一覧を取得してRendererに渡し、状態を連携する。ウィンドウリサイズ時など、OSのモーダルメッセージループによってメインループ (`Run`) がブロックされている最中に、リアルタイムで描画を追従させるためにも呼び出される。
    *   **メディアキー連携**: `Window::SetMediaCommandCallback` を用いてコールバックを登録しており、`APPCOMMAND_MEDIA_PLAY_PAUSE`、`APPCOMMAND_MEDIA_STOP`、`APPCOMMAND_MEDIA_NEXTTRACK`、`APPCOMMAND_MEDIA_PREVIOUSTRACK` に応じて再生コントロール（再生/一時停止切替、停止と巻き戻し、前後の曲への移行）を行う。キーの入力検知は `Window` クラス内の低レベルキーボードフックでグローバルに行われる。
    *   **音量調整連携**: `Window::SetMouseWheelCallback` により、ホバー領域内でのマウスホイール回転（`WM_MOUSEWHEEL`）をフックし、`AudioPlayer` を通じた音量の増減（5%刻み）および設定（`DefaultVolume`）の自動保存を行う。
    *   **プレイリスト連携**: `m_isPlaylistListViewMode` フラグを用いて現在のプレイリストUIがどちらのモード（曲一覧・プレイリスト一覧）であるかの状態管理を一元化している。`Window` からツールバーおよびリスト領域でのホイールスクロールや左クリック・ダブルクリックのコールバックを受け取り、現在のモードに基づいてモード遷移、リスト切替、曲・リストの削除、新規作成、ジャンプ再生などの操作を適切に振り分けるアーキテクチャとなっている。リスト操作において、曲一覧モードでは直感的にサクサク再生できるようシングルクリックで決定（フォーカス更新およびジャンプ再生）とし、プレイリスト一覧モードでは誤操作を防ぐためシングルクリックでフォーカスの設定のみを行い、ダブルクリック時に実際のリスト切り替えを行うハイブリッドな仕様としている。リスト領域のクリック判定（インデックス逆算）時には描画の計算式と同じロジックを用い、ツールバーの高さを加味して開始位置をずらしている。またジャンプ再生直後に、ジャンプ前後のインデックス差分から算出した補正値を手動スクロール量へ加算することで、曲の基準点が切り替わった瞬間にリストが視覚的にジャンプしてしまう現象を完全に相殺し、スクロール位置を維持する極上のUXを実現している。
*   **`void ClearPlaylist()`**
    *   プレイリストのキューを完全に空にし、現在の再生を即座に停止してUIを初期状態（"No Track"）にリセットする。また、クリア後の空の状態をデフォルトプレイリストファイルに上書き保存し、次回起動時も空の状態から始まるように同期する。
*   **`void CreateNewPlaylist()`**
    *   現在日時から自動生成した新しいファイル名（例: `playlist_YYYYMMDD_HHMM.ozl`）でプレイリストの保存パスを切り替える。1分以内に連続作成されて同名ファイルが存在する場合は、シーケンス番号（`_1`, `_2`）を付与して重複を回避する。設定パスを新しいものに更新後、内部で `ClearPlaylist()` を呼び出すことで、メモリ上のキュークリア・UIリセット・空ファイルの生成を一挙に安全に行う。
*   **`void SwitchPlaylist(const std::wstring& filepath)`**
    *   既存のプレイリストファイルへ動的に切り替える。設定ファイル(`OZtone.ini`)のパスを更新した後、現在の再生を停止・キューとUIをリセットし、メモリ上のプレイリストをクリア（`PlaylistManager::Clear`）してから指定されたファイルをロードする。ファイルのロード後は、自動で再生と先読みを開始するとともに、未解析のトラックがある場合はバックグラウンド解析キューへ追加し、専用スレッドを起床させて非同期でのタグ解析を開始する。
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
    *   **WM_MOUSEMOVE等のホバー制御**: `WM_MOUSEMOVE`、`WM_MOUSELEAVE` 等でマウス位置を追跡し、ロゴ領域、ロゴ拡張メニュー領域(`IsInLogoMenuRegion`)、再生コントロール領域へのホバー状態（フェードやスライド展開用）を管理する。また、ロゴ拡張メニュー展開中は `GetLogoMenuButtonAt` を用いて、特定メニューアイコンのホバーインデックス（`m_logoMenuHoveredIndex`）も精密にトラッキングする。
    *   **プレイリストのホバー制御と排他制御**: `WM_MOUSEMOVE` 時に `IsInPlaylistRegion` を用いてプレイリスト領域へのホバーを判定する。描画と入力の分離アーキテクチャを維持するため、プレイリスト上部の固定ツールバーへのホバー・クリック（`m_playlistToolbarHoveredIndex`）と、リスト領域へのホバー・クリックを独立して検知・判定する。この判定ではINI設定(`PlaylistPosition`)に基づき左右どちらからの展開かを動的に判定する。左配置設定時に左上のアプリアイコンおよびロゴ拡張メニュー展開領域と干渉しないよう、該当領域をプレイリスト領域判定から除外する排他制御が組み込まれている。ただしプレイリスト展開中(`m_isPlaylistHovered`)は、ホバー判定幅を `PlaylistHoverWidth` からリスト全体の幅に動的拡張するとともに、ロゴ領域や画面下部UIとの干渉回避用制限をすべて無効化し、リスト全域でホバー状態を維持する。また、プレイリストホバー時は、背後の再生コントロールや音量調整の判定をスキップする排他制御を行う。`WM_MOUSEWHEEL` ではプレイリスト用スクロールコールバックを呼び出し、`WM_LBUTTONDOWN` ではフォーカスの選択（シングルクリック）、`WM_LBUTTONDBLCLK` では再生やリスト切り替え処理（ダブルクリック）をそれぞれ呼び出して処理へ委譲する（この際も背後の他のUI判定より優先して早期リターンし誤動作を防ぐ）。リスト領域のクリック判定（インデックス逆算）はツールバーの高さを加味して開始位置をずらすよう実装されている。
    *   **リサイズモードと最小サイズ制限**: `WM_NCHITTEST` を捕捉し、設定（`EnableResize`）が有効な場合はウィンドウ右下隅でリサイズカーソル（`HTBOTTOMRIGHT`）を返す。また、`WM_SIZE` でリアルタイムなサイズ変更イベントをコールバックで `Application` へ通知し、スワップチェインのリサイズ直後に強制再描画（`ForceRender`）を呼び出すことで、リサイズドラッグ中も描画を追従させる。また、`WM_GETMINMAXINFO` メッセージを捕捉し、現在のDPIに基づいてスケーリングされた論理限界サイズ（Width: 495, Height: 260）を下回らないようにOSレベルで縮小を制限している。
    *   システムトレイのアイコンからのメッセージ (`WM_TRAYICON`) を処理し、右クリック時にコンテキストメニューを表示する。メニューの定義（構築順序）は `Window.cpp` 先頭の `TRAY_MENU_ORDER` 配列で行われており、この配列を編集することで容易に項目の追加・入れ替えが可能となっている。背景の表示モードやZオーダーの切り替え、リサイズモードの有効化、新規プレイリストの作成、プレイリストのクリア等に加え、`ConfigManager::GetAvailablePlaylists()` を用いて動的に利用可能なプレイリストファイルの一覧を取得し、「プレイリスト (Playlists)」サブメニューとして構築する機能を持つ。現在のプレイリストにはチェックマークが付与され、選択することでシームレスにプレイリストが切り替わる。
    *   **Zオーダーとフォーカス制御**: `WM_WINDOWPOSCHANGING` を捕捉し、Zオーダーが「最背面 (Bottom)」に設定されている場合は `HWND_BOTTOM` を強制して手前への移動を防いでいる。同時に `WM_MOUSEACTIVATE` で `MA_NOACTIVATE` を返すことで、クリックによるフォーカス奪取や意図しないアクティブ化を防止している。
    *   **メディアキーの処理 (低レベルキーボードフック)**: `WH_KEYBOARD_LL` を用いた低レベルキーボードフック (`LowLevelKeyboardProc`) により、アプリケーションが非アクティブな状態でもメディアキーの入力をグローバルに捕捉する。捕捉したキーイベントは `WM_APP_MEDIAKEY` カスタムメッセージとして変換・送信され、登録されたコールバック関数へ委譲される。
    *   **多重起動防止とプロセス間通信**: `WM_COPYDATA` を捕捉し、外部プロセス（2つ目以降の起動インスタンス）から送信されたファイルパス文字列を受け取り、登録されたコールバック(`SetCopyDataCallback`)を通じて本体のプレイリストへ追加する。
*   **`DropTarget` クラス (IDropTarget 実装)**
    *   OLE Drag and Drop によるファイルドロップを受け付けるクラス。ドロップ受付範囲はウィンドウのクライアント領域全体となっており、どこにドロップしてもファイルを追加可能である。また `DragEnter` や `DragOver` を処理し、ドラッグ中であってもロゴ領域内にマウスがある場合は `Window` 側のホバー状態を強制的にオンにすることで、ドラッグ中の視覚的フィードバック（ハイライト）を従来通りロゴ領域限定で実現している。
    *   **`bool IsInLogoRegion(int x, int y) const`**, **`bool IsInPlaybackControlRegion(int x, int y) const`**, **`bool IsInVolumeControlRegion(int x, int y) const`**, **`int GetPlaybackButtonAt(int x, int y) const`**
    *   指定された物理座標(x, y)を、DPIスケールを用いて論理座標に変換してから各領域内かどうかを判定する。画面下部のUI領域（再生・音量コントロール等）や右下リサイズグリップの判定では、固定の初期サイズではなく `GetClientRect` で現在の動的なクライアント領域のサイズを取得し、それを基準に相対的な領域計算を行うことで、リサイズ後も正確な座標判定を維持している。
*   **ウィンドウ位置・サイズの保存処理**
    *   `WM_DESTROY` 時に `GetWindowRect` と `GetClientRect` からウィンドウ位置・サイズを取得し、DPIスケールで論理サイズに変換して `ConfigManager` へ保存する。

#### `ConfigManager` クラス (src/ConfigManager.h, cpp)
アプリケーションの設定（`OZtone.ini`）の読み込み、およびデフォルト設定の書き出しを管理する。
*   **`bool Initialize()`**
    *   実行ファイルと同階層の `OZtone.ini` を解決し、存在しなければデフォルト値で作成する。その後、各設定値を読み込む。
*   **`bool CheckForUpdates()`**, **`void LoadSettings()`**
    *   `CheckForUpdates` によりINIファイルの最終更新日時(`std::filesystem::last_write_time`)を監視し、外部エディタ等による上書き保存を検知（ホットリロード）する。変更検知時は `LoadSettings` で最新設定をメモリへ再読み込みする。
*   ウィンドウ表示フラグ (`GetShowTitleBar` 等) やウィンドウの論理位置・サイズ (`GetWindowX` 等)、アプリ全体の視覚効果 (`GetEnableShadow`, `GetBgDarkenOpacity`, `GetBgOpacity` 等)、リサイズモード (`GetEnableResize` 等)、シャッフルモード（`GetShuffleMode` / `SetShuffleMode`）、背景の表示モード（`GetBackgroundArtMode` 等）、ビジュアライザのモード (`GetVisualizerMode` 等)、および各種UIの表示フラグ（`GetShowAppLogo`, `GetShowNowPlaying`, `GetShowNextTrack`, `GetShowSeekBar`, `GetShowPlaybackControls`, `GetShowVolumeControl` 等）、アプリアイコンやアルバムアートのレイアウト情報 (`GetLogoX` 等, `GetBaseX` 等, `GetBaseBottomOffset` 等, `GetFallbackArtOpacity` 等)、ロゴ拡張メニューのアニメーション・レイアウト・フォント設定 (`GetLogoMenuIconSize`, `GetLogoMenuScrollDuration`, `GetLogoMenuIconOffsetX`, `GetLogoMenuTypingLetterSpacing` 等)、再生コントロールのレイアウト設定 (`GetPlaybackBaseBottomOffset`, `ControlHoverHeight` 等)、音量コントロールの設定 (`GetVolumeIconSize`, `TextOffsetX`, `TextLetterSpacing`, 影設定等)、プレイリストUIの各テキスト（タイトル・アーティスト名・再生時間）の独立したフォント・色・オフセット設定およびスライドイン等のレイアウト設定 (`GetPlaylistTimeOffsetX`, `GetPlaylistWidth`, `PlaylistHoverWidth`, `PlaylistArtistColor`, `PlaylistTimeColor`、配置位置 `GetPlaylistPosition`/`SetPlaylistPosition`、グリップの描画設定等)、各テキストのフォントやレイアウト（`GetTitleFontFamily` 等）、シークバーのレイアウトと文字間隔設定（`GetSeekBarMargin`, `GetSeekBarTimeLetterSpacing` 等）、および「次の曲」表示のレイアウト設定・隠し機能フラグ (`GetNextBaseRightOffset`, `GetEnableNextTrack` 等) のゲッターとセッターを提供する。
*   **`int GetZOrder() const`, `void SetZOrder(int zOrder)`**
    *   ウィンドウのZオーダー設定（0: 通常, 1: 最前面, 2: 最背面）の取得と更新を行う。
*   **`bool GetSavePositionOnExit() const`, `void SetSavePositionOnExit(bool save)`**
    *   終了時にウィンドウ位置とサイズを自動保存するかどうかのフラグを取得・更新する。
*   **`void SaveDefaultSettings()`**
    *   初期（デフォルト）の設定値で `OZtone.ini` を上書き保存する。設定の完全初期化などで使用される。
*   **`std::wstring GetDefaultPlaylistPath() const`**, **`void SetDefaultPlaylistPath(const std::wstring& path)`**
    *   起動時に読み込むデフォルトのプレイリストファイルのフルパスを取得・設定する。設定が存在しない場合、初期値（`実行ファイルと同階層/oztone_playlist.lst`）を生成し、直ちにINIファイルへ書き出してユーザーが確認・編集できるようにする。`SetDefaultPlaylistPath` は新規プレイリスト作成時などに呼ばれ、INIファイルのパスを新しい拡張子（`.ozl`）へ永続化更新する。
*   **`std::vector<std::wstring> GetAvailablePlaylists() const`**
    *   デフォルトプレイリストの親ディレクトリ、または実行ファイルの配置ディレクトリを走査し、サポートされるプレイリストファイル（`.ozl` および `.lst`）の一覧をアルファベット順に取得する。システムトレイの動的メニュー構築などに利用される。
*   **`void SaveWindowPosition(int x, int y, int width, int height)`**
    *   ウィンドウの表示位置・サイズを ini ファイルに保存する。

#### `Renderer` クラス (src/Renderer.h, cpp)
Direct3D 11 と Direct2D を用いたハイブリッド描画エンジン。画像デコード用に WIC、テキスト描画用に DirectWrite を内包する。ウィンドウ透過にはDirectCompositionを使用し、アルファブレンドされたスワップチェインを合成する。
また、**内部ロジックを論理ピクセル（96DPI基準）に統一し、描画の根元で `SetTransform` を用いて一括スケールする** というDPIスケーリングアーキテクチャを採用している。
*   **`bool Initialize(HWND hwnd, const ConfigManager& config)`**
    *   D3D11デバイス、`CreateSwapChainForComposition` による透過対応スワップチェイン、DirectCompositionデバイスの作成とウィンドウへのバインド、D2D1ファクトリ等を作成。WICファクトリを初期化し、初期アセット等を読み込む。
    *   ここで各種ブラシ (`ID2D1SolidColorBrush`) や頻繁に描画される固定ジオメトリ (`ID2D1PathGeometry`) のキャッシュ生成（`CreateResources`）も行い、描画ループ内での動的リソース生成を排除している。
    *   ウィンドウのDPIを取得して `m_dpiScale` を算出し保持する。また D2D ターゲットのDPI設定は論理ピクセル基準(96.0f)に固定する。
*   **`bool LoadBitmapResource(const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap)`**
    *   指定されたファイル名がローカルに存在すればWICを用いてファイルからデコードし、無ければメモリリソース(`RCDATA`)からストリームを作成してデコードするフォールバック処理を行う。
*   **`bool LoadBitmapFromMemory(const std::vector<uint8_t>& data, ID2D1Bitmap** ppBitmap)`**
    *   `TagManager` から取得したバイナリデータ等から動的にD2Dビットマップを生成する。
*   **`void SetTrackInfo(const std::wstring& title, const std::wstring& artist)`**, **`void SetAlbumArt(ID2D1Bitmap* bitmap)`**, **`void SetNextTrackInfo(bool isReady, ID2D1Bitmap* art, const std::wstring& title, const std::wstring& artist)`**
    *   外部から曲名・アーティスト名およびアルバムアート画像を設定する。また、先読みされた「次の曲」の情報も設定する。
*   **`void Resize(UINT width, UINT height)`**
    *   ウィンドウのリサイズに伴い、DirectComposition のターゲット指定を解除し、バックバッファを解放した上でスワップチェインの `ResizeBuffers` を実行し、D2Dターゲットを安全に再構築する。
*   **`void ReloadResources()`**
    *   `ConfigManager` のホットリロード検知時に `Application` から呼び出される。フォントやレイアウトの変更を即座に反映させるため、テキストレイアウトのキャッシュクリアフラグを立てると同時に、`m_widgets` に登録された全てのWidgetに対して `ReleaseResources` と `CreateResources` を強制実行し、全Widgetのリソースを再生成させる。
*   **`void UpdateAnimation(float deltaTime, bool isControlHovered, bool isPlaylistHovered, size_t currentTrackIndex, size_t totalTracks)`**
    *   `Renderer::Render` から分離された、UIアニメーション（フェードやスライド等）の状態更新を一元管理するメソッド。
    *   `Render` の直前に呼び出され、再生コントロールのフェード状態 (`m_controlAlpha`) の更新や、各Widget（`IWidget::UpdateAnimation`）への状態更新処理の委譲を行う。描画APIは一切呼び出さない。
*   **`void UpdateTextLayouts(const std::wstring& timeString, float volume, size_t currentTrackIndex, size_t totalTracks)`**
    *   シークバーの再生時間や音量パーセント、プレイリストの曲数表記、および曲情報（現在の曲・次の曲のタイトルとアーティスト名）など、状態によって変動するテキストの `IDWriteTextLayout` を各Widgetでキャッシュ・更新するための起点メソッド。
    *   内部で最新の状態を反映した `WidgetContext` を構築して各 Widget の `UpdateLayout` へ委譲し、値が前回から変化した場合、またはウィンドウリサイズ等で強制更新フラグが立っている場合にのみ `CreateTextLayout` を実行してキャッシュを再構築させ、毎フレームの動的生成コストを削減している。
*   **`void Render(bool isHovered, bool isControlHovered, bool isPlaylistHovered, bool isPlaying, float progress, const std::vector<float>& spectrum, float volume, size_t currentTrackIndex, size_t totalTracks, const std::vector<TrackMetadata>& shuffleMetadataList)`**
    *   毎フレーム呼び出され、描画処理を実行する。内部での巨大化を防ぐため、背景アルバムアート(`DrawBackground`)とビジュアライザ(`DrawVisualizer`)以外の全てのUIコンポーネントは、`IWidget` インターフェースを実装した各Widgetクラス群（`m_widgets`）に完全に委譲されている。**`UpdateAnimation` や `UpdateTextLayouts` によりRendererでの共有状態更新が完了した後、構築した `WidgetContext` を各Widgetへ渡し、`widget->Draw()` を一斉に呼び出す Facade パターンとなっている。** `BeginDraw()` の直後に `SetTransform` によって `m_dpiScale` を適用し、以降の描画はすべて論理ピクセルのまま物理ピクセルへ自動拡大される。
    *   完全透明（リサイズモード有効時は、リアルタイムリサイズ時のコンポジタ処理の破綻を防ぐため不透明の黒）でクリアした後、`BackgroundArtMode` の設定値（0:再生中, 1:非表示, 2:デフォルト固定）に従って背景アルバムアートを描画し、設定に応じてダークオーバーレイ（全画面の黒半透明矩形）を描画して視認性を確保する。
    *   その後、`Visualizer` クラスを用いて、背景アートとUIの間にビジュアライザを描画する（`VisualizerMode` が `0` 以外の場合のみ呼び出し、`0` 時は負荷ゼロでスキップする）。
    *   以降の手前UI（アプリアイコン、曲情報、シークバー、再生・音量コントロール、プレイリスト、リサイズグリップ等）は全て `m_widgets` に登録された各Widgetによって描画される。各UI要素は自身内で `ConfigManager` から `Visibility` フラグ等を取得し、無効時は処理をスキップする。
    *   時間などの各種文字列の描画にはキャッシュされた `IDWriteTextLayout` を使用し、ConfigManager から取得した文字間隔を `SetCharacterSpacing` で生成時に適用しておくことで描画負荷を最小化するアプローチは、各Widgetの内部へそのまま引き継がれている。

#### `Widget` コンポーネント (src/Widget.h, Widgets.h, cpp)
UI要素ごとの独立した描画・状態管理を担うコンポーネント群。`Renderer` の巨大化を防ぐため、UI要素単位でクラス化されている。
*   **`IWidget` インターフェース**
    *   全Widgetが実装する共通インターフェース。`CreateResources` (リソース生成), `UpdateAnimation` (アニメーション更新), `UpdateLayout` (テキストやレイアウトの更新), `Draw` (描画) のライフサイクルメソッドを持つ。
*   **`WidgetContext`**
    *   各フレームで全Widgetに共通して必要な状態（`deltaTime`, `dpiScale`, `controlAlpha`, 現在のアルバムアート、スペクトルデータ、ConfigManagerへのポインタ等）を格納し、`Renderer` から各Widgetへ受け渡される構造体。
    *   **ルール**: `WidgetContext` は共有データのみを保持し、特定のWidget固有の状態（例：特定のテキストレイアウトキャッシュ）は持たない。
*   **各具象 Widget 実装 (`Widgets.h/cpp`)**
    *   `AppLogoWidget`: アプリアイコンの描画。メニュー展開中（`isLogoMenuHovered`がtrueの時）は、大元のアプリアイコンもホバー状態画像（`app_logo_hover.png`）を維持する。
    *   `LogoMenuWidget`: アプリアイコンホバー時に右側へアニメーション進行度に応じたイージングでスライド展開する「ロゴ拡張メニュー」の描画。配列ベースで管理されたメニューアイコンの展開、ホバー時における背面グロー画像（`app_logo_back.png`）の合成、トグルOFF時（またはビジュアライザ無効時）の視覚フィードバック（斜線描画）、およびビジュアライザアイコン右下の数字縁取り描画を担当する。また、状態（プレイリスト左右配置等）やホバーアイコンに連動した動的なテキスト説明表示（シャドウ付き）を行う。
    *   `TrackInfoWidget`: 左下のアルバムアート、曲名・アーティスト名、およびトラック番号（「XXX/XXX」表記、アライメント変更対応）の描画。正規のアルバムアートがない場合はプレースホルダー領域として黒い板（`FallbackArtOpacity`）を描画するフォールバック処理を行う。テキストトリミング機能を含む。また、IDWriteTextLayoutによるテキストのキャッシュ化を行い、描画ループ内での動的リソース生成を排除している。
    *   `NextTrackWidget`: 右下の「次の曲」の極小アルバムアートと曲情報描画。設定により非表示時はスキップ。
    *   `SeekBarWidget`: 進行度に応じたシークバーと現在時間の描画。ホバー領域に入ると輝度が最大50%減衰する機能を持つ。
    *   `PlaybackControlsWidget`, `VolumeControlWidget`: 画面下部ホバー時に白ベクターアイコンでフェードインする再生・音量コントロールの描画。
    *   `PlaylistWidget`: INI設定(`PlaylistPosition`)に応じ、画面左端または右端のホバー時にスライドインするシャッフル再生リストと「引き出しグリップ」の描画。グリップの矢印の向きも左右配置に応じて動的に反転する。`Application` から渡されたモード情報（`isPlaylistListViewMode`）に基づき、曲一覧またはプレイリスト一覧を切り替えて描画し、上部に固定ツールバー（上段にアイコン、下段に日本語説明テキスト）を描画する純粋な描画責務を持つ。自身でスライドインアニメーションやスクロール制御、限界値クランプ、各テキストの描画を担当し、対象曲のメタデータが解析済み（`isLoaded == true`）の場合は取得した曲名・アーティスト・再生時間を使用してリッチに描画、未解析の場合はファイル名から推測したタイトルでのフォールバック描画を行う。
    *   `ResizeGripWidget`: リサイズモード有効時に右下隅に半透明のグリップを描画。
    *   各Widgetは、自身に必要なフォントフォーマット(`IDWriteTextFormat`)やブラシ(`ID2D1SolidColorBrush`)、テキストレイアウト(`IDWriteTextLayout`)の生成とキャッシュ化を自己責任で管理し、外部（Renderer）に依存しない完全なカプセル化を実現している。

#### `LayoutCalculator` クラス (src/LayoutCalculator.h, cpp)
各UI要素の描画領域（矩形や座標）を計算するための純粋な計算クラス。
*   **状態を持たない設計**: ウィンドウの論理サイズ、ホバーやアニメーション状態、INI設定値（`ConfigManager`へのポインタ）などの入力値を受け取り、描画に必要なUI要素ごとの座標や矩形（`D2D1_RECT_F` や `D2D1_POINT_2F` など）を `TrackInfoLayout` や `PlaylistLayout`、`LogoMenuLayout` のような専用構造体として算出・返却するだけの純粋な静的メソッド群。プレイリストのスライド座標やクリップ矩形、ロゴ拡張メニュー展開時の各アイコンのイージング座標等もここで計算される。
*   **描画と計算の分離**: `Renderer` が担当していた複雑な座標計算やテキスト領域の制約算出ロジックをこのクラスへ外部化したことで、`Renderer` の各メソッドは「渡されたレイアウト構造体に従ってピクセルを打つだけ」のクリーンな処理に保たれている。DPIスケーリング前の論理ピクセルでの計算を維持している。

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
内部のリストデータは `TrackMetadata` 構造体（ファイルパス、曲名、アーティスト名、再生時間、解析済みフラグを保持）で管理され、`std::mutex` によりマルチスレッドからの安全な排他制御アクセスが保証されている。
*   **`size_t GetCurrentIndex() const`, `std::vector<TrackMetadata> GetShuffleMetadataList() const`**
    *   現在のシャッフルインデックスと、次に再生される順番のプレイリスト全体のメタデータ一覧を取得する。後方互換用としてファイルパスのみを返す `GetShuffleList()` も存在する。
*   **`void JumpToIndex(size_t index)`**
    *   指定したシャッフルインデックスの曲へ直接ジャンプし、インデックスを更新する。
*   **`bool Add(const std::wstring& filepath)`**
    *   指定されたファイルパス（絶対パス）をプレイリストに追加する。すでに存在する場合はスキップする。
    *   追加時、未解析状態(`isLoaded = false`)の `TrackMetadata` としてリストへ登録される。現在のシャッフルループおよび次回のシャッフルループの末尾に、シャッフルせずにそのまま追加する。これにより、再生中のキューにドロップした曲が即座に末尾に積まれる。
*   **`std::wstring GetCurrentTrack() const` / `std::wstring GetNextTrack() const`**
    *   現在または次の曲のファイルパスを返す。次の曲がループ末尾にある場合は次周リストの先頭曲を返し、先読み処理と整合性を保つ。
*   **`bool GetTrackMetadata(const std::wstring& filepath, TrackMetadata& outMeta) const`**
    *   指定したファイルパスのメタデータ情報を取得する。
*   **`void UpdateMetadata(...)` / `std::vector<std::wstring> GetUnparsedTracks() const`**
    *   バックグラウンド解析・自己修復のための更新用APIおよび、未解析の曲一覧を取得するAPI。
*   **`void SaveToFile(const std::wstring& outPath) const` / `void LoadFromFile(const std::wstring& inPath)`**
    *   プレイリストの実体を TSV（タブ区切り）形式のテキストファイルに入出力する。解析済みの曲は全メタデータ（`filepath \t title \t artist \t timeString`）が保存され、次回起動時にキャッシュとして即座に復元される。要素が足りない古いフォーマットからのフォールバック機能も持つ。
*   **`void RebuildQueue(bool isShuffle)`**
    *   シャッフルON/OFFに応じてインデックス配列（`m_shuffleIndices`）をランダムまたは連番で完全に再構築するロジック。設定変更時や即時再生モード時に呼び出される。
*   **`void WarpToTrack(const std::wstring& filepath)`**
    *   指定されたファイルパスの曲へインデックスを強制移動（ワープ）させる処理。キュー再構築後に現在の再生曲を維持したり、追加曲から即時再生を開始させたりする用途で活用される。
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

*   **`.mp4` / `.m4a` フォーマットの実験的対応**:
    `Application::OnFilesDropped` において、これらの動画コンテナフォーマットに対しては複雑なマジックナンバー検証をスキップし、無条件でプレイリストへ通過させる暫定処理を組み込んでいます。将来的には専用の検証ロジックを実装するか、より堅牢なメディア解析処理へ置き換えられる想定です。(`// [EXPERIMENTAL] MP4/M4A Support` コメントにて隔離済み)

*   **TODO (将来): `LayoutCalculator` の `ConfigManager` 依存の段階的除去**:
    現在、`LayoutCalculator` の各計算メソッドは引数で `const ConfigManager* config` を受け取っている。将来的にこの依存関係を除去し、`Renderer` 側で必要な設定値のみを `LayoutInput` 等の専用構造体へ詰め替え、`LayoutCalculator` が `ConfigManager` を一切知らない完全に独立した形へと移行する。

*    **TODO (将来): Renderer更新メソッドの統合 (Renderer::Update の導入)**:
    現在 Application から個別に呼び出している UpdateAnimation や UpdateTextLayouts などの更新処理を、単一の Renderer::Update(float deltaTime, ...) メソッドへ統合・隠蔽する。これにより Application は Renderer 内部のリソース更新手順を知る必要がなくなり、依存関係をさらにクリーンにする。
