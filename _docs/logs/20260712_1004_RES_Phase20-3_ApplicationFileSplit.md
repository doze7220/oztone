# RES:実装計画・作業レポート Phase 20-3: Application.cpp のファイル分割

## 1. 実装目的
肥大化した `Application.cpp` について、クラスの定義（`Application.h`）や依存関係は一切変更せず、AI-IDEのコンテキスト節約および可読性向上のため、実装のみを責務別の `.cpp` ファイルに物理分割する。

## 2. アーキテクチャ設計
### 要件1: 実装ファイルの分割
    - 既存の `Application.cpp` の実装を、以下の6ファイルに分割する。
      1. `Application.cpp`: コンストラクタ、デストラクタなど全体管理やエントリポイントに関わる最小限の処理。
      2. `Application_Initialize.cpp`: `Initialize`, `SetupCallbacks` など初期化処理に関する処理。
      3. `Application_Playback.cpp`: `PlayCurrentTrack`, `HandleMediaCommand`, `PrefetchNextTrack`, `UpdateTrackMetadataIfNeeded` など再生・メディア操作に関する処理。
      4. `Application_Playlist.cpp`: `SwitchPlaylist`, `ClearPlaylist`, `CreateNewPlaylist`, `OnPlaylistClicked`, `OnPlaylistDoubleClicked`, `OnPlaylistToolbarClicked` などプレイリスト制御に関する処理。
      5. `Application_FileDrop.cpp`: `OnFilesDropped`, `ProcessCommandLineArgs` など外部からのファイル入力に関する処理。
      6. `Application_Render.cpp`: `Run`, `ForceRender`, `UpdatePlaylistSummaries` などメインループおよび描画連携に関する処理。

### 要件2: CMakeLists.txt の更新
    - 新設した `Application_*.cpp` ファイル群をビルド対象として `CMakeLists.txt` に追加する。

## 3. 実装タスクリスト
[x] タスク1: `Application_Initialize.cpp` の作成
    - `Application.cpp` から `Initialize`, `SetupCallbacks` を抽出し、`Application_Initialize.cpp` へ移行する。
[x] タスク2: `Application_Playback.cpp` の作成
    - `Application.cpp` から `PlayCurrentTrack`, `HandleMediaCommand`, `PrefetchNextTrack`, `UpdateTrackMetadataIfNeeded` を抽出し、`Application_Playback.cpp` へ移行する。
[x] タスク3: `Application_Playlist.cpp` の作成
    - `Application.cpp` から `SwitchPlaylist`, `ClearPlaylist`, `CreateNewPlaylist`, `OnPlaylistClicked`, `OnPlaylistDoubleClicked`, `OnPlaylistToolbarClicked` を抽出し、`Application_Playlist.cpp` へ移行する。
[ ] タスク4: `Application_FileDrop.cpp` の作成
    - `Application.cpp` から `OnFilesDropped`, `ProcessCommandLineArgs` を抽出し、`Application_FileDrop.cpp` へ移行する。
[ ] タスク5: `Application_Render.cpp` の作成
    - `Application.cpp` から `Run`, `ForceRender`, `UpdatePlaylistSummaries` を抽出し、`Application_Render.cpp` へ移行する。
[ ] タスク6: `Application.cpp` の整理
    - コンストラクタ、デストラクタなど全体管理やエントリポイントに関する処理のみを残す。
[x] タスク7: `CMakeLists.txt` の更新
    - 新設した `Application_*.cpp` (Initialize, Playback, Playlist, FileDrop, Render) をソースリストに追加し、ビルドが通るように構成する。
[ ] タスク8: レポートの更新およびドキュメント確認
    - 本ファイル（RES）の各タスクを完了状態にし、詳細作業内容を追記する。
    - `PROJECT_ARCHITECTURE.md` を確認し、ファイル分割に関する追記が必要であれば修正を行う。

## 4. 詳細作業内容
### タスク1: Application_Initialize.cpp の作成
    - `src/Application_Initialize.cpp` を作成し、`Application.cpp` から `Initialize` と `SetupCallbacks` の実装を移行した。
    - `CMakeLists.txt` の `SOURCES` に `src/Application_Initialize.cpp` を追加した。
### タスク2: Application_Playback.cpp の作成
    - `src/Application_Playback.cpp` を作成し、`PlayCurrentTrack`, `HandleMediaCommand`, `PrefetchNextTrack`, `UpdateTrackMetadataIfNeeded` の実装を移行した。
### タスク3: Application_Playlist.cpp の作成
    - `src/Application_Playlist.cpp` を作成し、`SwitchPlaylist`, `ClearPlaylist`, `CreateNewPlaylist`, `OnPlaylistClicked`, `OnPlaylistDoubleClicked`, `OnPlaylistToolbarClicked` の実装を移行した。
### タスク4: Application_FileDrop.cpp の作成
    - (未実施)
### タスク5: Application_Render.cpp の作成
    - (未実施)
### タスク6: Application.cpp の整理
    - (未実施)
### タスク7: CMakeLists.txt の更新
    - `CMakeLists.txt` の `SOURCES` に `src/Application_Playback.cpp` と `src/Application_Playlist.cpp` を追加した。
### タスク8: レポートの更新およびドキュメント確認
    - (未実施)

## 5. HOTFIX
### 原因・理由:
    - (なし)

### 対応:
    - (なし)
