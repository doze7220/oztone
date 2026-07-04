# 実装計画レポート: Phase 11-2 プレイリストの自動認識とサブメニュー化

## 1. 目的とスコープ
本タスクは、システムトレイのコンテキストメニューに「プレイリスト (Playlists)」というサブメニューを追加し、ディレクトリ内に存在するプレイリストファイル（`.ozl`, `.lst`）を自動スキャンして動的にメニュー項目として一覧表示し、クリックでプレイリストを切り替えられるようにする。
現在再生中のプレイリスト機能（プレイリスト内の曲削除など）の拡張は本スコープ外とする。

## 2. アーキテクチャ設計

### 2.1 プレイリストファイルの自動スキャン機構
*   **配置先**: `ConfigManager` クラス
*   **追加メソッド**: `std::vector<std::wstring> GetAvailablePlaylists() const;`
*   **処理フロー**:
    1.  `GetDefaultPlaylistPath()` の戻り値から `std::filesystem::path` を用いて親ディレクトリを取得。親ディレクトリがない（カレントディレクトリなどの）場合は `std::filesystem::current_path()` を基準とする。
    2.  `std::filesystem::directory_iterator` を用いてディレクトリ内を走査。
    3.  拡張子が `.ozl` または `.lst` のファイルのみを抽出。
    4.  パスの文字列リスト（`std::vector<std::wstring>`）として返す。アルファベット順などでソートしておくことが望ましい。

### 2.2 システムトレイメニューの動的ネスト化 (Window)
*   **動的IDの定義**: `Window.h` に `ID_TRAY_PLAYLIST_START = 2000`、および配置用の `ID_TRAY_PLAYLIST_MENU` を定義。
*   **状態保持**: 動的に生成したメニュー項目とファイルパスの紐付けを保持するため、`Window` クラス内に `std::vector<std::wstring> m_dynamicPlaylistPaths;` を追加。
*   **メニュー構築処理 (`WM_TRAYICON` / `WM_RBUTTONUP`)**:
    1.  メインの `CreatePopupMenu()` 構築ループ内で、予約ID（`ID_TRAY_PLAYLIST_MENU`）に到達した際、サブメニュー用の `hPlaylistMenu = CreatePopupMenu();` を生成。
    2.  `ConfigManager::GetAvailablePlaylists()` を呼び出しリストを取得。取得したリストを `m_dynamicPlaylistPaths` に保存。
    3.  リストをループし、`hPlaylistMenu` に `AppendMenuW` で項目を追加。表示名は `std::filesystem::path(path).stem().wstring()` で拡張子を除去して美しく表示。
    4.  現在のプレイリスト（`GetDefaultPlaylistPath()` と一致）には `MF_CHECKED` を付与。
    5.  コマンドIDは `ID_TRAY_PLAYLIST_START + index` とする。
    6.  構築した `hPlaylistMenu` を `AppendMenuW` (MF_POPUP) でメインメニューに追加。
*   **メモリ管理**: サブメニュー（`hPlaylistMenu`）はメインメニューに `MF_POPUP` としてアタッチされるため、メインメニューに対して `DestroyMenu()` を呼べばWin32の仕様で再帰的に破棄される。メモリリークは発生しない。

### 2.3 プレイリストの動的切り替え処理
*   **Windowからの伝達**: `Window.h` にコールバック `SetPlaylistSwitchCallback(std::function<void(const std::wstring&)> cb)` を追加。`WM_COMMAND` にて `wmId >= ID_TRAY_PLAYLIST_START` の範囲を捕捉し、`m_dynamicPlaylistPaths[wmId - ID_TRAY_PLAYLIST_START]` をコールバックへ渡す。
*   **Applicationの切り替え処理**:
    *   `Application::SwitchPlaylist(const std::wstring& filepath)` を新設。
    *   処理フロー:
        1.  `m_config->SetDefaultPlaylistPath(filepath);` でINI設定を更新。
        2.  `ClearPlaylist();` を呼び出し、現在の再生停止、キュークリア、UIリセット、バックグラウンドスレッドの安全な停止などを行う。
        3.  `m_playlistManager.LoadFromFile(filepath);` で新しいリストをロード。
        4.  曲数が1以上あれば、最初の曲の再生と次曲の先読みを開始する。（`Initialize` の初回ロード時と同様の処理）

