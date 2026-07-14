### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-14_PlaylistWidgetFileSplit.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
Phase 20-13までに内部のメソッド抽出と目次化が完了している `src/Widget_Playlist.cpp` について、AI-IDEのコンテキスト節約と可読性向上のため、クラスの定義（`Widget_Playlist.h`）は一切変更せず、実装のみを責務別の `.cpp` ファイルに物理分割する。

*   **要件1: 実装ファイルの分割**
    現在の `Widget_Playlist.cpp` から以下の責務ごとにコードを抽出し、新規ファイルへ移行する計画を立てること。
    *   `src/Widget_Playlist_Resources.cpp` : `CreateResources`, `ReleaseResources` の移行。
    *   `src/Widget_Playlist_Update.cpp` : `UpdateAnimation`, `UpdateLayout` の移行。
    *   `src/Widget_Playlist_Toolbar.cpp` : `DrawToolbar`, `DrawPinButton`, `BuildToolbarText` 等の移行。
    *   `src/Widget_Playlist_DrawItems.cpp` : `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList` とローカルラムダ（`GetBlendedTextColor`等）の移行。
    *   `src/Widget_Playlist_Draw.cpp` : `Draw`, `DrawBackground`, `DrawGrip` などの大元およびベース描画の移行。
    *   `src/Widget_Playlist.cpp` (大元) : コンストラクタ、デストラクタ、GetScrollY 等のシンプルなセッター/ゲッターのみを残す。
