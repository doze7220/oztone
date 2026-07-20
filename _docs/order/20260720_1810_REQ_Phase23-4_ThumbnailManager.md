##### 作業指示書 REQ: Phase 23-4: ThumbnailManagerの新設 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反（Managerの巨大化など）がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-4_ThumbnailManager.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
本フェーズでは、「サムネイルに関すること」をすべて統括する視覚の司令塔である `ThumbnailManager` クラスを新設し、関連クラスをその配下にカプセル化・再編する。
*   **要件1: ThumbnailManager クラスの新設**
    *   `src/ThumbnailManager.h` および `src/ThumbnailManager.cpp` を新規作成する。
    *   外部（Application等）に対するサムネイル操作の唯一の窓口となるAPI（Facade）を設計する。
*   **要件2: ThumbCacher と ThumbnailDatabase のカプセル化**
    *   現在 `Application` クラスが直接保持・操作している `ThumbCacher` と `ThumbnailDatabase` を `ThumbnailManager` の内部に移動させ、外部の依存から隠蔽する。
*   **要件3: 外部クラスの配線付け替え**
    *   `Application` や描画層等で直接 `ThumbCacher` や `ThumbnailDatabase` にアクセスしていた箇所を、すべて新設した `ThumbnailManager` 経由に置き換える。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-4_ThumbnailManager.md）に、タスクごとに【作業ファイル】と【作業内容】を分離した新フォーマットで詳細作業内容を記載すること。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は `ThumbnailManager` の新設と、既存コンポーネント（ThumbCacher, ThumbnailDatabase）のカプセル化・配線付け替えのみを行う。画像処理ロジック自体の変更は行わない。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-4 Task 1 : ThumbnailManagerのクラス定義と実装作成
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1810_RES_Phase23-4_ThumbnailManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-4 Task 1: ThumbnailManagerのクラス定義と実装作成である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1810_RES_Phase23-4_ThumbnailManager.md）を読み、今回の自分のスコープが「タスク1」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1810_RES_Phase23-4_ThumbnailManager.md）の「タスク1」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク1: ThumbnailManagerのクラス定義と実装作成
    **【作業ファイル】**
    - `ファイル名` (新規作成/編集/削除/リネーム等)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク1の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
視覚の司令塔となる `ThumbnailManager` クラスの枠組みを新規に構築する。
*   **要件1: ThumbnailManagerクラスの新設**
    *   `src/ThumbnailManager.h` および `src/ThumbnailManager.cpp` を新規作成すること。
    *   クラスのメンバとして、`ThumbnailDatabase` および `ThumbCacher` を内部で保持（包含またはポインタ管理）すること。
    *   外部（Application等）に対するサムネイル操作の単一の窓口となるAPI（初期化、サムネイルID取得、画像の取得、バックグラウンド生成のエンキューなど、既存の処理をカプセル化できるメソッド群）を設計・実装し、内部のコンポーネントへ処理を委譲（ルーティング）すること。
    *   ※ このファイル内で必要なインクルードのみを行い、依存関係を極力隠蔽すること。
*   **要件2: CMakeLists.txtの更新**
    *   新規作成した `src/ThumbnailManager.cpp` および `src/ThumbnailManager.h` を CMakeLists.txt の適切な位置に追加し、ビルドが通る状態を確認すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `ThumbnailManager` の新規作成（枠組み作り）のみを行う。`Application` や `Renderer` など、他の外部クラスからの呼び出し箇所の書き換え（タスク2以降）は絶対にフライングで行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-4 Task 2 : Applicationクラスの配線付け替え (ヘッダ・コンストラクタ・初期化)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1810_RES_Phase23-4_ThumbnailManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-4 Task 2: Applicationクラスの配線付け替え (ヘッダ・コンストラクタ・初期化)である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1810_RES_Phase23-4_ThumbnailManager.md）を読み、今回の自分のスコープが「タスク2」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク2のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1810_RES_Phase23-4_ThumbnailManager.md）の「タスク2」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク2: Applicationクラスの配線付け替え (ヘッダ・コンストラクタ・初期化)
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク2の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在 `Application` クラスが直接保持・初期化しているサムネイル関連コンポーネントを、新設した `ThumbnailManager` へ統合する。
*   **要件1: Application.h のメンバ変数統合**
    *   `src/Application.h` において、`ThumbnailDatabase m_thumbnailDatabase;` および `ThumbCacher m_thumbCacher;` の2つのメンバ変数を削除し、代わりに `ThumbnailManager m_thumbnailManager;` 1つに統合すること。
    *   それに伴い、インクルードを `ThumbnailManager.h` へ変更すること。
*   **要件2: Application.cpp (コンストラクタ/デストラクタ) の修正**
    *   `src/Application.cpp` のコンストラクタ初期化子リスト、またはデストラクタ内での終了処理（スレッドの待機等が存在する場合）を、`m_thumbnailManager` 経由の呼び出しへ適切に修正すること。
*   **要件3: Application_Initialize.cpp の初期化ルーチン修正**
    *   `src/Application_Initialize.cpp` 内の `Application::Initialize` メソッドにおいて、個別に呼び出していた `m_thumbnailDatabase.Initialize()` と `m_thumbCacher.Initialize()` を削除し、`m_thumbnailManager.Initialize()` などの適切な初期化メソッド呼び出しへ置き換えること（※必要であれば ThumbnailManager 側に Initialize メソッドを実装して委譲させること）。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは初期化周りの配線切り替えのみを行う。再生、プレイリスト、レンダリング等の処理におけるメソッド呼び出しの置き換え（タスク3）や Renderer の配線切り替え（タスク4）は絶対にフライングで行わないこと。
*   **段階的なコンパイル** : 本タスク完了時点では、タスク3以降のコード（Application_*.cpp の他ファイル等）がコンパイルエラーになることが予想されるが、それで正常である。まずは初期化の配線のみを確実に完了させること。
