# RES:実装計画・作業レポート Phase 20-12: Rendererの完全解体（ファイル分割）

## 1. 実装目的
現在肥大化している `src/Renderer.cpp` について、クラスの定義（`Renderer.h`）や外部からの依存関係は一切変更せず、実装内容のみを各責務に応じた `.cpp` ファイルに物理分割する。これにより、AI-IDEのコンテキストを節約し、今後の開発における可読性とメンテナンス性を向上させる。

## 2. アーキテクチャ設計
### 要件1: 初期化とリソース管理の分離 (`Renderer_Initialize.cpp`)
    - `Initialize`、`Resize`、`ReloadResources` など、初期化やリソースの構築・再構築に関する実装を分離する。
### 要件2: 画像デコード処理の分離 (`Renderer_Image.cpp`)
    - `LoadBitmapResource`、`LoadBitmapFromMemory` などの画像読み込みやデコード処理を分離する。
### 要件3: 状態更新処理の分離 (`Renderer_Update.cpp`)
    - `UpdateAnimation`、`UpdateTextLayouts` などの、毎フレームの描画前状態更新に関わる処理群を分離する。
### 要件4: 描画ループ処理の分離 (`Renderer_Draw.cpp`)
    - `Render`、`DrawBackground`、`DrawVisualizer` などの純粋な描画処理群を分離する。
### 要件5: コンテキスト構築の分離 (`Renderer_Context.cpp`)
    - `WidgetContext` の構築を行う処理（`BuildAnimationContext` や `BuildLayoutContext` など、関数化や切り出しが可能な部分）を専用ファイルとして分離する。
### 要件6: 大元ファイルの整理と CMakeLists の更新
    - 巨大な責務を分離した後の `src/Renderer.cpp` には、コンストラクタ・デストラクタや各種セッターなどのみ残す。
    - `CMakeLists.txt` に新設した `.cpp` ファイル群を登録し、ビルドパスを確保する。
### 要件7: #include の最適化と依存関係の維持
    - 分割後の各 `.cpp` ファイルにおいて、必要なヘッダのみをインクルードし、不要な依存関係を持ち込まないように整理する。

## 3. 実装タスクリスト
- [x] タスク1: `src/Renderer_Initialize.cpp` の作成と実装移行
    - `Renderer::Initialize`, `Renderer::Resize`, `Renderer::ReloadResources` などを移行。
- [x] タスク2: `src/Renderer_Image.cpp` の作成と実装移行
    - `Renderer::LoadBitmapResource`, `Renderer::LoadBitmapFromMemory` を移行。
- [x] タスク3: `src/Renderer_Update.cpp` の作成と実装移行
    - `Renderer::UpdateAnimation`, `Renderer::UpdateTextLayouts` などの状態更新メソッドを移行。
- [x] タスク4: `src/Renderer_Draw.cpp` の作成と実装移行
    - `Renderer::Render`, `Renderer::DrawBackground`, `Renderer::DrawVisualizer` などの描画メソッドを移行。
- [x] タスク5: `src/Renderer_Context.cpp` の作成と実装移行
    - `WidgetContext` を構築するための処理を抽出し、移行。
- [x] タスク6: `src/Renderer.cpp` の整理
    - コンストラクタ、デストラクタ、セッター等の小規模メソッドのみを残し、ファイルサイズを削減。
- [x] タスク7: `CMakeLists.txt` の更新
    - `src/Renderer_Initialize.cpp` 等の新規ファイル5つをソース一覧に追加。
- [x] タスク8: `PROJECT_ARCHITECTURE.md` の更新
    - `Renderer` クラスのアーキテクチャ説明に、実装が物理分割された旨を追記する。

## 4. 詳細作業内容
### タスク1: src/Renderer_Initialize.cpp の作成と実装移行
    - 新規ファイルを作成し、以下の初期化・リソース管理系メソッドを移行した。
      - `Renderer::Initialize`: Direct3D, DXGI, DirectComposition, Direct2D, WIC, DirectWriteの各ファクトリ・デバイス・スワップチェインの構築、および `AppLogoWidget` から `OsdWidget` に至る全Widgetインスタンスの生成と初期化処理。
      - `Renderer::Resize`: スワップチェインとD2Dターゲットビットマップの再構築処理。
      - `Renderer::ReloadResources`: 設定変更時などに呼ばれる、ブラシの再生成および各Widgetの `CreateResources` の呼び出し処理。
      - 上記の実装に必要なすべてのWidgetヘッダ（`Widget_AppLogo.h` など）の `#include` を本ファイルに集約した。