*   **要件2: 依存関係の整理と CMakeLists の更新**
    *   新規作成する各 `.cpp` ファイルには、必要なインクルード（`Widget_Playlist.h` 等）を記述すること。
    *   `CMakeLists.txt` に新設するファイル群を追加する計画を含めること。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割のみである。コードのロジック変更、リファクタリング、クラス設計（.h）の変更は絶対に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (タスク1 実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-14の「タスク1の実装実行」である。他のタスクには絶対に触れず、以下の【実装要件】のみを実行すること。
2. 作業完了後、既存の作業レポート（20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md）の「タスク1」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク1が完了し、レポートを更新しました。タスク2の指示をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: リソース管理ファイルの分離 (Widget_Playlist_Resources.cpp)**
    *   `src/Widget_Playlist_Resources.cpp` を新規作成する。
    *   `src/Widget_Playlist.cpp` から `CreateResources` および `ReleaseResources` の実装を完全に抽出し、新設した `Widget_Playlist_Resources.cpp` へ移行する。
    *   移行先のファイルには、`#include "Widget_Playlist.h"` などの必要なインクルード文を記述し、コンパイルエラーが出ないように依存関係を整理すること。
    *   **※注意**: `Widget_Playlist.h` のクラス定義（ヘッダファイル）は一切変更しないこと。

#### 【絶対遵守ルール (Constraints)】
*   **1タスクの厳守**: 今回はタスク1のみを行うこと。他の更新処理や描画処理の分離は後続の指示で行うため、今は絶対に手を触れないこと。
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割のみである。コードのロジック変更、リファクタリングは絶対に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (タスク2 実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-14の「タスク2の実装実行」である。他のタスクには絶対に触れず、以下の【実装要件】のみを実行すること。
2. 作業完了後、既存の作業レポート（20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md）の「タスク2」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク2が完了し、レポートを更新しました。タスク3の指示をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク2: 更新処理ファイルの分離 (Widget_Playlist_Update.cpp)**
    *   `src/Widget_Playlist_Update.cpp` を新規作成する。
    *   `src/Widget_Playlist.cpp` から `UpdateAnimation` および `UpdateLayout` メソッドの実装を完全に抽出し、新設した `Widget_Playlist_Update.cpp` へ移行する。
    *   移行先のファイルには、`#include "Widget_Playlist.h"` 等の必要なインクルード文を記述し、コンパイルエラーが出ないように依存関係を整理すること。
    *   **※注意**: `Widget_Playlist.h` のクラス定義（ヘッダファイル）は一切変更しないこと。

#### 【絶対遵守ルール (Constraints)】
*   **1タスクの厳守**: 今回はタスク2のみを行うこと。他の描画処理の分離は後続の指示で行うため、今は絶対に手を触れないこと。
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割のみである。コードのロジック変更、リファクタリングは絶対に行わないこと。
-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (タスク3 実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-14の「タスク3の実装実行」である。他のタスクには絶対に触れず、以下の【実装要件】のみを実行すること。
2. 作業完了後、既存の作業レポート（20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md）の「タスク3」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク3が完了し、レポートを更新しました。タスク4の指示をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク3: ツールバー描画ファイルの分離 (Widget_Playlist_Toolbar.cpp)**
    *   `src/Widget_Playlist_Toolbar.cpp` を新規作成する。
    *   `src/Widget_Playlist.cpp` から `DrawToolbar`, `DrawPinButton`, `BuildToolbarText` メソッドの実装を完全に抽出し、新設した `Widget_Playlist_Toolbar.cpp` へ移行する。
    *   移行先のファイルには、`#include "Widget_Playlist.h"` 等の必要なインクルード文を記述し、コンパイルエラーが出ないように依存関係を整理すること。
    *   **※注意**: `Widget_Playlist.h` のクラス定義（ヘッダファイル）は一切変更しないこと。

#### 【絶対遵守ルール (Constraints)】
*   **1タスクの厳守**: 今回はタスク3のみを行うこと。他のアイテムリスト描画等の分離は後続の指示で行うため、今は絶対に手を触れないこと。
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割のみである。コードのロジック変更、リファクタリングは絶対に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (タスク4 実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-14の「タスク4の実装実行」である。他のタスクには絶対に触れず、以下の【実装要件】のみを実行すること。
2. 作業完了後、既存の作業レポート（20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md）の「タスク4」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク4が完了し、レポートを更新しました。タスク5の指示をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク4: アイテムリスト描画ファイルの分離 (Widget_Playlist_DrawItems.cpp)**
    *   `src/Widget_Playlist_DrawItems.cpp` を新規作成する。
    *   `src/Widget_Playlist.cpp` から `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList` メソッドの実装、およびそれらが使用するローカルラムダ（`GetBlendedTextColor` 等）を完全に抽出し、新設した `Widget_Playlist_DrawItems.cpp` へ移行する。
    *   移行先のファイルには、`#include "Widget_Playlist.h"` や `<filesystem>` 等の必要なインクルード文を記述し、コンパイルエラーが出ないように依存関係を整理すること。
    *   **※注意**: `Widget_Playlist.h` のクラス定義（ヘッダファイル）は一切変更しないこと。

#### 【絶対遵守ルール (Constraints)】
*   **1タスクの厳守**: 今回はタスク4のみを行うこと。ベース描画等の分離は後続の指示で行うため、今は絶対に手を触れないこと。
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割のみである。コードのロジック変更、リファクタリングは絶対に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (タスク5 実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-14の「タスク5の実装実行」である。他のタスクには絶対に触れず、以下の【実装要件】のみを実行すること。
2. 作業完了後、既存の作業レポート（20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md）の「タスク5」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「タスク5が完了し、レポートを更新しました。タスク6の指示をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク5: ベース描画ファイルの分離 (Widget_Playlist_Draw.cpp)**
    *   `src/Widget_Playlist_Draw.cpp` を新規作成する。
    *   `src/Widget_Playlist.cpp` から `Draw`, `DrawBackground`, `DrawGrip` メソッドの実装を完全に抽出し、新設した `Widget_Playlist_Draw.cpp` へ移行する。
    *   移行先のファイルには、`#include "Widget_Playlist.h"`, `#include "LayoutCalculator.h"` 等の必要なインクルード文を記述し、コンパイルエラーが出ないように依存関係を整理すること。
    *   **※注意**: `Widget_Playlist.h` のクラス定義（ヘッダファイル）は一切変更しないこと。

#### 【絶対遵守ルール (Constraints)】
*   **1タスクの厳守**: 今回はタスク5のみを行うこと。大元ファイルの整理やCMakeListsの更新は後続のタスクで行うため、今は絶対に手を触れないこと。
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割のみである。コードのロジック変更、リファクタリングは絶対に行わないこと。

-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-14: PlaylistWidgetの物理ファイル分割 (タスク6・7 最終実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-14の「タスク6およびタスク7の実装実行」である。以下の【実装要件】のみを実行すること。
2. 作業完了後、既存の作業レポート（20260714_0025_RES_Phase20-14_PlaylistWidgetFileSplit.md）の「タスク6」「タスク7」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「PlaylistWidgetの物理ファイル分割(Phase 20-14)がすべて完了しました！ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク6: 大元ファイルの整理 (Widget_Playlist.cpp)**
    *   タスク1〜5で別ファイルへ分離済みのメソッドの実装を `src/Widget_Playlist.cpp` から完全に削除する。
    *   残すのは、コンストラクタ、デストラクタ、および `GetScrollY` 等のシンプルなセッター/ゲッター等の最小限の処理のみとする。
    *   不要になった `#include` をクリーンアップすること。
*   **タスク7: CMakeLists.txt の更新**
    *   新設した5つのファイル（`Widget_Playlist_Resources.cpp`, `Widget_Playlist_Update.cpp`, `Widget_Playlist_Toolbar.cpp`, `Widget_Playlist_DrawItems.cpp`, `Widget_Playlist_Draw.cpp`）を `CMakeLists.txt` のソースリストに追記し、ビルドターゲットとして正しく構成すること。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本タスクは純粋なファイルの物理分割の総仕上げとクリーンアップのみである。コードのロジック変更、リファクタリングは絶対に行わないこと。
*   `Widget_Playlist.h` のクラス定義（ヘッダファイル）は一切変更しないこと。

-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
