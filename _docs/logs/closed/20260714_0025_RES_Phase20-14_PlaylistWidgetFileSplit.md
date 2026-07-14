# RES:実装計画・作業レポート Phase 20-14: PlaylistWidgetの物理ファイル分割

## 1. 実装目的
Phase 20-13までに内部のメソッド抽出と目次化が完了している `src/Widget_Playlist.cpp` について、AI-IDEのコンテキスト節約と可読性向上のため、クラスの定義（`Widget_Playlist.h`）は一切変更せず、実装のみを責務別の `.cpp` ファイルに物理分割する。

## 2. アーキテクチャ設計
### 要件1: 実装ファイルの分割
    - 現在の `Widget_Playlist.cpp` を以下の責務ごとに新規ファイルへ分割・移行する。
        - `src/Widget_Playlist_Resources.cpp` : `CreateResources`, `ReleaseResources` の移行。
        - `src/Widget_Playlist_Update.cpp` : `UpdateAnimation`, `UpdateLayout` の移行。
        - `src/Widget_Playlist_Toolbar.cpp` : `DrawToolbar`, `DrawPinButton`, `BuildToolbarText` 等の移行。
        - `src/Widget_Playlist_DrawItems.cpp` : `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList` とローカルラムダ（`GetBlendedTextColor`等）の移行。
        - `src/Widget_Playlist_Draw.cpp` : `Draw`, `DrawBackground`, `DrawGrip` などの大元およびベース描画の移行。
        - `src/Widget_Playlist.cpp` (大元) : コンストラクタ、デストラクタ、GetScrollY 等のシンプルなセッター/ゲッターのみを残す。
    - **機能変更の禁止**: コードのロジック変更、リファクタリング、クラス設計（.h）の変更は一切行わない。

### 要件2: 依存関係の整理と CMakeLists の更新
    - 新規作成する各 `.cpp` ファイルの先頭に必要なインクルード（`#include "Widget_Playlist.h"` 等）を記述する。
    - `CMakeLists.txt` のソースファイルリストに新設するファイル群を追加し、ビルドを通す。

## 3. 実装タスクリスト
[x] タスク1: リソース管理ファイルの分離 (`Widget_Playlist_Resources.cpp`)
    - `Widget_Playlist.cpp` から `CreateResources`, `ReleaseResources` を抽出し `Widget_Playlist_Resources.cpp` を作成する。
[x] タスク2: 更新処理ファイルの分離 (`Widget_Playlist_Update.cpp`)
    - `Widget_Playlist.cpp` から `UpdateAnimation`, `UpdateLayout` を抽出し `Widget_Playlist_Update.cpp` を作成する。
[x] タスク3: ツールバー描画ファイルの分離 (`Widget_Playlist_Toolbar.cpp`)
    - `Widget_Playlist.cpp` から `DrawToolbar`, `DrawPinButton`, `BuildToolbarText` 等を抽出し `Widget_Playlist_Toolbar.cpp` を作成する。
[x] タスク4: アイテムリスト描画ファイルの分離 (`Widget_Playlist_DrawItems.cpp`)
    - `Widget_Playlist.cpp` から `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList` および関連するローカル関数を抽出し `Widget_Playlist_DrawItems.cpp` を作成する。
[x] タスク5: ベース描画ファイルの分離 (`Widget_Playlist_Draw.cpp`)
    - `Widget_Playlist.cpp` から `Draw`, `DrawBackground`, `DrawGrip` 等を抽出し `Widget_Playlist_Draw.cpp` を作成する。
[x] タスク6: 大元ファイルの整理 (`Widget_Playlist.cpp`)
    - 分離したコードを `Widget_Playlist.cpp` から削除し、コンストラクタ、デストラクタ、セッター/ゲッターのみを残す。
[x] タスク7: `CMakeLists.txt` の更新
    - 追加したすべての `.cpp` ファイルを `CMakeLists.txt` に追記する。
