##### 作業指示書 REQ: Phase 23-5: サムネイルパイプラインの完全開通 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-5_ThumbnailPipeline.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
本フェーズでは、Phase 23-1で物理切断されていたサムネイル生成工場（`ThumbCacher`）の画像バイナリ抽出処理を復活させ、新設した `FileManager` と結線することで、視覚のデータパイプラインを完全開通させる。
*   **要件1: ThumbCacher への FileManager 参照の追加**
    *   `src/ThumbCacher.h` に `FileManager` のポインタ（または参照）を保持するメンバ変数を追加する。
    *   `ThumbCacher` の初期化時等に、外部から `FileManager` を受け取れるようにメソッドのシグネチャを拡張する。
*   **要件2: ThumbnailManager の依存解決（ルーティング）**
    *   `src/ThumbnailManager.h` および `src/ThumbnailManager.cpp` を修正し、自身の初期化時（Initialize 等）に `FileManager` を受け取り、内部の `ThumbCacher` へ渡すように配線を繋ぐ。
*   **要件3: Application クラス等での配線結線**
    *   司令塔である `Application` クラス等の初期化ルーチンにて、`ThumbnailManager` の初期化時に `FileManager` (`m_fileManager`) のポインタを渡すように修正する。
*   **要件4: 画像バイナリ抽出処理の復活**
    *   `src/ThumbCacher.cpp` のワーカースレッドループ内において、対象ファイルの画像バイナリを抽出する処理を `m_fileManager` の該当メソッドを用いて復活させる。
    *   抽出したバイナリデータをWICでデコードし、サムネイルDB（`.img`）へ保存する既存の流れと接続し、コンパイルを通すこと。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-5_ThumbnailPipeline.md）に、タスクごとに【作業ファイル】と【作業内容】を分離した新フォーマットで詳細作業内容を記載すること。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は FileManager と ThumbCacher (ThumbnailManager経由) の結線、および画像抽出処理の復活のみを行う。背景アートの機能復活など、要件外の作業は絶対に行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-5 Task 1 : ThumbCacherでの画像バイナリ抽出とDB保存処理の復活
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1923_RES_Phase23-5_ThumbnailPipeline.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-5 Task 1: ThumbCacherでの画像バイナリ抽出とDB保存処理の復活である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1923_RES_Phase23-5_ThumbnailPipeline.md）を読み、今回の自分のスコープが「タスク1」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2（ドキュメント更新）をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1923_RES_Phase23-5_ThumbnailPipeline.md）の「タスク1」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク1: ThumbCacherでの画像バイナリ抽出とDB保存処理の復活
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク1の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
Phase 23-1で物理切断されていたサムネイル生成工場（`ThumbCacher`）の画像バイナリ抽出処理を復活させ、視覚のデータパイプラインを完全開通させる。
*   **要件1: 画像バイナリ抽出処理の復活**
    *   `src/ThumbCacher.cpp` を修正し、必要なヘッダ（`FileManager.h` 等）をインクルードすること。
    *   ワーカースレッド (`WorkerLoop`) 内にて、非同期にファイルパスから `FileManager::ExtractAlbumArtBinary` を静的呼び出しして画像バイナリを抽出する処理を復活・実装すること。
    *   抽出したバイナリデータが存在する場合、既存の `CookThumbnailImage` メソッドに渡してWICデコードおよびリサイズを行い、生成されたJPEGバイナリを `m_db->StoreCookedData` を用いてサムネイルDBへ保存するフローを完成させること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `ThumbCacher.cpp` の結線と抽出ロジックの復活のみを行う。アーキテクチャ資料の更新（タスク2）は絶対に行わないこと。
