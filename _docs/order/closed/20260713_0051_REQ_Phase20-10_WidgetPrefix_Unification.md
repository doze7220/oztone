### 作業指示書 REQ: Phase 20-10: 全Widgetのプレフィックス命名統一 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-10_WidgetPrefix_Unification.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
**[背景と大目的]**
Phase 20-9 までに `Widgets.cpp` の解体と `Widget_` プレフィックスの導入が行われたが、過去のフェーズ（Phase 17 等）で先行して独立していた中型Widget群は旧来のファイル名（例: `PlaylistWidget.cpp`）のままとなっている。
本フェーズでは、これらのファイル名を `Widget_*.h/cpp` の命名規則にリネームし、プロジェクト内のすべてのWidgetファイルの一覧性と統一性を極限まで高める。

*   **要件1: 先行独立Widget群のファイルリネーム**
    *   以下のファイルをプレフィックス付きの名称に物理的にリネームする計画を立てること。
        *   `src/LogoMenuWidget.h / .cpp` → `src/Widget_LogoMenu.h / .cpp`
        *   `src/PlaylistWidget.h / .cpp` → `src/Widget_Playlist.h / .cpp`
        *   `src/PlaybackControlsWidget.h / .cpp` → `src/Widget_PlaybackControls.h / .cpp`
        *   `src/VolumeControlWidget.h / .cpp` → `src/Widget_VolumeControl.h / .cpp`
    *   ※C++のクラス名（`PlaylistWidget` 等）は変更せず、ファイル名のみを変更すること。
*   **要件2: 依存関係の解決**
    *   呼び出し元（`src/Renderer.cpp`, `src/Application_*.cpp` 等）における旧ファイル名の `#include` パスを新ファイル名へ書き換える手順を整理すること。
*   **要件3: ビルド設定の更新**
    *   `CMakeLists.txt` のソースリストおよびヘッダリストを新ファイル名へと更新する手順を整理すること。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-10_WidgetPrefix_Unification.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、すべてのWidgetが `Widget_*.h/cpp` というファイル構成に統一された旨を反映する（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋なファイルのリネームと依存関係の修正であり、アプリケーションの挙動やUIには一切変更を加えないこと。
*   **計画のみの実行**: 重ねて指示するが、本プロンプトにおいてソースコードおよびCMakeLists.txtの直接修正は絶対に行ってはならない。

--------------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-10 Task 1-3 : 先行独立Widget群のプレフィックス命名統一
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260713_0051_RES_Phase20-10_WidgetPrefix_Unification.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは先行独立Widget群のリネームと依存関係修正の実装実行である。直ちに以下の【実装要件】に従ってファイルの物理リネームおよびコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260713_0051_RES_Phase20-10_WidgetPrefix_Unification.md）の「タスク1」「タスク2」「タスク3」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「全Widgetのプレフィックス命名統一(Phase 20-10 Task 1-3)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
以下の4つの先行独立Widgetのファイル名をリネームし、関連する依存関係とビルド設定を一括で更新してビルドを通す。
*   **要件1: ファイルのリネーム (タスク1)**
    *   `src/LogoMenuWidget.h / .cpp` → `src/Widget_LogoMenu.h / .cpp`
    *   `src/PlaylistWidget.h / .cpp` → `src/Widget_Playlist.h / .cpp`
    *   `src/PlaybackControlsWidget.h / .cpp` → `src/Widget_PlaybackControls.h / .cpp`
    *   `src/VolumeControlWidget.h / .cpp` → `src/Widget_VolumeControl.h / .cpp`
    *   ※C++のクラス名（`PlaylistWidget` 等）や内部ロジックは一切変更しないこと。
*   **要件2: 依存関係の解決 (タスク2)**
    *   呼び出し元（`src/Renderer.cpp`, `src/Application_*.cpp` 等）における旧ファイル名の `#include` パスを新ファイル名へと一斉に書き換える。
*   **要件3: ビルド設定の更新 (タスク3)**
    *   `CMakeLists.txt` のソースリストおよびヘッダリストを新ファイル名へと更新し、正常にビルドが通る状態にする。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋な物理ファイル名のリネームおよび依存関係の修正のみであり、アプリケーションの挙動やUIには一切変更を加えないこと。

--------------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-10 Task 4 : アーキテクチャ資料の更新
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260713_0051_RES_Phase20-10_WidgetPrefix_Unification.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはアーキテクチャ資料の更新（ドキュメント修正）実行である。直ちに以下の【更新要件】に従ってドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260713_0051_RES_Phase20-10_WidgetPrefix_Unification.md）の「タスク4」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「アーキテクチャ資料の更新(Phase 20-10 Task 4)が完了しました。すべてのプレフィックス統一が完了しました！」と報告すること。

#### 【更新要件】
*   `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` 内の「Widget コンポーネント」に関する記述を確認する。
*   ファイル名が変更された以下の先行独立 Widget クラス群について、記載されているファイルパスを新しいプレフィックス命名規則に従って更新する。
    *   `LogoMenuWidget (src/LogoMenuWidget.h/cpp)` → `LogoMenuWidget (src/Widget_LogoMenu.h/cpp)`
    *   `PlaylistWidget (src/PlaylistWidget.h/cpp)` → `PlaylistWidget (src/Widget_Playlist.h/cpp)`
    *   `PlaybackControlsWidget`, `VolumeControlWidget` などについても、旧ファイル名での記載があれば `src/Widget_PlaybackControls.h/cpp`, `src/Widget_VolumeControl.h/cpp` へと修正（または追記）を行う。
*   クラス名自体（`LogoMenuWidget` 等）は変更せず、ファイルパスの表記のみを統一すること。

#### 【絶対遵守ルール (Constraints)】
*   **コード変更の禁止**: 本作業はドキュメント（.mdファイル）の更新のみである。C++のソースコード（.h / .cpp）やCMakeLists.txtには一切触れないこと。
