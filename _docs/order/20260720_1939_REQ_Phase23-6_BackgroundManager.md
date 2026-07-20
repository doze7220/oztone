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

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-6 Task 2 : FileManager連携とWICデコードの実装
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1939_RES_Phase23-6_BackgroundManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-6 Task 2: FileManager連携とWICデコードの実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1939_RES_Phase23-6_BackgroundManager.md）を読み、今回の自分のスコープが「タスク2」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク2のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1939_RES_Phase23-6_BackgroundManager.md）の「タスク2」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク2: FileManager連携とWICデコードの実装
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク2の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
新設した `BackgroundManager` のワーカースレッド内にて、画像バイナリの非同期抽出とWICデコード処理を実装する。
*   **要件1: 画像ロード要求（発注）インターフェースの追加**
    *   `src/BackgroundManager.h` に、外部から背景画像のロードを要求するためのメソッド（例: `RequestLoad` 等）を追加すること。
    *   要求されたファイルパス（`std::wstring` 等）をキューへスレッドセーフにエンキューし、条件変数（`condition_variable`）でワーカーを起床させる処理を実装すること。
*   **要件2: FileManagerからのバイナリ抽出とWICデコード (WorkerLoopの実装)**
    *   `src/BackgroundManager.cpp` のワーカースレッドループ内にて、キューから取り出したファイルパスに対して `FileManager::ExtractAlbumArtBinary` を静的呼び出しし、画像バイナリを抽出すること。
    *   抽出したバイナリデータが存在する場合、WIC (`IWICImagingFactory` 等) を用いてデコードし、`IWICFormatConverter`（PBGRA形式等）の状態でスレッドセーフにクラス内部へ保持する処理を実装すること。（※D2D1Bitmapの生成はUIスレッドを持つRendererで行う必要があるため、ここではWICレベルまでのデコード・保持に留めること）

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクはFileManagerを用いた非同期画像抽出とWICデコードの処理実装のみを行う。クロスフェード状態の管理（タスク3）や、Application / Renderer への結線（タスク4, 5）は絶対にフライングで行わないこと。
*   **D2D非依存の原則** : ワーカースレッド内では `ID2D1Bitmap` の生成は行わず、必ずWICリソースとして保持すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-6 Task 3 : クロスフェード状態管理の実装
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1939_RES_Phase23-6_BackgroundManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-6 Task 3: クロスフェード状態管理の実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1939_RES_Phase23-6_BackgroundManager.md）を読み、今回の自分のスコープが「タスク3」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク3のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク4以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1939_RES_Phase23-6_BackgroundManager.md）の「タスク3」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク3: クロスフェード状態管理の実装
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク3の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`BackgroundManager` 内で、背景アートのクロスフェードアニメーション（ブレンド率の更新等）を管理する状態更新ロジックを実装する。
*   **要件1: クロスフェード用の状態変数の追加**
    *   `src/BackgroundManager.h` に、フェードの進行度（例: `m_fadeProgress`、0.0f〜1.0f）、古いWIC画像（例: `m_oldWicImage`）を保持する変数を追加すること。
*   **要件2: アニメーション更新処理 (UpdateAnimation) の実装**
    *   `src/BackgroundManager.h` および `src/BackgroundManager.cpp` に、毎フレーム呼び出される `UpdateAnimation(float deltaTime)` メソッドを追加・実装すること。
    *   `UpdateAnimation` 内で、フェード中であれば進行度を加算し、1.0fに達したらフェード完了として `m_oldWicImage` を解放するなどの状態遷移ロジックを実行すること。
*   **要件3: 画像切り替え時の状態遷移ロジックの実装**
    *   ワーカースレッドで新しいWIC画像のデコードが完了し、それを表示用に反映させるタイミングで、現在の画像（`m_currentWicImage`）を `m_oldWicImage` に退避し、新しい画像をセットして `m_fadeProgress` を 0.0f にリセットする処理を実装すること（スレッドセーフな受け渡しに配慮すること）。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクはクロスフェードの「状態管理（更新ロジックと変数追加）」のみを行う。`Application` や `Renderer` への結線（タスク4, 5）は絶対にフライングで行わないこと。
*   **D2D非依存の原則** : この層ではまだ `ID2D1Bitmap` の生成・管理は行わず、あくまでWICイメージ（`IWICFormatConverter` 等）またはメタデータに対する状態管理に留めること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-6 Task 4 : Applicationクラスとの結線
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1939_RES_Phase23-6_BackgroundManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-6 Task 4: Applicationクラスとの結線である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1939_RES_Phase23-6_BackgroundManager.md）を読み、今回の自分のスコープが「タスク4」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク4のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク5以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1939_RES_Phase23-6_BackgroundManager.md）の「タスク4」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク4: Applicationクラスとの結線
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク4の実装が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
空間の司令塔である `BackgroundManager` を `Application` クラスに保持させ、ライフサイクルと画像ロードの結線を行う。
*   **要件1: Application.h への追加**
    *   `src/Application.h` に `BackgroundManager` をメンバ変数（例: `m_backgroundManager`）として追加すること。
