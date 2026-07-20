##### 作業指示書 REQ: Phase 23-6: BackgroundManagerの新設 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反（Managerの巨大化、Rendererへの状態混入など）がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-6_BackgroundManager.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
本フェーズでは、Phase 23-1の解体工事で姿を消した背景画像を、完全に独立した美しい描画パイプラインとして復活させるため、「空間の司令塔」となる `BackgroundManager` を新設する。
*   **要件1: BackgroundManager クラスの新設**
    *   `src/BackgroundManager.h` および `src/BackgroundManager.cpp` を新規作成する。
    *   背景アートの保持、クロスフェード状態の管理、描画層（Renderer）への画像提供などを統括するAPIを設計する。
*   **要件2: FileManager との非同期連携による背景ロード**
    *   新曲決定時等に、`FileManager` を用いて画像バイナリを非同期抽出し、WICを用いてデコードする仕組みを構築する。UIスレッドをブロックしない設計とすること。
*   **要件3: クロスフェードアニメーションの管理**
    *   新しい背景画像がロードされた際、古い背景画像から新しい背景画像へと滑らかに遷移（クロスフェード）するアニメーションの進行状態（ブレンド率など）を管理する機構を設計する。
*   **要件4: 司令塔 (Application) および 描画層 (Renderer) との結線**
    *   `Application` クラスが `BackgroundManager` を保持・初期化し、必要に応じて画像のロード発注（または更新指示）を行うように配線を整理する。
    *   `Renderer` 側が背景を描画する際、直接 `BackgroundManager` から現在の背景描画に必要な情報（現在画像、過去画像、ブレンド率など）を取得して描画できるように結線する（描画処理の完全分離）。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-6_BackgroundManager.md）に、タスクごとに【作業ファイル】と【作業内容】を分離した新フォーマットで詳細作業内容を記載すること。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は `BackgroundManager` の新設と背景描画パイプラインの復活に留める。UI操作など他の機能への影響は最小限に抑えること。
*   **Rendererの状態非保持**: `Renderer` 自体に背景のクロスフェード状態（タイマーや過去画像の保持など）を持たせないこと。状態の管理はすべて `BackgroundManager` で行うこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-6 Task 1 : BackgroundManagerクラスの基盤作成
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1939_RES_Phase23-6_BackgroundManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-6 Task 1: BackgroundManagerクラスの基盤作成である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1939_RES_Phase23-6_BackgroundManager.md）を読み、今回の自分のスコープが「タスク1」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1939_RES_Phase23-6_BackgroundManager.md）の「タスク1」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク1: BackgroundManagerクラスの基盤作成
    **【作業ファイル】**
    - `ファイル名` (新規作成/編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク1の実装が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
空間の司令塔となる `BackgroundManager` クラスの枠組み（非同期ワーカーの基盤）を新規に構築する。
*   **要件1: クラスの新規作成**
    *   `src/BackgroundManager.h` および `src/BackgroundManager.cpp` を新規作成すること。
*   **要件2: 非同期ワーカーの骨組み実装**
    *   バックグラウンドで画像デコード処理等を行うための専用ワーカースレッド（`std::thread`）、タスクキュー、ミューテックス（`std::mutex`）、および条件変数（`std::condition_variable`）を用いたスレッドセーフな非同期処理の基盤を実装すること。
    *   `Initialize` および `Uninitialize` メソッドを定義し、安全なスレッドの起動と終了（join）を行えるようにすること。
*   **要件3: CMakeLists.txt の更新**
    *   新規作成した `BackgroundManager.h` と `BackgroundManager.cpp` を `CMakeLists.txt` に追加し、コンパイルが通る状態にすること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクはクラスの骨組みと非同期ワーカーの基盤作成のみを行う。`FileManager` との連携（タスク2）や、`Application` / `Renderer` への結線（タスク4, 5）は絶対にフライングで行わないこと。
