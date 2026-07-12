### 作業指示書 REQ: Phase 20-9: Widgets.cpp の完全解体とプレフィックス命名分割 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-9_Widgets_Destruction.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
**[背景と大目的]**
AI-IDEのコンテキスト節約および保守性の極限向上を目指し、現在複数の小型Widgetが同居している `src/Widgets.h` および `src/Widgets.cpp` を完全に解体する。
その際、ファイル名の一覧性を高めるため、すべてのファイル名に `Widget_` というプレフィックスを付与する。

*   **要件1: 各Widgetの個別ファイル化 (プレフィックス命名)**
    *   現在 `Widgets.h / .cpp` 内に残存している全てのWidgetクラスを、以下の命名規則に従ってそれぞれ個別のヘッダおよび実装ファイルへと完全に物理分割する計画を立てること。
        *   `AppLogoWidget` → `src/Widget_AppLogo.h / .cpp`
        *   `TrackInfoWidget` → `src/Widget_TrackInfo.h / .cpp`
        *   `NextTrackWidget` → `src/Widget_NextTrack.h / .cpp`
        *   `SeekBarWidget` → `src/Widget_SeekBar.h / .cpp`
        *   `GlobalHotkeysWidget` → `src/Widget_GlobalHotkeys.h / .cpp`
        *   `ResizeGripWidget` → `src/Widget_ResizeGrip.h / .cpp`
        *   `OsdWidget` → `src/Widget_Osd.h / .cpp`
    *   ※ファイル名は変わるが、C++のクラス名（`AppLogoWidget` 等）は変更しない。
*   **要件2: 依存関係の再構築**
    *   新しく作成する各ファイルにおいて、必要なヘッダ（`WidgetCommon.h` や `WidgetContext.h` など）をどのようにインクルードするかの整理。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスをどのように書き換えるかの整理。
*   **要件3: ビルド設定の更新と旧ファイルの削除**
    *   中身が空になった `src/Widgets.h / .cpp` を物理削除し、`CMakeLists.txt` のソースリストを更新する手順を確立すること。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-9_Widgets_Destruction.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋なファイルの物理分割であり、各Widgetの描画ロジックや計算結果等の挙動を一切変えないこと。
*   **計画のみの実行**: 重ねて指示するが、本プロンプトにおいてソースコードおよびCMakeLists.txtの直接修正は絶対に行ってはならない。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 1 : AppLogoWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは AppLogoWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「AppLogoWidget の分離(Phase 20-9 Task 1)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している `AppLogoWidget` を物理分割し、単独のファイルへと移行する。
*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_AppLogo.h` を新規作成し、`Widgets.h` から `AppLogoWidget` クラスの定義を完全に移行する。
    *   `src/Widget_AppLogo.cpp` を新規作成し、`Widgets.cpp` から `AppLogoWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `AppLogoWidget` に関する記述を削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_AppLogo.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_AppLogo.cpp` と `src/Widget_AppLogo.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目や挙動（ホバー判定等）を一切変えないこと。
*   **スコープの厳守**: 今回は `AppLogoWidget` のみを作業対象とし、他のWidgetの抽出・分割は絶対に同時に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 2 : TrackInfoWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは TrackInfoWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク2」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「TrackInfoWidget の分離(Phase 20-9 Task 2)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している `TrackInfoWidget` を物理分割し、単独のファイルへと移行する。

*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_TrackInfo.h` を新規作成し、`Widgets.h` から `TrackInfoWidget` クラスの定義を完全に移行する。
    *   `src/Widget_TrackInfo.cpp` を新規作成し、`Widgets.cpp` から `TrackInfoWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `TrackInfoWidget` に関する記述を削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_TrackInfo.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_TrackInfo.cpp` と `src/Widget_TrackInfo.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目やテキストトリミング等の挙動を一切変えないこと。
*   **スコープの厳守**: 今回は `TrackInfoWidget` のみを作業対象とし、他のWidgetの抽出・分割は絶対に同時に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 3 : NextTrackWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは NextTrackWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク3」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「NextTrackWidget の分離(Phase 20-9 Task 3)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している `NextTrackWidget` を物理分割し、単独のファイルへと移行する。

*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_NextTrack.h` を新規作成し、`Widgets.h` から `NextTrackWidget` クラスの定義を完全に移行する。
    *   `src/Widget_NextTrack.cpp` を新規作成し、`Widgets.cpp` から `NextTrackWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `NextTrackWidget` に関する記述を削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_NextTrack.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_NextTrack.cpp` と `src/Widget_NextTrack.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目や挙動を一切変えないこと。
