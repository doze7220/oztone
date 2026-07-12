### 作業指示書 REQ: Phase 20-4: ConfigManager の実装対象別ファイル分割 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-4_ConfigManagerFileSplit.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、以下の細分化されたタスクリストをそのまま含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
現在2300行を超えて肥大化している `ConfigManager.cpp` について、クラスの定義（`ConfigManager.h`）は一切変更せず、人間の編集領域とAIの編集領域を完全分離するため、実装対象（UI）別に実装ファイルを物理分割する。
※AIのハルシネーション防止と出力トークン節約のため、1つのタスクにつき1ファイルの移行のみを行うこと。

*   **タスクリスト（厳格な1タスク1ファイル制）**
    *   [ ] **タスク1: ConfigManager_Window.cpp の作成**：ウィンドウ設定、Z-Order、可視性(Visibility)、背景アート等に関する実装を移行する。
    *   [ ] **タスク2: ConfigManager_Playlist.cpp の作成**：プレイリストUI全般に関する実装を移行する。
    *   [ ] **タスク3: ConfigManager_Playback.cpp の作成**：シークバー、再生コントロール、音量コントロールに関する実装を移行する。
    *   [ ] **タスク4: ConfigManager_LogoMenu.cpp の作成**：アプリアイコン、ロゴ拡張メニューに関する実装を移行する。
    *   [ ] **タスク5: ConfigManager_Visualizer.cpp の作成**：ビジュアライザ全般に関する実装を移行する。
    *   [ ] **タスク6: ConfigManager_System.cpp の作成**：グローバルホットキー、OSDなどシステム連携に関する実装を移行する。
    *   [ ] **タスク7: CMakeLists.txt の更新と ConfigManager.cpp の整理**：分割した6ファイルを `CMakeLists.txt` に追加し、元の `ConfigManager.cpp` には `DEFAULT_INI_CONTENT` とコアのファイルI/O処理のみを残してクリーンアップする。
    *   [ ] **タスク8: PROJECT_ARCHITECTURE.md の更新**：ConfigManager が実装対象別に7ファイルへ物理分割された旨と各ファイルの役割を追記する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-4_ConfigManagerFileSplit.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: 既存の機能や動作は絶対に壊さないこと。


--------------------------------------------------------------------------------


##### 作業指示書 REQ: Phase 20-4 Task 1: ConfigManager_Window.cpp の作成 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリング（ファイル分割）の「実装実行」である。直ちに以下の【実装要件】に従ってコードの分割・移行を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク1（ConfigManager_Window.cpp の作成）の実装が完了しました。」と報告すること。

---

###### 【実装要件】
`ConfigManager.h` のクラス定義や既存の機能・ロジックは一切変更せず、`ConfigManager.cpp` の実装の一部を新規ファイルに物理分割する。今回はタスク1のみを実行する。

*   **1. `ConfigManager_Window.cpp` の作成と移行**
    *   新規ファイル `src/ConfigManager_Window.cpp` を作成する。
    *   ファイルの先頭に `#include "ConfigManager.h"` と、その実装に必要なインクルード文を記述する。
    *   `ConfigManager.cpp` から、以下の設定に関する Getter/Setter メソッドの実装を切り取り、`ConfigManager_Window.cpp` へ完全に移行（ペースト）する。
        *   ウィンドウ設定 (サイズ、位置、リサイズモード、画面固定等)
        *   Z-Order設定
        *   可視性フラグ (Visibilityセクション関連全般)
        *   背景アート設定 (`GetBackgroundArtMode`, `GetBgOpacity` 等)
    *   ※ 注意: `CMakeLists.txt` への追加はタスク7で行うため、本タスクでは不要である。

###### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や `ConfigManager.h` 内のメンバ変数の移動など、論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: コピー＆ペーストによる安全な移行を心がけ、既存の動作を絶対に壊さないこと。


--------------------------------------------------------------------------------


##### 作業指示書 REQ: Phase 20-4 Task 2: ConfigManager_Playlist.cpp の作成 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリング（ファイル分割）の「実装実行」である。直ちに以下の【実装要件】に従ってコードの分割・移行を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md）の「タスク2」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク2（ConfigManager_Playlist.cpp の作成）の実装が完了しました。」と報告すること。

---

###### 【実装要件】
`ConfigManager.h` のクラス定義や既存の機能・ロジックは一切変更せず、`ConfigManager.cpp` の実装の一部を新規ファイルに物理分割する。今回はタスク2のみを実行する。

*   **1. `ConfigManager_Playlist.cpp` の作成と移行**
    *   新規ファイル `src/ConfigManager_Playlist.cpp` を作成する。
    *   ファイルの先頭に `#include "ConfigManager.h"` と、その実装に必要なインクルード文を記述する。
    *   `ConfigManager.cpp` から、以下の設定に関するメソッド（主にSetterなど、`.cpp`に実装が存在するもの）を切り取り、`ConfigManager_Playlist.cpp` へ完全に移行（ペースト）する。
        *   プレイリストのデフォルトパス設定 (`SetDefaultPlaylistPath` など)
        *   プレイリストの配置位置設定 (`SetPlaylistPosition` など)
        *   プレイリストのピン留め状態設定 (`SetIsPlaylistPinned` など)
        *   その他、プレイリストのレイアウト、フォント、色、ツールバー設定等に関連し、`.cpp` 側に実装が存在するすべての関数。
    *   ※ 注意: ヘッダファイル（`.h`）内でインライン定義されているだけの Getter 等は移行の対象外である。
    *   ※ 注意: `CMakeLists.txt` への追加はタスク7で行うため、本タスクでのビルド検証は保留とする。

###### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や `ConfigManager.h` 内のメンバ変数の移動など、論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: コピー＆ペーストによる安全な移行を心がけ、既存の動作を絶対に壊さないこと。



--------------------------------------------------------------------------------


##### 作業指示書 REQ: Phase 20-4 Task 3: ConfigManager_Playback.cpp の作成 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリング（ファイル分割）の「実装実行」である。直ちに以下の【実装要件】に従ってコードの分割・移行を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md）の「タスク3」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク3（ConfigManager_Playback.cpp の作成）の実装が完了しました。」と報告すること。

---

###### 【実装要件】
`ConfigManager.h` のクラス定義や既存の機能・ロジックは一切変更せず、`ConfigManager.cpp` の実装の一部を新規ファイルに物理分割する。今回はタスク3のみを実行する。

*   **1. `ConfigManager_Playback.cpp` の作成と移行**
    *   新規ファイル `src/ConfigManager_Playback.cpp` を作成する。
    *   ファイルの先頭に `#include "ConfigManager.h"` と、その実装に必要なインクルード文を記述する。
    *   `ConfigManager.cpp` から、以下のUI要素に関するメソッド（`.cpp`に実装が存在するもの）を切り取り、`ConfigManager_Playback.cpp` へ完全に移行（ペースト）する。
        *   **シークバー (SeekBar)** 関連の設定
        *   **再生コントロール (PlaybackControls)** 関連の設定 (10秒スキップ等の設定も含む)
        *   **音量コントロール (VolumeControl)** 関連の設定 (デフォルト音量、ツールチップ等の設定も含む)
    *   ※ 注意: プレイリストの時と同様、ヘッダファイル（`.h`）内でインライン定義されているだけの Getter 等は移行の対象外である。もし該当セクションの実装がすべてインラインであり、`.cpp`側に移行すべきものが一つも無い場合は、その旨をレポートに記載して空のファイル（またはインクルードのみ）とするか、報告のみでタスク完了としてよい。
    *   ※ 注意: `CMakeLists.txt` への追加はタスク7で行うため、本タスクでのビルド検証は保留とする。

###### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: コピー＆ペーストによる安全な移行を心がけ、既存の動作を絶対に壊さないこと。


--------------------------------------------------------------------------------


##### 作業指示書 REQ: Phase 20-4 Task 4: ConfigManager_LogoMenu.cpp の作成 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリング（ファイル分割）の「実装実行」である。直ちに以下の【実装要件】に従ってコードの分割・移行を実行すること。
2. 作業完了後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260712_1524_RES_Phase20-4_ConfigManagerFileSplit.md）の「タスク4」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク4（ConfigManager_LogoMenu.cpp の作成）の実装が完了しました。」と報告すること。

---

###### 【実装要件】
`ConfigManager.h` のクラス定義や既存の機能・ロジックは一切変更せず、`ConfigManager.cpp` の実装の一部を新規ファイルに物理分割する。今回はタスク4のみを実行する。

*   **1. `ConfigManager_LogoMenu.cpp` の作成と移行**
    *   新規ファイル `src/ConfigManager_LogoMenu.cpp` を作成する。
    *   ファイルの先頭に `#include "ConfigManager.h"` と、その実装に必要なインクルード文（`<string>`等）を記述する。
    *   `ConfigManager.cpp` から、以下のUI要素に関するメソッド（`.cpp`に実装が存在するもの）を切り取り、`ConfigManager_LogoMenu.cpp` へ完全に移行（ペースト）する。
        *   **アプリアイコン (AppLogo)** 関連の設定
        *   **ロゴ拡張メニュー (LogoMenu)** 関連の設定
    *   ※ 注意: 以前のタスクと同様、ヘッダファイル（`.h`）内でインライン定義されているだけの Getter 等は移行の対象外である。もし該当セクションの実装がすべてインラインであり、`.cpp`側に移行すべきものが一つも無い場合は、その旨をレポートに記載して `#include "ConfigManager.h"` のみ記述したファイルとするか、報告のみでタスク完了としてよい。
    *   ※ 注意: `CMakeLists.txt` への追加はタスク7で行うため、本タスクでのビルド検証は保留とする。

###### 【絶対遵守ルール (Constraints)】
*   **クラス設計の維持**: 本タスクは「物理ファイルの分割」のみである。新たなクラスの作成や論理構造の変更は絶対に行わないこと。
*   **機能変更の禁止**: コピー＆ペーストによる安全な移行を心がけ、既存の動作を絶対に壊さないこと。


--------------------------------------------------------------------------------