*   **要件2: 初期化・終了・更新処理の結線**
    *   `src/Application_Initialize.cpp` 等における初期化（`Initialize`）処理内で、`m_backgroundManager.Initialize()` を呼び出すこと。
    *   終了処理（デストラクタ等）で `Uninitialize()` を呼び出すこと。
    *   毎フレームの更新処理（`ForceRender` または `Run` 内）にて、`m_backgroundManager.UpdateAnimation(deltaTime)` を呼び出すようにすること。
*   **要件3: 曲切り替え時のロード発注の結線**
    *   `src/Application_Playback.cpp` 内の曲決定時（`PlayCurrentTrack`等）において、対象のファイルパスを `m_backgroundManager.RequestLoad(filepath)` へ渡し、背景画像の非同期ロードを発注するように結線すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `Application` クラスの配線のみを行う。`Renderer` 側への結線・画像渡し（タスク5）は絶対にフライングで行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-6 Task 5 : Rendererクラスとの結線
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1939_RES_Phase23-6_BackgroundManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-6 Task 5: Rendererクラスとの結線である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1939_RES_Phase23-6_BackgroundManager.md）を読み、今回の自分のスコープが「タスク5」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク5のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク6（ドキュメント更新）をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1939_RES_Phase23-6_BackgroundManager.md）の「タスク5」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク5: Rendererクラスとの結線
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「タスク5の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Renderer` クラスが背景を描画する際、直接 `BackgroundManager` から情報を取得し、クロスフェードを伴う美しい背景描画を行うよう結線・復元する。
*   **要件1: Renderer への BackgroundManager の参照渡し**
    *   `src/Renderer.h` および `src/Renderer.cpp` に `SetBackgroundManager(BackgroundManager* bgManager)` 等を追加し、`Application` 側から初期化時にポインタを渡せるように配線すること。（またはコンテキスト経由で受け取る）
*   **要件2: WIC から D2D ビットマップへの動的変換と保持**
    *   `Renderer` 側で、`BackgroundManager` が保持している「現在のWIC画像」と「過去のWIC画像」を取得し、D2Dビットマップ（`ID2D1Bitmap`）へ変換して描画用にキャッシュするロジックを実装すること。UIスレッド（描画スレッド）で `CreateBitmapFromWicBitmap` を呼ぶ必要があるため。
*   **要件3: Renderer_Draw.cpp (DrawBackground) の描画復活とクロスフェード**
    *   Phase 23-1 で削除された背景アルバムアートの全画面フィット・トリミング描画ロジックを復活させること。
    *   過去画像が存在する場合はそれを下地に描画し、その上に `BackgroundManager` から取得したブレンド率（`fadeProgress`）を不透明度として適用した「現在画像」を描画することで、美しいクロスフェードを実現すること。
    *   背景アートを描画した後、既存のダークオーバーレイ（黒半透明矩形）を描画する処理は維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `Renderer` 層での背景描画の復元と結線のみを行う。ドキュメントの更新（タスク6）は絶対に行わないこと。
*   **状態分離の原則** : フェードの進行度や過去画像のライフサイクル管理は `BackgroundManager` が行っているため、`Renderer` 側は「取得したWIC画像をD2Dに変換し、指定された不透明度で描画するだけ」の受動態に徹すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-6 Task 6 : アーキテクチャドキュメントの更新
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1939_RES_Phase23-6_BackgroundManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-6 Task 6: アーキテクチャドキュメントの更新である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1939_RES_Phase23-6_BackgroundManager.md）を読み、今回の自分のスコープが「タスク6」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク6のみ」** の修正を実行すること。
4. 作業完了後、既存の作業レポート（20260720_1939_RES_Phase23-6_BackgroundManager.md）の「タスク6」のチェックボックスを完了 [x] にし、必ず以下のフォーマットで詳細作業内容を追記すること。
    ### タスク6: アーキテクチャドキュメントの更新
    **【作業ファイル】**
    - `ファイル名` (編集)
    **【作業内容】**
    - 詳細な作業内容
5. チャットにて「Phase 23-6 がすべて完了しました。確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: PROJECT_ARCHITECTURE.md の更新**
    *   「5. 実装済みクラス・関数リファレンス」に `BackgroundManager` クラスの項目を新設する。
    *   記載内容は「背景アートの保持、クロスフェード、および描画層への連携を統括する空間の司令塔。Rendererから背景画像の管理責務が完全に分離され、FileManagerを用いた非同期画像抽出とWICデコードを担当する」といった旨を簡潔に記載すること。
    *   Renderer クラスの記述において、背景描画が BackgroundManager からの情報に基づくようになった旨を（もし必要であれば）微調整すること。
