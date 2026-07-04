# Phase 11-1: 新規プレイリスト作成機能 実装計画レポート

## 1. 実装目的
本タスクでは、システムトレイメニューの拡張を行い、現在日時ベースの新しいプレイリストファイル（`playlist_YYYYMMDD_HHMM.lst`等）を自動生成し、空の状態で切り替える機能を実装する。これにより、ユーザーはプレイリストを安全かつ簡単に切り替えて管理できるようになる。

## 2. アーキテクチャ設計・実装方針

### 2.1 システムトレイメニューの拡張 (Windowクラス)
- `src/Window.h` に新しいメニューID `ID_TRAY_NEW_PLAYLIST`（定数値: `1024` など）を定義する。
- メニュー選択時のコールバックを保持するため、インターフェース `void SetNewPlaylistCallback(std::function<void()> cb)` と、そのメンバ変数 `m_onNewPlaylistCommand` を追加する。
- `src/Window.cpp` の `TRAY_MENU_ORDER` 配列にて、`ID_TRAY_CLEAR_PLAYLIST` の上（または直後）など、適切な位置に `ID_TRAY_NEW_PLAYLIST` を挿入する。
- コンテキストメニュー構築時、IDが `ID_TRAY_NEW_PLAYLIST` の場合に「新規プレイリスト作成 (New Playlist)」という文字列をメニューに追加する。
- `WM_TRAYICON` のメニュー選択処理にて、該当IDがクリックされた際に `m_onNewPlaylistCommand` を発火させる。

### 2.2 プレイリストパスの保存機構拡張 (ConfigManagerクラス)
- `src/ConfigManager.h` および `.cpp` にて、プレイリストパスを更新する `void SetDefaultPlaylistPath(const std::wstring& path)` を追加する。
- このメソッド内では、内部変数 `m_defaultPlaylistPath` を更新した直後、`WritePrivateProfileStringW` を呼び出して `OZtone.ini` へパスを書き込み永続化させる。

### 2.3 ファイル名の自動生成とプレイリスト切り替え (Applicationクラス)
- `src/Application.h` に `void CreateNewPlaylist()` を追加する。
- `Application::Initialize` にて、`m_window.SetNewPlaylistCallback([this]() { this->CreateNewPlaylist(); });` のようにコールバックを紐付ける。
- **`CreateNewPlaylist()` の実装ロジック**:
  1. `<chrono>` や `<iomanip>` を用い、現在のシステム日時から `playlist_YYYYMMDD_HHMM.lst` の基本ファイル名を生成する。
  2. `ConfigManager::GetDefaultPlaylistPath` から現在のパスを取得し、`std::filesystem::path::parent_path()` で保存先ディレクトリを特定する（親ディレクトリがない場合はカレントとなる）。
  3. `std::filesystem::exists` を用いて重複確認を行う。既に同名ファイルが存在する場合は、`playlist_YYYYMMDD_HHMM_1.lst`, `_2.lst` のように末尾にシーケンス番号を付与して安全なパスを確定させる。
  4. 確定したフルパスを `ConfigManager::SetDefaultPlaylistPath` に渡してINIへ設定を反映する。
  5. 既存の `ClearPlaylist()` を呼び出す。`ClearPlaylist()` の内部では新しい `GetDefaultPlaylistPath()` が参照され、自動的に**「メモリのクリア」「再生の停止」「"No Track"画面へのリセット」「新規パスへの空ファイル作成」**が一挙に実行されるため、非常にスマートかつ安全に移行が完了する。

### 2.4 懸念事項と対策
- **ディレクトリ階層の解決**: `GetDefaultPlaylistPath` の値がファイル名のみ（例: `oztone_playlist.lst`）の場合、`parent_path()` は空となり、カレントディレクトリに新しいファイルが作成される。これは現在の `oztone_playlist.lst` の挙動と一致するため問題ない。
- **スコープの厳守**: サブディレクトリの作成や、作成済みプレイリストの読み込み一覧化等は実装しない。本要件の「新規作成」のみに留める。

## 3. タスクリスト

- [x] **Task 1: Windowクラスの拡張**
  - `Window.h` に `ID_TRAY_NEW_PLAYLIST` と `SetNewPlaylistCallback` を追加。
  - `Window.cpp` でメニューに組み込み、コールバック処理を追加。
- [x] **Task 2: ConfigManagerクラスの拡張**
  - `ConfigManager.h` に `SetDefaultPlaylistPath` を追加。
  - `ConfigManager.cpp` で `WritePrivateProfileStringW` による保存処理を実装。
- [x] **Task 3: Applicationクラスのロジック実装**
  - `Application.h` に `CreateNewPlaylist()` を追加。
  - `Application.cpp` で日時からのファイル名生成、重複回避ロジック、パスの設定と `ClearPlaylist()` の呼び出しを実装。
  - `Application::Initialize` でウィンドウのコールバックへ登録。

## 4. 詳細作業内容
- `Window.h` および `Window.cpp` にシステムトレイメニュー拡張とコールバックを追加し、`ID_TRAY_NEW_PLAYLIST` (1024) を定義しました。
- `ConfigManager.h` および `ConfigManager.cpp` にプレイリストのデフォルトパスを動的に変更してINIファイルへ保存する `SetDefaultPlaylistPath` メソッドを実装しました。
- `Application.h` および `Application.cpp` に `CreateNewPlaylist()` を追加し、システム日時を元にした `playlist_YYYYMMDD_HHMM.ozl` 形式（拡張子を `.lst` から `.ozl` に変更）のファイル名を生成する処理を実装しました。1分以内に連続作成された場合の重複回避ロジック（`_1`, `_2` の付与）を実装し、カレントディレクトリと親ディレクトリの両方のケースに安全に対応させています。
- 最後に `ClearPlaylist()` を呼び出し、メモリ上のプレイリストの破棄とUIのリセット（"No Track"画面への移行）を自動で行うよう設定しました。これによって、INIファイルのパス更新と空ファイルの準備がアトミックに行われます。
