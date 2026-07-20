##### 作業指示書 REQ: Phase 23-7: パイプラインの完全開通と司令塔の整理 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-7_PipelineCompletion.md`）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
本フェーズでは、Phase 23の総仕上げとして、`Application` クラスにおける曲ロードフローを整理し、新設した各Managerへのデータ分配を完成させる。また、旧アーキテクチャ解体時の残骸を大掃除し、一時的に失われていた機能を修復する。
*   **要件1: 司令塔 (Application) のデータ分配フローの完成**
    *   `src/Application_Playback.cpp` 等の曲決定時（`PlayCurrentTrack` 等）の処理を見直し、曲のパスを基に `AudioManager`（音声再生）、`ThumbnailManager`（サムネイル生成とID取得）、`BackgroundManager`（背景ロードと時間管理）へ的確に処理を委譲（発注）する美しいパイプラインへと整理・純化する。
*   **要件2: バックグラウンド表示モードの復活 (Hotfix)**
    *   `ConfigManager::GetBackgroundArtMode()` (0:再生中, 1:非表示, 2:デフォルト固定) の設定に基づき、背景の描画を適切に切り替える処理を復活させる。
    *   状態管理と画像の提供は `BackgroundManager` が行い、表示の分岐（描画するか否か、デフォルトに差し替えるか等）は描画層である `Renderer` 側で行うこと。
*   **要件3: コードの大掃除（工事残骸のパージ）**
    *   `src/Renderer_Draw.cpp` 等に散存する「Phase 23-1で削除」や「背景マネージャ結線待ち」といった解体・工事用の不要なマーカーコメントを徹底的に検索し、全て削除してコードをクリーンアップする。

###### 【タスクごとの終了後作業】
1. 作業レポート（YYYYMMDD_HHMM_RES_Phase23-7_PipelineCompletion.md）に、タスクごとに【作業ファイル】と【作業内容】を分離した新フォーマットで詳細作業内容を記載すること。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

-------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 23-7 Task 1 : 司令塔 (Application) のデータ分配フローの整理
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_2100_RES_Phase23-7_PipelineCompletion.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-7 Task 1: 司令塔 (Application) のデータ分配フローの整理である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_2100_RES_Phase23-7_PipelineCompletion.md）を読み、今回の自分のスコープが「タスク1」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_2100_RES_Phase23-7_PipelineCompletion.md）の「タスク1」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク1: 司令塔 (Application) のデータ分配フローの整理
    **【対象ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク1の実装が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
*   `src/Application_Playback.cpp` 内の `PlayCurrentTrack` 等において、曲決定時に `ThumbnailManager::GetOrGenerateThumbId` および `EnqueueTrack` を用いたサムネイル発注処理など、各門番（Manager）へのデータ分配と非同期処理の依頼が漏れなく的確に行われるよう、パイプラインの結線を整理し純化すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `Application` 側のデータ分配フローの配線整理のみを行う。背景表示モードの復活（タスク2）や、コードの大掃除（タスク3）は絶対にフライングで行わないこと。
