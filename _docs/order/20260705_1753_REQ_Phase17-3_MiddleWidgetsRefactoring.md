### 作業指示書 REQ: Phase 17-3: 中型Widgetの独立化1 (計画立案)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase17-3_MiddleWidgetsRefactoring.md`）として新規作成すること。
3. レポートのフォーマットは、`PROJECT_CONSTITUTION.md` の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
現在 `Widgets.h` および `Widgets.cpp` に実装されている `VolumeControlWidget` と `PlaybackControlsWidget` を、それぞれ独立したファイルへと切り出すリファクタリングを実行する。機能の追加やロジックの変更は一切行わないこと。

* **要件1: 新規ファイルの作成とコードの移行**
  * `src/VolumeControlWidget.h`, `src/VolumeControlWidget.cpp` (新規作成)
  * `src/PlaybackControlsWidget.h`, `src/PlaybackControlsWidget.cpp` (新規作成)
  * 現在 `Widgets.h` / `Widgets.cpp` にある上記の2クラスの宣言と実装を完全に移動する。
* **要件2: 依存関係の整理**
  * 各新規ヘッダにおいて、`Widget.h`、`WidgetContext.h` など必要なインクルードを過不足なく記述する。
  * `Renderer.cpp` など呼び出し元で必要なインクルードを追加し、コンパイルが通るよう依存関係を整理する。
* **要件3: ビルド設定の更新**
  * `CMakeLists.txt` を更新し、新規作成した4つのファイルをビルド対象に追加する。

#### 【作業終了後】
1. 作業レポート（`...RES_Phase17-3_MiddleWidgetsRefactoring.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **純粋なリファクタリングの徹底**: 本作業はファイルの分割のみを目的とする。変数の追加、関数のシグネチャ変更、UIのレイアウトや描画ロジックの変更は **絶対に行ってはならない**。