### タスク2: src/Renderer_Image.cpp の作成と実装移行
    - 新規ファイルを作成し、WICを用いた画像デコード処理を以下の通り移行した。
      - `Renderer::LoadBitmapResource`: exe同階層のファイルからの読み込み、および失敗時の埋め込みリソース(`RCDATA`)からのフォールバック読み込み・デコード処理。
      - `Renderer::LoadBitmapFromMemory`: `TagManager` 等から渡されるバイナリデータストリームからの動的デコード処理。

### タスク3: src/Renderer_Update.cpp の作成と実装移行
    - 新規ファイルを作成し、毎フレームの状態更新メソッドを以下の通り移行した。
      - `Renderer::UpdateAnimation`: コントロールのホバー離脱ディレイ処理、OSD・フライテキストのフェードアニメーション計算、および全Widgetに対する `UpdateAnimation` 呼び出し処理。
      - `Renderer::UpdateTextLayouts`: 前フレームからの状態キャッシュ比較による更新要否判定と、全Widgetに対する `UpdateLayout` 呼び出し処理。

### タスク4: src/Renderer_Draw.cpp の作成と実装移行
    - 新規ファイルを作成し、描画パイプラインのメソッドを以下の通り移行した。
      - `Renderer::Render`: `BeginDraw` から始まり、背景描画、ビジュアライザ描画、全Widgetの描画（`Draw`）、そして `EndDraw` と `Present` を行うメインの描画ループ。
      - `Renderer::DrawBackground`: アルバムアートのフレーミングレイアウト計算およびD2Dによる描画と、ダークオーバーレイの半透明矩形描画処理。
      - `Renderer::DrawVisualizer`: 楽曲のスペクトルデータに基づくビジュアライザ（PrismBeat, Halo Dust等）の描画処理。

### タスク5: src/Renderer_Context.cpp の作成と実装移行
    - 各種Update/Renderメソッド内で巨大化していた `WidgetContext` 構造体の構築処理を関数化するため、`Renderer.h` の private メソッドとして以下を新規定義し、実装を本ファイルへ分離した。
      - `BuildAnimationContext`
      - `BuildLayoutContext`
      - `BuildRenderContext`
    - これにより、`Renderer_Update.cpp` および `Renderer_Draw.cpp` のループ内処理の見通しが大幅に向上した。

### タスク6: src/Renderer.cpp の整理
    - 上記タスクにより巨大な描画・更新アルゴリズムがパージされ、本ファイルには以下のような軽量なメソッドのみが残された。
      - コンストラクタ (`Renderer::Renderer`)、デストラクタ (`Renderer::~Renderer`)
      - 状態セット系 (`SetTrackInfo`, `SetAlbumArt`, `SetNextTrackInfo`, `SetBackgroundFraming`, `SetFocusedPlaylistIndex` など)
      - 小規模ユーティリティ (`ClampArtFraming`, `AddPlaylistScroll`, `GetPlaylistManualScrollY`, `TriggerVolumeOsd`, `ForceClearHoverDelays` など)
    - 物理的なファイルサイズが劇的に縮小され、今後のコード把握が容易になった。

### タスク7: CMakeLists.txt の更新
    - `CMakeLists.txt` の `SOURCES` リストにおいて、`src/Renderer.cpp` の直下に新設した5つのファイル（`src/Renderer_Initialize.cpp`, `src/Renderer_Image.cpp`, `src/Renderer_Update.cpp`, `src/Renderer_Draw.cpp`, `src/Renderer_Context.cpp`）を追記し、ビルド対象に組み込んだ。

### タスク8: PROJECT_ARCHITECTURE.md の更新
    - `PROJECT_ARCHITECTURE.md` 内の `Renderer` クラスの解説セクションへ追記を行った。
    - 実装の物理ファイルが6つに分割されたこと、および各ファイル（`_Initialize.cpp`, `_Image.cpp`, `_Update.cpp`, `_Draw.cpp`, `_Context.cpp`, そして大元の `.cpp`）がどの処理を担当しているかの責務一覧を明記し、AI-IDEがアーキテクチャを即座に把握できるようにドキュメントを最新化した。
