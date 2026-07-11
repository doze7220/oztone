### 作業指示書 REQ: Phase 19-1: ビジュアライザ描画ロジックの分離・プラグイン化準備 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-1_VisualizerRefactoring.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
現在の `src/Visualizer.cpp` に混在している「波形データの処理」と「画面へのピクセル描画ロジック」を完全に分離し、将来的なDLLプラグイン化を見据えた拡張性の高いアーキテクチャへとリファクタリングする。

*   **要件1: 共通描画インターフェースの新設**
    *   `src/IVisualizerStyle.h` を新規作成し、描画を担う共通インターフェース `IVisualizerStyle` を定義する。
    *   `virtual void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) = 0;` などの純粋仮想関数を持たせ、共通のライフサイクル（初期化・リソース解放等）を定義する。

*   **要件2: 各ビジュアライザの独立クラス化 (プラグイン化の準備)**
    *   `src/Visualizer_PrismBeat.h` / `.cpp` を新規作成し、`IVisualizerStyle` を実装する `VisualizerPrismBeat` クラスを定義。現在の `Visualizer::DrawPrismBeat` のロジックと、それに紐づく専用リソース（ネオンブラシ等）をすべて移植する。
    *   `src/Visualizer_HaloDust.h` / `.cpp` を新規作成し、同様に `VisualizerHaloDust` クラスを定義。現在の `Visualizer::DrawCircleParticle` のロジック、専用リソース、およびパーティクルやレーザーの構造体・計算処理をすべて移植する。

*   **要件3: 本体 (Visualizer) のファサード（頭脳）化**
    *   `src/Visualizer.h` / `.cpp` を改修し、自身では描画処理を持たない「頭脳・管理役」とする。
    *   内部に `IVisualizerStyle` のインスタンス群（PrismBeatとHaloDust）を生成して保持し、ConfigManager から取得したモードに応じて、対応するインスタンスの `Draw` を呼び出すようにルーティングする。

*   **要件4: 依存関係とビルド設定の更新**
    *   `CMakeLists.txt` のソースリストに、新規作成した `Visualizer_PrismBeat.cpp` および `Visualizer_HaloDust.cpp` を追加し、正常にコンパイルおよびリンクが行われるようにする。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-1_VisualizerRefactoring.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **ロジック不変の原則**: 本フェーズはリファクタリング（ファイルの分離）のみを目的とする。描画の見た目や波形の計算式（対数マッピング等）を勝手に変更・追加してはならない。
