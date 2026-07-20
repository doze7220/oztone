# PROJECT_ARCHITECTURE.md
## OZtone 全体仕様および実装定義書

**【特記事項】現在はPhase 23にて FileManager リファクタリングが完了し、FileManager が TagLib の機能を完全に吸収してファイルI/Oおよびメタデータ抽出の単一の門番として稼働を開始した。旧来の TagManager は完全にパージされた。**

本資料は、AIがタスクを実行する際の「全体マップ（見取り図）」および「インデックス（索引）」として機能するものである。AIがどの責務を修正する際にどのファイル（個室）を開くべきかを示す道標に特化させる。
したがって、直接コードを読めば瞬時に理解できる詳細なメソッドの内部ロジック、関数の引数、設定パラメータの全列挙などはAIのコンテキストを圧迫する「ノイズ」となるため、本資料には記載しないこと。フェーズが進むごとに、実装された項目をここへ随時追記・更新していくこと。

---

### 1. プロジェクト概要と基本方針
*   **アプリ名**: OZtone (デスクトップアクセサリ風MP3プレイヤー)
*   **言語・環境**: C++20, Win32 API, Direct3D 11, Direct2D, DirectComposition
*   **ビルド**: CMake (Visual Studio Developer Command Prompt を用いた MSBuild や Ninja の利用を想定)
*   **外部ライブラリ**: miniaudio (音声処理), TagLib (MP3タグ処理)
*   **設定ファイル**: `OZtone.ini` に保存 (`ConfigManager` により管理)
    *   設定値は`DEFAULT_INI_CONTENT`(`ConfigManager_DefaultIni.h`)を**Single Source of Truth**とする。
*   **UI/ビジュアル仕様方針**:
    1. **ストイックなステルスUI**: 普段はアルバムアートとビジュアライザとシークバーのみ。操作に必要なUIはマウスホバー時のみ現れ、操作後にディレイを伴ってフワッと消えること。
    2. **コンフィグウィンドウの禁止**: 専用の設定画面UIは絶対に作成しない。設定はすべて OZtone.ini の直接編集とシステムトレイの右クリックメニューのみで完結させること。
    3. **High DPI (Per-Monitor V2) と座標系の統一**: OSからの入力（マウス座標やウィンドウサイズ）は物理ピクセルで受け取るが、LayoutCalculator や描画ロジックはすべて**「96DPI基準の論理ピクセル」**で統一して計算すること。スケーリングは描画の根元（SetTransform）で一括適用される。
    4. **トラックドラム（プレイドラム）とトランジション**: 曲切り替え時は、質量と慣性を持ったドラムが回転するトランジション演出を行う。非同期画像ロード中は半透明ガラス板（フォールバック）を描画し、ロード完了・着地時にクロスフェードで出現させる。

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
 ├── assets/                 # ボタン類などのリソース、テスト用音声ファイル
 ├── _docs/                  # 開発関連ドキュメント
 │   └── logs/               # AIの実装レポート出力先 (YYYYMMDD_HHMM_タスク名.md)
 ├── PROJECT_ARCHITECTURE.md # 本ファイル（全体仕様・設計・アーキテクチャ）
 └── PROJECT_CONSTITUTION.md # プロジェクト憲法（AI向け絶対ルール）
