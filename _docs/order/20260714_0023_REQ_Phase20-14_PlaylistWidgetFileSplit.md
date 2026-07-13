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
