##### 作業指示書 REQ: Phase 20-1: 背景アートフレーミング設定とプレイリストの分離 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-1_FramingDataSeparation.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

###### 【実装要件】
現在、PlaylistManager によってプレイリストファイル（`.ozl`）内に保存されている背景アートのフレーミング設定（`artOffsetX`, `artOffsetY`, `artScale`）を完全に分離し、独立したデータベースで管理するアーキテクチャへリファクタリングする。

*   **要件1: ArtFramingDatabase クラスの新設 (データ層の独立)**
    *   曲のファイルパス（または画像パス）をキーとして、フレーミング設定（X, Y, Scale）を保持・管理する `ArtFramingDatabase` クラスを新設する。
    *   TrackDatabase と同様に、`oztone_framing.odb`（またはTSV形式）として独立したファイルに設定を永続化（Save/Load）する処理を実装する。
*   **要件2: PlaylistManager の純化と後方互換性（マイグレーション）**
    *   `PlaylistManager` および `TrackMetadata` (または `PlaylistItem`) からフレーミング関連の変数と保存ロジック（`.ozl` への出力）を削除し、純粋なリスト管理クラスに戻す。
    *   **【重要】後方互換性:** `PlaylistManager::LoadFromFile` にて、古い `.ozl` ファイル（フレーミングデータが含まれているフォーマット）を読み込んだ際は、そのフレーミングデータを `ArtFramingDatabase` へ移譲（マイグレーション）する安全なフォールバック処理を実装し、ユーザーの過去の設定データを保護する。
*   **要件3: Application 等のルーティング修正**
    *   右ドラッグ等によるフレーミング操作が行われた際、保存先を `PlaylistManager` から `ArtFramingDatabase` へ変更する。
    *   曲の再生開始時や背景アートの描画更新時、`LayoutCalculator` や `Renderer` へ渡すフレーミング設定値を `ArtFramingDatabase` から取得するよう連携ロジックを修正する。

###### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-1_FramingDataSeparation.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、データ構造の分離に関する仕様を修正・追記すること。
3. D:\ozlab\oztone\task.md の「保存データの責務分離」タスクを更新する前提で計画を立てること。

---------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 20-1 Task 1: ArtFramingDatabase クラスの作成 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトは「実装実行」である。直ちに以下の【実装要件】に従ってコードの作成・修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_0134_RES_Phase20-1_FramingDataSeparation.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク1の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

###### 【実装要件】
Phase 20-1 のタスク1として、背景フレーミング情報を独立して管理・永続化するデータベースクラス `ArtFramingDatabase` を新規作成する。

*   **1. `ArtFramingDatabase.h` / `.cpp` の新規作成**
    *   `src/ArtFramingDatabase.h` および `src/ArtFramingDatabase.cpp` を作成する。
    *   曲のファイルパス（`std::wstring`）をキーとして、フレーミング設定（`artOffsetX`, `artOffsetY`, `artScale` の構造体など）を保持するデータ構造（`std::unordered_map` 等）を定義する。
    *   複数スレッドからアクセスされることを想定し、内部データへのアクセスは `std::mutex`（または `std::shared_mutex`）で保護しスレッドセーフにすること。
    *   以下のパブリックメソッドを実装する。
        *   `bool GetFraming(const std::wstring& filepath, float& outX, float& outY, float& outScale) const;`
        *   `void SetFraming(const std::wstring& filepath, float x, float y, float scale);`
        *   `void LoadFromFile(const std::wstring& dbPath);`
        *   `void SaveToFile(const std::wstring& dbPath) const;`

*   **2. TSV形式によるファイル入出力**
    *   `SaveToFile` および `LoadFromFile` は、タブ区切り（TSV）形式で読み書きを行うこと（フォーマット例: `filepath \t offsetX \t offsetY \t scale`）。
    *   パース時はエラー耐性を持たせ、不正な行はスキップすること。

*   **3. ビルド環境の更新**
    *   `CMakeLists.txt` を修正し、新規作成した `src/ArtFramingDatabase.cpp` と `src/ArtFramingDatabase.h` をビルド対象に追加すること。

--------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 20-1 Task 2: PlaylistManager の純化とマイグレーション (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトは「実装実行」である。直ちに以下の【実装要件】に従ってコードの作成・修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_0134_RES_Phase20-1_FramingDataSeparation.md）の「タスク2」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク2の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

###### 【実装要件】
Phase 20-1 のタスク2として、`PlaylistManager` からフレーミング設定の管理責務を完全に削除し、古いプレイリストファイルからのマイグレーション（データ移行）処理を実装する。

*   **1. `TrackMetadata` の純化と関連メソッドの削除**
    *   `src/PlaylistManager.h` の `TrackMetadata` 構造体から、フレーミング関連の変数（`artOffsetX`, `artOffsetY`, `artScale` など）を完全に削除する。
    *   `PlaylistManager` クラスに存在していたフレーミング更新・取得用のメソッド（`UpdateFraming`, `GetFraming` 等）を削除する。

*   **2. `PlaylistManager::SaveToFile` のフォーマット修正**
    *   `src/PlaylistManager.cpp` の `SaveToFile` メソッドを修正し、フレーミング情報（X, Y, Scale）の出力処理を削除する。
    *   純粋なプレイリストメタデータ（`filepath \t title \t artist \t timeString` など）のみを出力するフォーマットへと純化させる。

*   **3. `PlaylistManager::LoadFromFile` の後方互換性とマイグレーション**
    *   `LoadFromFile` メソッドのシグネチャを変更し、移行先のデータベースを受け取れるよう引数に `class ArtFramingDatabase* framingDb = nullptr` を追加する（必要に応じて前方宣言するかヘッダをインクルードする）。
    *   TSV行のパース時、**古いフォーマット（列数が多く、フレーミング情報が含まれている行）を検知した場合は、その情報を抽出し、`framingDb` が有効であれば `framingDb->SetFraming(filepath, x, y, scale)` を呼び出して新データベースへデータを自動移行（マイグレーション）させる**。
    *   パース処理自体は古いフォーマットであってもエラーにならず、曲のメタデータ部分（`TrackMetadata`）を正しく復元できるようにフォールバック処理を堅牢に保つこと。


##### 作業指示書 REQ: Phase 20-1 Task 4: ドキュメントの更新 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトは「実装実行（ドキュメント更新）」である。直ちに以下の【実装要件】に従ってファイルの修正を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_0134_RES_Phase20-1_FramingDataSeparation.md）の「タスク3」および「タスク4」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること（前回タスク3まで先行して実装したため）。
3. チャットにて「Phase 20-1 の全タスクが完了しました。」と報告すること。

---

###### 【実装要件】
Phase 20-1 のタスク4として、今回実装したデータ分離のアーキテクチャをプロジェクト資料へ反映する。

*   **1. `PROJECT_ARCHITECTURE.md` の更新**
    *   `PlaylistManager` クラスの解説部分から、フレーミング情報（artOffsetX, artOffsetY, artScale）の保持・保存に関する記述を削除する。
    *   適切な箇所（`TrackDatabase` 等のデータ層クラスの並び）に、新設した `ArtFramingDatabase` クラスに関する解説を追記する。「曲のファイルパスをキーとして、背景アートのフレーミング設定を独立したデータベース（TSV形式）で永続化・管理する責務を持つ」といった内容を含めること。

*   **2. `task.md` の更新**
    *   【機能新規実装(UIが余り関係しない機能)】セクション等に記載されている「保存データの責務分離」に関するタスクのチェックボックスを `[x]`（完了）に更新すること。