[x] タスク8: アーキテクチャ資料の更新
    - `PROJECT_ARCHITECTURE.md` 内の `Widget_Playlist` に関する説明を更新し、物理ファイルが完全に分割された事実を追記する。

## 4. 詳細作業内容
### タスク1: リソース管理ファイルの分離
    - `src/Widget_Playlist.cpp` から `CreateResources` および `ReleaseResources` メソッドの実装を抽出した。
    - 抽出したメソッドを `src/Widget_Playlist_Resources.cpp` に移行し、新規ファイルとして作成した。
    - 新規ファイルには `#include "Widget_Playlist.h"` および `#include "ConfigManager.h"` を追加し、依存関係を解決した。

### タスク2: 更新処理ファイルの分離
    - `src/Widget_Playlist.cpp` から `UpdateAnimation` および `UpdateLayout` メソッドの実装を抽出した。
    - 抽出したメソッドを `src/Widget_Playlist_Update.cpp` に移行し、新規ファイルとして作成した。
    - 新規ファイルには `#include "Widget_Playlist.h"` などの必要なインクルード文を追加し、依存関係を整理した。

### タスク3: ツールバー描画ファイルの分離
    - `src/Widget_Playlist.cpp` から `DrawToolbar`, `DrawPinButton`, `BuildToolbarText` メソッドの実装を抽出した。
    - 抽出したメソッドを `src/Widget_Playlist_Toolbar.cpp` に移行し、新規ファイルとして作成した。
    - 新規ファイルには `#include "Widget_Playlist.h"`, `#include "ConfigManager.h"`, `#include <filesystem>` 等の必要なインクルード文を追加し、コンパイル可能な状態とした。

### タスク4: アイテムリスト描画ファイルの分離
    - `src/Widget_Playlist.cpp` から `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList` メソッドの実装を抽出した。
    - 抽出したメソッドを `src/Widget_Playlist_DrawItems.cpp` に移行し、新規ファイルとして作成した。
    - 新規ファイルには `#include "Widget_Playlist.h"`, `#include "ConfigManager.h"`, `#include "LayoutCalculator.h"`, `#include <filesystem>` の必要なインクルード文を追加し、依存関係を整理した。

### タスク5: ベース描画ファイルの分離
    - `src/Widget_Playlist.cpp` から `Draw`, `DrawBackground`, `DrawGrip` メソッドの実装を抽出し、新設した `src/Widget_Playlist_Draw.cpp` へ移行した。
    - 移行先のファイルには `#include "Widget_Playlist.h"`, `#include "ConfigManager.h"`, `#include "LayoutCalculator.h"` を追加し、依存関係を整理した。
### タスク6: 大元ファイルの整理
    - `src/Widget_Playlist.cpp` 内に最後まで残っていた `Draw`, `DrawGrip`, `DrawBackground` の実装を完全に削除した。
    - コンストラクタ、デストラクタ（ヘッダ定義）、`AddScroll`、`GetScrollY`、`ForceClearHoverDelay` などの最小限の処理のみを残した。
    - `ConfigManager.h`, `LayoutCalculator.h`, `<filesystem>` など、不要になった `#include` を削除し、依存関係をクリーンアップした。

### タスク7: CMakeLists.txt の更新
    - `CMakeLists.txt` のソースファイル一覧（`SOURCES`）に、分割作成した `src/Widget_Playlist_Resources.cpp`, `src/Widget_Playlist_Update.cpp`, `src/Widget_Playlist_Toolbar.cpp`, `src/Widget_Playlist_DrawItems.cpp`, `src/Widget_Playlist_Draw.cpp` の5つのファイルを追記し、ビルド対象として登録した。

### タスク8: アーキテクチャ資料の更新
    - `PROJECT_ARCHITECTURE.md` の「各具象 Widget 実装」セクションにおける `Widget_Playlist` の説明を更新した。
    - AI-IDEでのコンテキスト節約と可読性向上のため、`Widget_Playlist.cpp` が責務別（リソース管理、状態更新、ツールバー、アイテム描画、ベース描画、本体）に6つの物理ファイルとして完全に分割された事実を追記した。