## 3. タスクリスト

- [x] **Task 1: 自動スキャン機能の実装 (`ConfigManager`)**
    - `ConfigManager.h/cpp` に `GetAvailablePlaylists()` を実装。
    - `<filesystem>` を利用したディレクトリ走査と拡張子フィルタリング、ソート処理の実装。
- [x] **Task 2: UIおよびメニューの動的構築 (`Window`)**
    - `Window.h` に定数（`ID_TRAY_PLAYLIST_START`, `ID_TRAY_PLAYLIST_MENU`）と `m_dynamicPlaylistPaths` を追加。
    - `Window.cpp` の `TRAY_MENU_ORDER` 配列に `ID_TRAY_PLAYLIST_MENU` を適切な位置に追加。
    - `WM_TRAYICON` でのメニュー構築時、サブメニューの作成および `AppendMenuW` によるアタッチ処理を実装。
- [x] **Task 3: メッセージルーティングとコールバック実装 (`Window` & `Application`)**
    - `Window::SetPlaylistSwitchCallback` の追加と `WM_COMMAND` でのイベント発火処理の実装。
    - `Application::SwitchPlaylist` の実装。
    - `Application::Initialize` でコールバックを登録。
- [x] **Task 4: 動作確認とメモリチェック**
    - 複数のプレイリストファイルを作成し、サブメニューに正しく表示されるか確認。
    - クリック時に再生が停止し、新しいプレイリストが読み込まれ再生が開始されるか確認。
    - サブメニュー展開時にメモリリークが発生していないことを確認。

## 4. 詳細作業内容
* **Task 1**: `ConfigManager::GetAvailablePlaylists()` を実装しました。実行ファイルのパスまたはカレントディレクトリを基準に `.ozl` および `.lst` ファイルを走査し、一覧を取得できるようにしました。コンパイルエラーを解消するため `<vector>` などのインクルードも追加しました。
* **Task 2**: `Window.h` に `ID_TRAY_PLAYLIST_MENU` などの必要な定数と `m_dynamicPlaylistPaths` を追加し、重複していた不要な定数定義を削除しました。`Window.cpp` にて、トレイアイコン右クリック時のメニュー構築ループに動的プレイリストメニュー構築処理を追加しました。現在のプレイリストファイルにチェックマークを付ける処理も含まれています。
* **Task 3**: `Application.cpp` に `SwitchPlaylist()` を実装しました。切り替え時に `ClearPlaylist()` を呼び出し、安全に既存状態とパーサーキューをクリアした上で、新しいリストをロードして再生・先読み処理を開始する設計にしました。
* **Task 4**: すべてのソースのコンパイルが成功し、リンクエラーが発生せずに `OZtone.exe` がビルドされることを確認しました。

### [Hotfix] プレイリスト切り替え時の不具合修正
1. **問題**: `SwitchPlaylist()` 時に `ClearPlaylist()` を呼び出すことで、新しいプレイリストファイルの内容が空に上書きされる不具合が発生。
   **対応**: `ClearPlaylist()` の呼び出しをやめ、内部でオーディオ停止・パーサーキューのクリア・UIリセットを直接行うように修正しました。
2. **問題**: `PlaylistManager::LoadFromFile()` が既存リストへの「追記」動作を行う仕様だったため、切り替え後に前のプレイリストが残ったままになる不具合が発生。
   **対応**: 新しいプレイリストをロードする直前に、明示的に `m_playlistManager.Clear()` を呼び出してメモリ上のリストを初期化するように修正しました。