```

---

### 4. 全体のルーチン（アーキテクチャと実行フロー）
1.  **エントリポイント (`wWinMain`)**:
    *   `CreateMutexW` を用いて多重起動を防止。すでにプロセスが存在する場合は、コマンドライン引数のパスを `WM_COPYDATA` で既存プロセスへ送信して自身は終了する。
    *   `OleInitialize` にて COM を初期化後、`Application` インスタンスを生成。初回起動で引数がある場合は `ProcessCommandLineArgs` にて読み込ませる。
2.  **初期化 (`Application::Initialize`)**: 内部で `Window::Initialize`、`Renderer::Initialize`、`AudioPlayer::Initialize` を順次呼び出しシステムの初期化を行う。
3.  **メインループ (`Application::Run`)**: `Window::ProcessMessages` を毎フレーム呼び出し、OSのメッセージを処理。
4.  **メッセージ処理 (`Window::WindowProc` / `DropTarget`)**: OSからのイベントを処理。`WM_DESTROY` の他、アプリアイコン上での `WM_LBUTTONDOWN`（ドラッグ移動）を処理。ファイルのドラッグ＆ドロップは `IDropTarget` を通じて処理される。
5.  **状態管理と画像ロード（3スロット式リングバッファ）の究極純化**:
    *   旧来の2スロットを用いた0.5でのフリップという手品的な手法は完全に廃止され、3つの `DrumSlot` 構造体を循環させる「3スロットのリングバッファ」による状態管理アーキテクチャへと進化した。
    *   画面外（完全に見えない位置）に押し出されたスロットを利用し、「0ぴったり（整数位置到達時）」のタイミングで次に表示される曲のデータを先行注入（Pre-fill）する、物理法則に完全に準拠したスクロール機構が構築されている。
    *   外部（ApplicationやWidget）から `DrumSlot` への直接操作は厳禁であり、データの上書きは `Drum` エンジン内部の先行注入時にのみ行われる「究極のカプセル化」が維持されている。
    *   **動的間引きスクロール（ダイナミックスケール）の仕様**:
        *   遠距離ジャンプ時、TrackDrumエンジンはINI設定の物理パラメータ（MaxDuration, MaxSpeed）を用いて、「時間 × 速度」によるリニアな計算からアニメーションの「限界フリップ数」を動的に算出する。
        *   目標までの論理的なインデックス距離がこの限界を超える場合、物理的な移動距離をクランプし、「1フリップあたりに進むインデックスのステップ幅」をスケールさせる。
        *   スクロール中の画面外Pre-fill時に、このステップ幅を乗算して「中間に位置する間引かれた曲」を動的に算出し、パラパラ漫画のように差し込みながら目標曲への完全な同期着地を実現している。
6.  **コールバック駆動とイベント駆動のステートマシン**:
    *   アニメーションは外部から「相対距離」と「データ取得用コールバック（パイプ）」、「完了時コールバック」を渡すだけの **コールバック駆動** となった。
    *   アニメーションの物理演算層（`UpdateAnimation`）は距離を計算するだけであり、1スロット分進んだという「事実」のみをエンジン本体へ通知する **イベント駆動** となった。
    *   通知を受けたエンジン本体が、目標到達前ならコールバック経由で必要なメタデータのみをオンデマンドで取得してフリップし、到達していれば完了通知を出す自律的な意思決定を行う。
7.  **描画更新層（UpdateTextLayouts）の純化**:
    *   テキストレイアウトの更新処理は、既に `DrumSlot` 内に保護されているデータを用いてキャッシュを再構築するのみであり、外部データからの毎フレーム上書き処理は完全にパージされた。

---

### 5. 実装済みクラス・関数リファレンス

#### `Application` クラス (src/Application.h)
アプリケーション全体のライフサイクル、メインループ、再生制御、プレイリスト管理、ファイル入力処理を統括するマネージャ。スリープ時のオーディオエンジンのサスペンド・レジューム管理や、オーディオエンジンの異常停止を検知し自己修復するウォッチドッグ機構を内包。メインループ内ではプレイリストファイル群のスナップショット監視（毎秒ポーリング）を行い、アクティブファイルの消失時には安全な空状態へ移行するフェイルセーフ機構を持つ。さらに、ThumbnailDatabaseとThumbCacherを仲介する司令塔としての責務を持ち、単方向依存でサムネイル生成の発注管理を行う。
**物理分割ファイル:**
*   `Application.cpp`: コンストラクタ、デストラクタ等のエントリポイント
*   `Application_Initialize.cpp`: Initialize, SetupCallbacks 等の初期化処理
*   `Application_Playback.cpp`: PlayCurrentTrack, HandleMediaCommand 等の再生制御
*   `Application_Playlist.cpp`: SwitchPlaylist, ClearPlaylist, プレイリストUIのクリックコールバック等
*   `Application_FileDrop.cpp`: OnFilesDropped 等のファイル入力処理
*   `Application_Render.cpp`: Run, ForceRender 等のメインループおよび描画連携処理

#### `Window` クラス (src/Window.h)
Win32 APIのウィンドウ生成・破棄、メッセージディスパッチ、マウス入力・ホバー判定、トレイアイコン、ドラッグ＆ドロップ、グローバルホットキー等のOS連携処理を隠蔽・カプセル化するクラス。OSの電源イベント（スリープ移行・復帰）のフック。
**物理分割ファイル:**
*   `Window_Proc.cpp`: メッセージディスパッチの基盤（薄い司令塔）
*   `Window_Mouse.cpp`: 座標判定（ヒットテスト）とマウス入力イベント処理
*   `Window_TrayMenu.cpp`: トレイアイコンおよびコンテキストメニューの生成・コマンド処理
*   `Window_DropTarget.cpp`: OLE Drag and Drop によるファイルドロップ処理
*   `Window_System.cpp`: 低レベルフック、グローバルホットキー、多重起動防止などのシステム連携処理
*   `Window_Initialize.cpp`: ウィンドウの登録・生成・初期化および破棄処理

#### `ConfigManager` クラス (src/ConfigManager.h)
アプリケーションの設定（`OZtone.ini`）の読み込み・書き出し・ホットリロードを管理する。全設定の初期値は `ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` にSSOT化されている。サムネイルのJPEG品質を制御する `ThumbnailJpegQuality` や、プレイリスト上のサムネイルレイアウトを制御する `PlaylistThumbSize`, `PlaylistThumbOffsetX`, `PlaylistThumbOffsetY` などの設定パラメータが追加された。また、プレイリストディレクトリのスナップショット監視によるホットリロード機能を備え、外部からの `.ozl` ファイル（旧 `.lst` フォーマットは完全廃止）の増減や変更を検知して自動的に反映させる。
**物理分割ファイル:**
1. `ConfigManager.cpp`: デフォルト設定文字列とファイルI/Oコア処理
2. `ConfigManager_Window.cpp`: ウィンドウ設定、Z-Order、可視性、背景アート
3. `ConfigManager_Playlist.cpp`: プレイリストUI全般
4. `ConfigManager_Playback.cpp`: シークバー、再生・音量コントロール
5. `ConfigManager_LogoMenu.cpp`: アプリアイコン、ロゴ拡張メニュー
6. `ConfigManager_Visualizer.cpp`: ビジュアライザ全般
7. `ConfigManager_System.cpp`: グローバルホットキー、OSDなど

#### `Renderer` クラス (src/Renderer.h)
Direct3D 11 / Direct2D / DirectComposition を用いたハイブリッド描画エンジン。内部ロジックを論理ピクセル（96DPI基準）に統一し、描画の根元で `SetTransform` を用いて一括DPIスケールするアーキテクチャ。ビジュアライザを自前で描画し、それ以外のUI要素は全て `IWidget` 実装クラス群に委譲する。ドラムアニメーション（リングバッファ）のステートマシンは独立した部品として `TrackDrum` クラスにカプセル化され、機能が `Renderer` から切り離されている。これにより、コールバック・イベント駆動によるデータ層とのデカップリングおよび「究極のカプセル化」を実現している。旧来の背景アート描画・保持機構は完全にパージされており、背景はダークオーバーレイのみを描画している（将来の BackgroundManager 結線に備えたマーカーを残している）。
**物理分割ファイル:**
*   `Renderer_Initialize.cpp`: 初期化、リソース生成、リサイズ処理
*   `Renderer_Image.cpp`: WICを用いた画像デコード処理
*   `Renderer_Update.cpp`: アニメーションおよびテキストレイアウトの状態更新処理
*   `Renderer_Draw.cpp`: 純粋な描画ループ処理本体
*   `Renderer_Context.cpp`: WidgetContextの構築処理
*   `Renderer_TrackDrum.cpp`: ドラムアニメーションの物理演算と状態管理（TrackDrumクラス）。INI設定（MaxDuration, MaxSpeed）に基づく動的間引きスクロール（ダイナミックスケール）の計算と、目標インデックスへの同期着地を担う。
*   `Renderer.cpp`: コンストラクタ・デストラクタやセッター等の最小限の窓口機能

#### `Widget` コンポーネント (src/Widget.h, src/WidgetContext.h, src/WidgetCommon.h/cpp)
UI要素ごとの独立した描画・状態管理を担うコンポーネント群。`IWidget` インターフェースが `CreateResources`, `UpdateAnimation`, `UpdateLayout`, `Draw` のライフサイクルメソッドを定義する。`WidgetContext` は各フレームの共有状態を全Widgetに受け渡す構造体。`WidgetCommon` は共通ユーティリティ（影付きテキスト描画、Hex色変換等）を集約する名前空間。
**各具象Widget:**
*   `AppLogoWidget` (src/Widget_AppLogo.h/cpp): アプリアイコンの描画
*   `LogoMenuWidget` (src/Widget_LogoMenu.h/cpp): アプリアイコンホバー時にスライド展開するロゴ拡張メニューの描画
*   `TrackInfoWidget` (src/Widget_TrackInfo.h/cpp): 左下のアルバムアート・曲名・アーティスト名の描画。データ層への直接アクセス（オンデマンド取得）の責務を剥奪され、コンテキストから渡される独立した3つのスロット (`drumSlots`) の情報を無条件で描画するだけの受動態へ純化された。中間アニメーション時はガラス板のみのフォールバックを描画する。また、テキストレイアウト更新処理 (`UpdateTextLayouts`) についても、既に保護されているスロット内データからのキャッシュ再構築に限定され、毎フレームの外部データ上書きは完全にパージされている。
*   `SeekBarWidget` (src/Widget_SeekBar.h/cpp): シークバーと再生時間の描画
*   `PlaybackControlsWidget` (src/Widget_PlaybackControls.h/cpp): 画面下部の再生コントロール（5ボタン）の描画
*   `VolumeControlWidget` (src/Widget_VolumeControl.h/cpp): 音量コントロールの描画（ツールチップ含む）
*   `GlobalHotkeysWidget` (src/Widget_GlobalHotkeys.h/cpp): グローバルホットキーのチートシート表示
*   `PlaylistWidget` (src/Widget_Playlist.h/cpp): プレイリスト一覧・曲一覧のスライドイン描画およびツールバー。以下に物理分割：
    *   `Widget_Playlist_Resources.cpp`: リソースの生成と解放
    *   `Widget_Playlist_Update.cpp`: アニメーションやレイアウトの状態更新
    *   `Widget_Playlist_Toolbar.cpp`: ツールバーやピン留めボタンの描画
    *   `Widget_Playlist_DrawItems.cpp`: プレイリストの曲一覧およびリスト一覧のアイテム描画（アスペクト比を維持したサムネイルのフィット描画を含む。プレイリスト一覧モードでは1曲目のサムネイルがアルバムジャケットとして表示され、デザインの統一感を出すための数字なしCD帯が追加された）
    *   `Widget_Playlist_Draw.cpp`: 大元の描画ループと背景などのベース描画
    *   `Widget_Playlist.cpp` (本体): コンストラクタや最小限の窓口機能
*   `ResizeGripWidget` (src/Widget_ResizeGrip.h/cpp): リサイズグリップの描画
*   `OsdWidget` (src/Widget_Osd.h/cpp): OSD（On-Screen Display）テキストの描画

#### `LayoutCalculator` クラス (src/LayoutCalculator.h, cpp)
各UI要素の描画領域（矩形や座標）を算出する純粋な計算クラス。状態を持たず、入力値から `TrackInfoLayout` や `PlaylistLayout` 等の構造体を返す静的メソッド群で構成される。

#### `Visualizer` クラス (src/Visualizer.h, cpp) / `IVisualizerStyle` (src/IVisualizerStyle.h)
ビジュアライザを統括するファサードクラス。生FFTスペクトルの前処理（ノーマライズ、対数サンプリング、EQ適用）を一元的に行い、完成済みデータをプラグイン化された各描画スタイルへディスパッチする。
**スタイル一覧:**
*   `VisualizerPrismBeat` (src/Visualizer_PrismBeat.cpp): 直線型の心電図表現（ネオングロー3パス描画）
*   `VisualizerHaloDust` (src/Visualizer_HaloDust.cpp): 円形パーティクル表現（レーザー・破片パーティクル物理演算）

#### `AudioPlayer` クラス (src/AudioPlayer.h, cpp)
音声処理ライブラリ `miniaudio` をラップし、MP3/FLAC/WAV/OGG の再生・停止・シーク・音量制御を管理するクラス。リアルタイムFFTスペクトル解析および楽曲の事前スキャン機能を内包する。

#### `FileManager` クラス (src/FileManager.h, cpp)
ファイルI/Oおよびメタデータ抽出を単一の窓口として引き受ける門番クラス。外部に対して TagLib などの存在を完全に隠蔽する Adapter パターンとして機能する。大量の楽曲ロード時のメモリ圧迫を防ぐため、軽量なテキストメタデータの抽出と、重い画像バイナリのオンデマンド抽出に責務を分割して定義されている。

#### `PlaylistManager` クラス (src/PlaylistManager.h, cpp)
再生待ちキュー（プレイリスト）を管理するクラス。常時シャッフル再生およびダブルバッファリングによるシームレスなループをサポートし、純粋に「ファイルパス（曲順）」のみを管理する軽量設計。

#### `TrackDatabase` クラス (src/TrackDatabase.h, cpp)
楽曲固有の解析データ（曲名、アーティスト名、再生時間、peakAmplitude、maxFrequency等）を集中管理するデータベース。ファイルパスをキーとする `std::unordered_map` でO(1)アクセスを提供し、`oztone_track.odb` に永続化する。

#### `ArtFramingDatabase` クラス (src/ArtFramingDatabase.h, cpp)
背景アートのフレーミング設定（X, Y, Scale）を曲ごとに独立管理するデータベース。`oztone_framing.odb` (TSV形式) に永続化する。

#### `TrackAnalyzer` クラス (src/TrackAnalyzer.h, cpp)
タグ解析およびFFT波形事前スキャンをバックグラウンドスレッドで非同期実行し、完了データを `TrackDatabase` へ自律的に書き込むクラス。

#### `ThumbnailDatabase` クラス (src/ThumbnailDatabase.h, cpp)
サムネイル画像のバイナリデータ（パックファイル）の読み書きと、LRUアルゴリズムに基づくオンメモリキャッシュ管理を行うデータベースクラス。IDとファイルパスの紐付け管理を行う「純粋な辞書管理・窓口」として機能する。スレッドセーフなバイナリ追記機能とセクタ情報のメモリ同期機能に加え、必要な時にのみパックファイルからJPEGバイナリを吸い出してWICデコードする「オンデマンド読み出し機能」と、VRAM使用量を管理して古い画像を破棄する「LRUキャッシュ機構」を備える。
さらに、インデックス(`.idx`)保存時のUTF-8 ⇔ UTF-16変換によるUnicodeファイルパスへの完全対応や、デストラクタにおけるタスクカウントを用いたスレッドの安全終了（待機）機構を備えており、文字化けやアプリ終了時のクラッシュを防止する堅牢なライフサイクル管理を実現している。

#### `ThumbCacher` クラス (src/ThumbCacher.h, cpp)
バックグラウンドスレッドで非同期にサムネイル画像を生成・リサイズし、キューイングされたタスクを順次処理する専用のエンジンクラス。外部からの発注（IDとパスのペア）に従って黙々とサムネイルを生成する「純粋な工場（ワーカー）」として機能する。TagManagerの純化に伴い、現在は一時的に画像抽出呼び出しがパージされており、次世代パイプライン（FileManager連携）の構築待ち状態である。

