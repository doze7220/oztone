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
