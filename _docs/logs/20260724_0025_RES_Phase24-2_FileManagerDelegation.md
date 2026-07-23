# RES:実装計画・作業レポート Phase 24-2: ConfigManagerのファイルI/O責務移管

## 1. 実装目的
ConfigManagerに残存しているファイルシステム操作のロジック（ディレクトリ走査およびスナップショット監視）を、本来の責務を持つFileManagerへ移管する。これにより、ConfigManagerを純粋な設定コンテナとして純化し、プロジェクトのアーキテクチャ（責務分離）をより強固なものにする。

## 2. アーキテクチャ設計
### 要件1: ConfigManagerからのファイルI/O処理の削除
    - `ConfigManager.h` および `ConfigManager.cpp` から `GetAvailablePlaylists` メソッドと `CheckPlaylistSnapshotChanged` メソッドの宣言・実装を削除する。
    - 状態として保持されていたメンバ変数 `m_playlistSnapshot` を削除する。

### 要件2: FileManagerへの処理移管
    - `FileManager.h` と `FileManager.cpp` に、プレイリストファイルの一覧取得とスナップショット監視のための静的メソッドを新規追加する。
    - 追加するメソッド：
      - `static std::vector<std::wstring> GetAvailablePlaylists(const std::wstring& defaultPlaylistPath);`
      - `static bool CheckPlaylistSnapshotChanged(const std::wstring& defaultPlaylistPath, std::vector<std::pair<std::wstring, std::filesystem::file_time_type>>& inOutSnapshot);`
    - これらのメソッドでは、引数として渡されたパス（およびフォールバックとしての実行ファイルパス等）を基にディレクトリを解決し、I/O処理を実行する。

### 要件3: Application/Window層の配線修正
    - スナップショットの状態変数 `m_playlistSnapshot` の保持責務を、利用側である `Application` クラス（`Application.h`）へ移動する。
    - `Application` 内（`Application_Initialize.cpp`, `Application_Playlist.cpp`, `Application_Render.cpp`）でのメソッド呼び出しを、`FileManager` 経由のものに置き換える。引数には `m_config.GetPlaylist().DefaultPlaylistPath` を渡す。
    - `Window_TrayMenu.cpp` でのメソッド呼び出しも同様に `FileManager::GetAvailablePlaylists` を呼ぶように修正し、`m_config->GetPlaylist().DefaultPlaylistPath` を渡す。

## 3. 実装タスクリスト
- [x] タスク1: FileManagerへのメソッド追加と実装
    - `FileManager.h` と `FileManager.cpp` に `GetAvailablePlaylists` および `CheckPlaylistSnapshotChanged` の実装を追加する。
- [x] タスク2: ConfigManagerからのメソッドとメンバ変数の削除
    - `ConfigManager.h` と `ConfigManager.cpp` から対象のメソッドおよび変数を完全に削除する。
- [x] タスク3: 状態変数の追加とメインループの結線修正
    - `Application.h` に `m_playlistSnapshot` を追加し、`Application_Render.cpp` のメインループ内のポーリング呼び出しを `FileManager` に書き換える。
- [ ] タスク4: プレイリスト操作・初期化の結線修正
    - `Application_Initialize.cpp` (初回ロード時) と `Application_Playlist.cpp` (切り替え時) の呼び出しを `FileManager` に書き換える。
- [ ] タスク5: Window層（TrayMenu）の結線修正
    - `Window_TrayMenu.cpp` の呼び出しを `FileManager` に書き換える。
- [ ] タスク6: アーキテクチャドキュメントの更新
    - 移管が完了した旨を `PROJECT_ARCHITECTURE.md` に追記・修正する。

## 4. 詳細作業内容
### タスク1: FileManagerへのメソッド追加と実装
    **【対象ファイル】**
    - `src/FileManager.h` : (更新)
    - `src/FileManager.cpp` : (更新)
    **【作業内容】**
    - 削除予定のロジックをベースに、引数で `defaultPlaylistPath` と（更新チェック用には）状態変数 `snapshot` への参照を受け取るようにシグネチャを整え、静的メソッドとして実装を追加する。
    - **[完了]** `GetAvailablePlaylists` と `CheckPlaylistSnapshotChanged` の静的メソッドを実装し、実行ファイルパス取得ロジック（`GetExecutablePath`）も内部ユーティリティとして実装した。

### タスク2: ConfigManagerからのメソッドとメンバ変数の削除
    **【対象ファイル】**
    - `src/Config/ConfigManager.h` : (更新)
    - `src/Config/ConfigManager.cpp` : (更新)
    **【作業内容】**
    - 移管対象となったメソッド（`GetAvailablePlaylists`, `CheckPlaylistSnapshotChanged`）および変数（`m_playlistSnapshot`）を削除する。
    - **[完了]** `ConfigManager.h` および `ConfigManager.cpp` から対象コードを完全に削除した。

### タスク3: 状態変数の追加とメインループの結線修正
    **【対象ファイル】**
    - `src/Application.h` : (更新)
    - `src/Application_Render.cpp` : (更新)
    **【作業内容】**
    - `Application` のプライベートメンバに `std::vector<std::pair<std::wstring, std::filesystem::file_time_type>> m_playlistSnapshot;` を追加する。
    - `Application_Render.cpp` において、`m_config.CheckPlaylistSnapshotChanged()` を `FileManager::CheckPlaylistSnapshotChanged(m_config.GetPlaylist().DefaultPlaylistPath, m_playlistSnapshot)` に置換し、さらに `m_config.GetAvailablePlaylists()` を `FileManager::GetAvailablePlaylists(m_config.GetPlaylist().DefaultPlaylistPath)` に置換する。必要に応じてインクルード (`FileManager.h`) を追加する。
    - **[完了]** `Application.h` に `<filesystem>` をインクルードし、メンバ変数 `m_playlistSnapshot` を追加した。`Application_Render.cpp` の3箇所のメソッド呼び出しを `FileManager` に移譲するように書き換えた。

### タスク4: プレイリスト操作・初期化の結線修正
    **【対象ファイル】**
    - `src/Application_Initialize.cpp` : (更新)
    - `src/Application_Playlist.cpp` : (更新)
    **【作業内容】**
    - `Application_Initialize.cpp` と `Application_Playlist.cpp` における `m_config.GetAvailablePlaylists()` を `FileManager::GetAvailablePlaylists(m_config.GetPlaylist().DefaultPlaylistPath)` に置換する。必要に応じてインクルード (`FileManager.h`) を追加する。

### タスク5: Window層（TrayMenu）の結線修正
    **【対象ファイル】**
    - `src/Window_TrayMenu.cpp` : (更新)
    **【作業内容】**
    - `m_config->GetAvailablePlaylists()` の呼び出しを `FileManager::GetAvailablePlaylists(m_config->GetPlaylist().DefaultPlaylistPath)` に置換する。必要に応じてインクルードを追加する。

### タスク6: アーキテクチャドキュメントの更新
    **【対象ファイル】**
    - `PROJECT_ARCHITECTURE.md` : (更新)
    **【作業内容】**
    - `ConfigManager` の説明から「プレイリストディレクトリのスナップショット監視」に関する記述を削除し、`FileManager` または `Application` の責務として移管されたことを明記する。
