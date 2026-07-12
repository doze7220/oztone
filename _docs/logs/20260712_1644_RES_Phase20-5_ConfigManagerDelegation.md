# RES:実装計画・作業レポート Phase 20-5: ConfigManager 究極の委譲化とINIオートフィル機構

## 1. 実装目的
Phase 20-4 で物理分割した6つのファイル（Window, Playlist, Playback, LogoMenu, Visualizer, System）に対し、`ConfigManager.cpp` の大元に残存している `LoadSettings` や `ResetToDefaults` の巨大な処理を各ファイルへ委譲しカプセル化する。
また、INIファイルに設定値が存在しなかった場合に、デフォルト値をメモリに読み込むだけでなく「INIファイルへ自動的に追記（自己修復）」するオートフィル機構を導入し、重複するデフォルト値設定ロジックを一元化する。

## 2. アーキテクチャ設計
### 要件1: オートフィルヘルパーの導入
    - `ConfigManager.h` に private メソッドとして `LoadOrWriteInt`, `LoadOrWriteFloat`, `LoadOrWriteString` といったヘルパー関数を定義し、大元の `ConfigManager.cpp` に実装する。
    - これらのヘルパー関数は、`GetPrivateProfileStringW` 等でINIを読み込み、キーが存在しない場合は引数で渡されたデフォルト値をINIファイルへ書き出して追記し、そのデフォルト値を返す構造とする。

### 要件2: カプセル化と委譲
    - 各UI領域に対応するサブメソッド（例: `LoadPlaylistSettings()`, `LoadWindowSettings()` など）を `ConfigManager.h` に宣言する。
    - 大元の `ConfigManager::LoadSettings()` はこれらのサブメソッドを呼び出すだけの司令塔にする。
    - 各サブメソッドの実装は対応する `ConfigManager_*.cpp` ファイル内に記述し、そこで上記のヘルパー関数を用いることで、「INIからの読み込み」「デフォルト値の設定」「不足項目のINI自動追記」を1行で完結させる。
    - これに伴い、`ResetToDefaults()` や `LoadSettings()` に散らばっていた重複する初期値代入処理を完全に排除・一元化する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManager の基盤改修
    - オートフィルヘルパー関数群の実装と、サブメソッドのプロトタイプ宣言。
[x] タスク2: Window設定の委譲
    - `ConfigManager_Window.cpp` に `LoadWindowSettings` を実装し、大元から処理を移行・リファクタリングする。
[x] タスク3: Playlist設定の委譲
    - `ConfigManager_Playlist.cpp` に `LoadPlaylistSettings` を実装し、大元から処理を移行・リファクタリングする。
[x] タスク4: Playback設定の委譲
    - `ConfigManager_Playback.cpp` に `LoadPlaybackSettings` を実装し、大元から処理を移行・リファクタリングする。
[x] タスク5: LogoMenu設定の委譲
    - `ConfigManager_LogoMenu.cpp` に `LoadLogoMenuSettings` を実装し、大元から処理を移行・リファクタリングする。
[ ] タスク6: Visualizer設定の委譲
    - `ConfigManager_Visualizer.cpp` に `LoadVisualizerSettings` を実装し、大元から処理を移行・リファクタリングする。
[ ] タスク7: System設定の委譲
    - `ConfigManager_System.cpp` に `LoadSystemSettings` を実装し、大元から処理を移行・リファクタリングする。
[ ] タスク8: ConfigManager.cpp の最終整理とドキュメント更新
    - 大元の `LoadSettings` 等を司令塔として純化させ、不要な旧ロジックを全削除してビルドを通す。その後 `PROJECT_ARCHITECTURE.md` と `task.md` を更新する。

## 4. 詳細作業内容
### タスク1: ConfigManager の基盤改修
    - ConfigManager.h にオートフィル用のヘルパーメソッド (`LoadOrWriteInt`, `LoadOrWriteFloat`, `LoadOrWriteString`) と、各UI設定を委譲するためのサブメソッド群 (`LoadWindowSettings` 等) を定義。
    - ConfigManager.cpp にヘルパーメソッドの実装を追加。`GetPrivateProfileStringW` でキーが存在しない場合は `WritePrivateProfileStringW` でデフォルト値を自動追記する処理（自己修復機能）を構築。
### タスク2: Window設定の委譲
    - `ConfigManager_Window.cpp` に `ConfigManager::LoadWindowSettings()` を実装。
    - `ConfigManager.cpp` の `LoadSettings()` と `ResetToDefaults()` から Window 関連のロジックを削除・移行し、ヘルパー関数を用いたオートフィル化を適用。
### タスク3: Playlist設定の委譲
    - `ConfigManager_Playlist.cpp` に `ConfigManager::LoadPlaylistSettings()` を実装。
    - `ConfigManager.cpp` の `LoadSettings()` から Playlist設定の読み込みを削除し、`LoadPlaylistSettings()` に委譲。オートフィルを適用。
    - `ResetToDefaults()` から Playlist 関連の変数の初期化を削除。
### タスク4: Playback設定の委譲
    - `ConfigManager_Playback.cpp` に `ConfigManager::LoadPlaybackSettings()` を実装。
    - `ConfigManager.cpp` の `LoadSettings()` と `ResetToDefaults()` から Playback, Volume, SeekBar 関連の読み込みや初期化処理を完全に削除し、`LoadPlaybackSettings()` の呼び出しへ置き換えた。
    - 各変数に対して `LoadOrWriteInt` や `LoadOrWriteFloat`, `LoadOrWriteString` を使用するオートフィル機構を適用。
### タスク5: LogoMenu設定の委譲
    - `ConfigManager_LogoMenu.cpp` に `ConfigManager::LoadLogoMenuSettings()` を実装。
    - `ConfigManager.cpp` の `LoadSettings()` と `ResetToDefaults()` から AppLogo および LogoMenu 関連の読み込みや初期化処理を完全に削除し、`LoadLogoMenuSettings()` の呼び出しへ置き換えた。
    - 各変数に対して `LoadOrWriteInt` や `LoadOrWriteFloat`, `LoadOrWriteString` を使用するオートフィル機構を適用。
### タスク6: Visualizer設定の委譲
    - (未実施)
### タスク7: System設定の委譲
    - (未実施)
### タスク8: ConfigManager.cpp の最終整理とドキュメント更新
    - (未実施)
