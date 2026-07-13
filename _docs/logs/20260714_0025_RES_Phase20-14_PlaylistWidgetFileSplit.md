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
[ ] タスク2: 更新処理ファイルの分離 (`Widget_Playlist_Update.cpp`)
    - `Widget_Playlist.cpp` から `UpdateAnimation`, `UpdateLayout` を抽出し `Widget_Playlist_Update.cpp` を作成する。
[ ] タスク3: ツールバー描画ファイルの分離 (`Widget_Playlist_Toolbar.cpp`)
    - `Widget_Playlist.cpp` から `DrawToolbar`, `DrawPinButton`, `BuildToolbarText` 等を抽出し `Widget_Playlist_Toolbar.cpp` を作成する。
[ ] タスク4: アイテムリスト描画ファイルの分離 (`Widget_Playlist_DrawItems.cpp`)
    - `Widget_Playlist.cpp` から `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList` および関連するローカル関数を抽出し `Widget_Playlist_DrawItems.cpp` を作成する。
[ ] タスク5: ベース描画ファイルの分離 (`Widget_Playlist_Draw.cpp`)
    - `Widget_Playlist.cpp` から `Draw`, `DrawBackground`, `DrawGrip` 等を抽出し `Widget_Playlist_Draw.cpp` を作成する。
[ ] タスク6: 大元ファイルの整理 (`Widget_Playlist.cpp`)
    - 分離したコードを `Widget_Playlist.cpp` から削除し、コンストラクタ、デストラクタ、セッター/ゲッターのみを残す。
[ ] タスク7: `CMakeLists.txt` の更新
    - 追加したすべての `.cpp` ファイルを `CMakeLists.txt` に追記する。

## 4. 詳細作業内容
### タスク1: リソース管理ファイルの分離
    - `src/Widget_Playlist.cpp` から `CreateResources` および `ReleaseResources` メソッドの実装を抽出した。
    - 抽出したメソッドを `src/Widget_Playlist_Resources.cpp` に移行し、新規ファイルとして作成した。
    - 新規ファイルには `#include "Widget_Playlist.h"` および `#include "ConfigManager.h"` を追加し、依存関係を解決した。

### タスク2: 更新処理ファイルの分離
    - (作業実行時に追記)

### タスク3: ツールバー描画ファイルの分離
    - (作業実行時に追記)

### タスク4: アイテムリスト描画ファイルの分離
    - (作業実行時に追記)

### タスク5: ベース描画ファイルの分離
    - (作業実行時に追記)

### タスク6: 大元ファイルの整理
    - (作業実行時に追記)

### タスク7: CMakeLists.txt の更新
    - (作業実行時に追記)
