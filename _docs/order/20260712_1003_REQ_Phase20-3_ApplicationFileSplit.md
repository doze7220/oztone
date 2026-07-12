### 作業指示書 REQ: Phase 20-3: Application.cpp のファイル分割 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-3_ApplicationFileSplit.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
肥大化した `Application.cpp` について、クラスの定義（`Application.h`）や依存関係は一切変更せず、AI-IDEのコンテキスト節約および可読性向上のため、実装のみを責務別の `.cpp` ファイルに物理分割する。

*   **要件1: 実装ファイルの分割**
    既存の `Application.cpp` の実装を、以下の6ファイルに分割する。
    1.  `Application.cpp`: コンストラクタ、デストラクタなど全体管理やエントリポイントに関わる最小限の処理。
    2.  `Application_Initialize.cpp`: `Initialize`, `SetupCallbacks` など初期化処理に関する処理。
    3.  `Application_Playback.cpp`: `PlayCurrentTrack`, `HandleMediaCommand`, `PrefetchNextTrack`, `UpdateTrackMetadataIfNeeded` など再生・メディア操作に関する処理。
    4.  `Application_Playlist.cpp`: `SwitchPlaylist`, `ClearPlaylist`, `CreateNewPlaylist`, `OnPlaylistClicked`, `OnPlaylistDoubleClicked`, `OnPlaylistToolbarClicked` などプレイリスト制御に関する処理。
    5.  `Application_FileDrop.cpp`: `OnFilesDropped`, `ProcessCommandLineArgs` など外部からのファイル入力に関する処理。
    6.  `Application_Render.cpp`: `Run`, `ForceRender`, `UpdatePlaylistSummaries` などメインループおよび描画連携に関する処理。

*   **要件2: `CMakeLists.txt` の更新**
    新設した `Application_*.cpp` ファイル群をビルド対象として `CMakeLists.txt` に追加する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-3_ApplicationFileSplit.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や `Application.h` 内のメンバ変数の移動など、クラス設計（論理構造）の変更は絶対に行わないこと。
*   **機能変更の禁止**: 既存の機能や動作、アーキテクチャの役割分担は絶対に壊さないこと。


--------------------------------------------------------------------------------


##### 作業指示書 REQ: Phase 20-3 Task 1: Application_Initialize.cpp の作成 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_0134_RES_Phase20-1_FramingDataSeparation.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリング（ファイル分割）の「実装実行」である。直ちに以下の【実装要件】に従ってコードの分割・移行を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_1004_RES_Phase20-3_ApplicationFileSplit.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク1（Application_Initialize.cpp の作成）が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

###### 【実装要件】
`Application.h` のクラス定義や既存の機能・ロジックは一切変更せず、`Application.cpp` の実装の一部を新規ファイルに物理分割する。今回はタスク1のみを実行し、完了時点でビルドが通る状態を維持すること。

*   **1. `Application_Initialize.cpp` の作成と移行**
    *   新規ファイル `src/Application_Initialize.cpp` を作成する。
    *   ファイルの先頭に `#include "Application.h"` を記述し、さらに `Initialize` と `SetupCallbacks` の実装に必要なその他のインクルード文を `Application.cpp` からコピーして配置すること。
    *   `Application.cpp` から `Application::Initialize` および `Application::SetupCallbacks` の実装（メソッド全体）を切り取り、`Application_Initialize.cpp` へ移行（ペースト）する。
*   **2. `CMakeLists.txt` の更新**
    *   `CMakeLists.txt` の `SOURCES` (ビルド対象ファイルリスト) に、新設した `src/Application_Initialize.cpp` を追加する。

###### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や `Application.h` 内のメンバ変数の移動など、論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: コピー＆ペーストによる安全な移行を心がけ、既存の動作を絶対に壊さないこと。

-----------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 20-3 Task 2 & 3: Playback と Playlist の分離 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_0134_RES_Phase20-1_FramingDataSeparation.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリング（ファイル分割）の「実装実行」である。直ちに以下の【実装要件】に従ってコードの分割・移行を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_1004_RES_Phase20-3_ApplicationFileSplit.md）の「タスク2」「タスク3」「タスク7（一部）」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク2と3のファイル分割が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

###### 【実装要件】
`Application.h` のクラス定義や既存の機能・ロジックは一切変更せず、`Application.cpp` の実装の一部を2つの新規ファイルに物理分割する。完了時点でビルドが通る状態を維持すること。

*   **1. `Application_Playback.cpp` の作成と移行 (タスク2)**
    *   新規ファイル `src/Application_Playback.cpp` を作成する。
    *   先頭に `#include "Application.h"` および必要なインクルード文を配置すること。
    *   `Application.cpp` から `PlayCurrentTrack`, `HandleMediaCommand`, `PrefetchNextTrack`, `UpdateTrackMetadataIfNeeded` の実装を切り取り、移行する。
*   **2. `Application_Playlist.cpp` の作成と移行 (タスク3)**
    *   新規ファイル `src/Application_Playlist.cpp` を作成する。
    *   先頭に `#include "Application.h"` および必要なインクルード文を配置すること。
    *   `Application.cpp` から `SwitchPlaylist`, `ClearPlaylist`, `CreateNewPlaylist`, `OnPlaylistClicked`, `OnPlaylistDoubleClicked`, `OnPlaylistToolbarClicked` の実装を切り取り、移行する。
*   **3. `CMakeLists.txt` の更新 (タスク7の一部)**
    *   `CMakeLists.txt` の `SOURCES` に、新設した `src/Application_Playback.cpp` と `src/Application_Playlist.cpp` を追加する。

###### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: コピー＆ペーストによる安全な移行を心がけ、既存の動作を絶対に壊さないこと。

-----------------------------------------------------------------------------