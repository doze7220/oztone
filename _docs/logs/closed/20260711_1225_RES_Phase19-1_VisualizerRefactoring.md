# RES:実装計画・作業レポート Phase 19-1: ビジュアライザ描画ロジックの分離・プラグイン化準備

## 1. 実装目的
現在の `src/Visualizer.cpp` に混在している「波形データの処理」と「画面へのピクセル描画ロジック」を完全に分離し、将来的なDLLプラグイン化を見据えた拡張性の高いアーキテクチャへとリファクタリングする。

## 2. アーキテクチャ設計
### 要件1: 共通描画インターフェースの新設
    - `src/IVisualizerStyle.h` を新規作成する。
    - 描画を担う共通インターフェース `IVisualizerStyle` を定義し、`virtual void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) = 0;` などの純粋仮想関数を持たせる。共通の初期化・リソース解放等のライフサイクルも定義する。

### 要件2: 各ビジュアライザの独立クラス化 (プラグイン化の準備)
    - `src/Visualizer_PrismBeat.h` / `.cpp` を新規作成する。`IVisualizerStyle` を実装する `VisualizerPrismBeat` クラスを定義し、現在の `Visualizer::DrawPrismBeat` のロジックとネオンブラシ等の専用リソースをすべて移植する。
    - `src/Visualizer_HaloDust.h` / `.cpp` を新規作成する。`IVisualizerStyle` を実装する `VisualizerHaloDust` クラスを定義し、現在の `Visualizer::DrawCircleParticle` のロジック、専用リソース、およびパーティクル・レーザーの構造体や計算処理をすべて移植する。

### 要件3: 本体 (Visualizer) のファサード（頭脳）化
    - `src/Visualizer.h` / `.cpp` を改修し、自身では描画処理を持たない「頭脳・管理役」とする。
    - 内部に `IVisualizerStyle` のインスタンス（`VisualizerPrismBeat`、`VisualizerHaloDust`）を保持し、`ConfigManager` から取得したモードに応じて、対応するインスタンスの `Draw` メソッドを呼び出すようにルーティングする設計とする。

### 要件4: 依存関係とビルド設定の更新
    - `CMakeLists.txt` のソースリストに新規追加ファイル（`Visualizer_PrismBeat.cpp`, `Visualizer_HaloDust.cpp`）を登録し、正常にビルドできるようにする。

## 3. 実装タスクリスト
- [x] タスク1: 共通インターフェース `IVisualizerStyle.h` の作成
    - `IVisualizerStyle` インターフェースを定義し、純粋仮想関数 `Draw` 等を追加する。
- [x] タスク2: `VisualizerPrismBeat` クラスの作成と移植
    - `src/Visualizer_PrismBeat.h` / `.cpp` を作成し、PrismBeatの実装とリソースを `Visualizer.cpp` から移植する。
- [x] タスク3: `VisualizerHaloDust` クラスの作成と移植
    - `src/Visualizer_HaloDust.h` / `.cpp` を作成し、HaloDustの実装、パーティクル・レーザー構造体を `Visualizer.cpp` から移植する。
- [x] タスク4: `Visualizer` クラスのファサード化
    - `src/Visualizer.h` / `.cpp` を改修し、描画処理を削除し、各スタイルのインスタンス生成とモードに応じた `Draw` のルーティング処理を実装する。
- [x] タスク5: `CMakeLists.txt` の更新
    - 新規ファイル2つを `SOURCES` と `HEADERS` に追加する。
- [x] タスク6: ドキュメントの更新
    - 実行完了後、本ファイル（RESファイル）のタスクリストおよび詳細作業内容を更新し、完了をマークする。
    - `PROJECT_ARCHITECTURE.md` の `Visualizer` クラスの解説部分を、ファサード化された新しいアーキテクチャの仕様に更新する。

## 4. 詳細作業内容
### タスク1: 共通インターフェース `IVisualizerStyle.h` の作成
    - `src/IVisualizerStyle.h`を新規作成し、描画(`Draw`)や初期化(`Initialize`)、リソース解放(`ReleaseResources`)を定義したインターフェースを作成した。

### タスク2: `VisualizerPrismBeat` クラスの作成と移植
    - `src/Visualizer_PrismBeat.h`と`.cpp`を作成。
    - 元の `Visualizer::DrawPrismBeat` ロジックを移植し、ネオン用ブラシ等のプライベートメンバー・初期化処理を移行した。

### タスク3: `VisualizerHaloDust` クラスの作成と移植
    - `src/Visualizer_HaloDust.h`と`.cpp`を作成。
    - 元の `Visualizer::DrawCircleParticle` ロジックおよび `Particle`, `LaserRay` 構造体をクラス内に移植した。

### タスク4: `Visualizer` クラスのファサード化
    - `src/Visualizer.h`から描画ロジックに関するメンバー変数・関数を削除し、`std::unique_ptr<IVisualizerStyle>` で `VisualizerPrismBeat` と `VisualizerHaloDust` を保持する構成に変更。
    - `Visualizer.cpp` では `Initialize` や `Draw` が呼ばれた際、設定モードに応じて適切なスタイルインスタンスへ処理を委譲するようルーティング処理を実装した。

### タスク5: `CMakeLists.txt` の更新
    - `SOURCES`に `src/Visualizer_PrismBeat.cpp`, `src/Visualizer_HaloDust.cpp` を追加した。
    - `HEADERS`に `src/IVisualizerStyle.h`, `src/Visualizer_PrismBeat.h`, `src/Visualizer_HaloDust.h` を追加した。

### タスク6: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の `Visualizer` セクションを更新し、ファサードパターンの導入と新設した `IVisualizerStyle` 等の構造について記述を反映した。
    - 現在のドキュメント (RESファイル) のタスク完了チェックと作業内容を追記した。