*   **スコープの厳守**: 今回は `NextTrackWidget` のみを作業対象とし、他のWidgetの抽出・分割は絶対に同時に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 4 : SeekBarWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは SeekBarWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク4」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「SeekBarWidget の分離(Phase 20-9 Task 4)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している `SeekBarWidget` を物理分割し、単独のファイルへと移行する。

*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_SeekBar.h` を新規作成し、`Widgets.h` から `SeekBarWidget` クラスの定義を完全に移行する。
    *   `src/Widget_SeekBar.cpp` を新規作成し、`Widgets.cpp` から `SeekBarWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `SeekBarWidget` に関する記述を削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_SeekBar.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_SeekBar.cpp` と `src/Widget_SeekBar.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目や挙動（ホバー時の減衰処理等）を一切変えないこと。
*   **スコープの厳守**: 今回は `SeekBarWidget` のみを作業対象とし、他のWidgetの抽出・分割は絶対に同時に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 5 : GlobalHotkeysWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは GlobalHotkeysWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク5」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「GlobalHotkeysWidget の分離(Phase 20-9 Task 5)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している `GlobalHotkeysWidget` を物理分割し、単独のファイルへと移行する。

*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_GlobalHotkeys.h` を新規作成し、`Widgets.h` から `GlobalHotkeysWidget` クラスの定義を完全に移行する。
    *   `src/Widget_GlobalHotkeys.cpp` を新規作成し、`Widgets.cpp` から `GlobalHotkeysWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `GlobalHotkeysWidget` に関する記述を削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_GlobalHotkeys.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_GlobalHotkeys.cpp` と `src/Widget_GlobalHotkeys.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目や挙動を一切変えないこと。
*   **スコープの厳守**: 今回は `GlobalHotkeysWidget` のみを作業対象とし、他のWidgetの抽出・分割は絶対に同時に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 6 : ResizeGripWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは ResizeGripWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク6」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「ResizeGripWidget の分離(Phase 20-9 Task 6)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している `ResizeGripWidget` を物理分割し、単独のファイルへと移行する。

*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_ResizeGrip.h` を新規作成し、`Widgets.h` から `ResizeGripWidget` クラスの定義を完全に移行する。
    *   `src/Widget_ResizeGrip.cpp` を新規作成し、`Widgets.cpp` から `ResizeGripWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `ResizeGripWidget` に関する記述を削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_ResizeGrip.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_ResizeGrip.cpp` と `src/Widget_ResizeGrip.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目や挙動を一切変えないこと。
*   **スコープの厳守**: 今回は `ResizeGripWidget` のみを作業対象とし、他のWidgetの抽出・分割は絶対に同時に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-9 Task 7 : OsdWidget の分離
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_2333_RES_Phase20-9_Widgets_Destruction.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは OsdWidget の分離実装実行 である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260712_2333_RES_Phase20-9_Widgets_Destruction.md）の「タスク7」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「OsdWidget の分離(Phase 20-9 Task 7)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widgets.h` / `.cpp` に同居している最後のWidgetである `OsdWidget` を物理分割し、単独のファイルへと移行する。

*   **要件1: ファイルの新規作成とコード移行**
    *   `src/Widget_Osd.h` を新規作成し、`Widgets.h` から `OsdWidget` クラスの定義を完全に移行する。
    *   `src/Widget_Osd.cpp` を新規作成し、`Widgets.cpp` から `OsdWidget` の実装を完全に移行する。
    *   移行後、元の `Widgets.h` および `Widgets.cpp` から `OsdWidget` に関する記述を完全に削除する。
*   **要件2: 依存関係の解決とビルド設定**
    *   新規作成したファイルにおいて、必要なヘッダ（`WidgetCommon.h`, `WidgetContext.h` など）をインクルードする。
    *   呼び出し元（`Renderer.cpp` 等）のインクルードパスに `#include "Widget_Osd.h"` を追加する。
    *   `CMakeLists.txt` に `src/Widget_Osd.cpp` と `src/Widget_Osd.h` を追加し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理分割（コードの移動）であり、UIの見た目や挙動（フライテキストのフェード等）を一切変えないこと。
*   **スコープの厳守**: 今回は `OsdWidget` のみを作業対象とする。旧 `Widgets.h / .cpp` の物理削除は次のタスクで行うため、中身が空になってもファイル自体は残しておくこと。

-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------

