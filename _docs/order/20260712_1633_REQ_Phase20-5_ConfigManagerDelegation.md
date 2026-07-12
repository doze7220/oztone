### 作業指示書 REQ: Phase 20-5: ConfigManager 究極の委譲化とINIオートフィル機構 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-5_ConfigManagerDelegation.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、以下の細分化されたタスクリストをそのまま含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
Phase 20-4 で物理分割した6つのファイル（Window, Playlist, Playback, LogoMenu, Visualizer, System）に対し、`ConfigManager.cpp` の大元に残存している `LoadSettings` や `ResetToDefaults` の巨大な処理を各ファイルへ委譲しカプセル化する。
さらに、INIファイルに設定値が存在しなかった場合に、デフォルト値をメモリに読み込むだけでなく「INIファイルへ自動的に追記（自己修復）」するオートフィル機構を導入し、重複するデフォルト値設定ロジックを一元化する。

*   **アーキテクチャ設計方針: オートフィルヘルパーの導入**
    *   `ConfigManager.h` に private メソッドとして `LoadOrWriteInt`, `LoadOrWriteFloat`, `LoadOrWriteString` といったヘルパー関数を定義し、大元の `ConfigManager.cpp` に実装する。
    *   これらのヘルパー関数は、`GetPrivateProfileStringW` 等でINIを読み込み、キーが存在しない場合は引数で渡されたデフォルト値をINIファイルへ書き出して追記し、そのデフォルト値を返す構造とする。

*   **アーキテクチャ設計方針: カプセル化と委譲**
    *   各UI領域に対応するサブメソッド（例: `LoadPlaylistSettings()`, `LoadWindowSettings()` など）を `ConfigManager.h` に宣言する。
    *   大元の `ConfigManager::LoadSettings()` はこれらのサブメソッドを呼び出すだけの司令塔にする。
    *   各サブメソッドの実装は対応する `ConfigManager_*.cpp` ファイル内に記述し、そこで上記のヘルパー関数を用いることで、「INIからの読み込み」「デフォルト値の設定」「不足項目のINI自動追記」を1行で完結させる。
    *   これに伴い、`ResetToDefaults()` や `LoadSettings()` に散らばっていた重複する初期値代入処理を完全に排除・一元化する。

*   **タスクリスト（厳格な1タスク1領域制）**
    *   [ ] **タスク1: ConfigManager の基盤改修**：オートフィルヘルパー関数群の実装と、サブメソッドのプロトタイプ宣言。
    *   [ ] **タスク2: Window設定の委譲**：`ConfigManager_Window.cpp` に `LoadWindowSettings` を実装し、大元から処理を移行・リファクタリングする。
    *   [ ] **タスク3: Playlist設定の委譲**：`ConfigManager_Playlist.cpp` に `LoadPlaylistSettings` を実装し、大元から処理を移行・リファクタリングする。
    *   [ ] **タスク4: Playback設定の委譲**：`ConfigManager_Playback.cpp` に `LoadPlaybackSettings` を実装し、大元から処理を移行・リファクタリングする。
    *   [ ] **タスク5: LogoMenu設定の委譲**：`ConfigManager_LogoMenu.cpp` に `LoadLogoMenuSettings` を実装し、大元から処理を移行・リファクタリングする。
    *   [ ] **タスク6: Visualizer設定の委譲**：`ConfigManager_Visualizer.cpp` に `LoadVisualizerSettings` を実装し、大元から処理を移行・リファクタリングする。
    *   [ ] **タスク7: System設定の委譲**：`ConfigManager_System.cpp` に `LoadSystemSettings` を実装し、大元から処理を移行・リファクタリングする。
    *   [ ] **タスク8: ConfigManager.cpp の最終整理とドキュメント更新**：大元の `LoadSettings` 等を司令塔として純化させ、不要な旧ロジックを全削除してビルドを通す。その後 `PROJECT_ARCHITECTURE.md` と `task.md` を更新する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 設定項目のキー名やINIファイルのセクション構造は絶対に変更しないこと。

--------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 20-5 Task 1 : ConfigManagerの基盤改修
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-5の基盤となるヘルパー関数の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク1（ConfigManagerの基盤改修） (Phase 20-5)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
ConfigManagerの大元（基盤）に、INIオートフィル機構（自己修復機能）のヘルパー関数と、各UI領域へ委譲するためのサブメソッドのプロトタイプを定義する。

*   **要件1: オートフィルヘルパーの実装 (`src/ConfigManager.h`, `src/ConfigManager.cpp`)**
    *   `ConfigManager.h` の `private` セクションに以下のヘルパーメソッドを宣言する。
        *   `int LoadOrWriteInt(const std::wstring& section, const std::wstring& key, int defaultValue);`
        *   `float LoadOrWriteFloat(const std::wstring& section, const std::wstring& key, float defaultValue);`
        *   `std::wstring LoadOrWriteString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue);`
    *   `ConfigManager.cpp` にこれらの実装を追加する。`GetPrivateProfileStringW` で読み込みを行い、キーが存在しなかった場合は引数の `defaultValue` を `WritePrivateProfileStringW` でINIファイルへ自動追記（自己修復）してから返す構造とすること。

*   **要件2: 委譲用サブメソッドのプロトタイプ宣言 (`src/ConfigManager.h`)**
    *   `ConfigManager.h` の `private` セクションに、以下の委譲先メソッドを宣言する。
        *   `void LoadWindowSettings();`
        *   `void LoadPlaylistSettings();`
        *   `void LoadPlaybackSettings();`
        *   `void LoadLogoMenuSettings();`
        *   `void LoadVisualizerSettings();`
        *   `void LoadSystemSettings();`
        *   （※必要に応じて `ResetWindowDefaults();` などのリセット用メソッドも宣言してよい）

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は「基盤となるメソッド群の追加」のみを行う。既存の `LoadSettings` や `ResetToDefaults` の中身を消したり、各サブメソッドの具体的な実装を書いたりは**絶対にしない**こと（後続のタスクで行うため）。

--------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-5 Task 2 : Window設定の委譲
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow設定の委譲の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク2（Window設定の委譲）(Phase 20-5)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`ConfigManager.cpp` の `LoadSettings` や `ResetToDefaults` に残存している Window 関連の初期値設定およびINI読み込み処理を、タスク1で作成したヘルパー関数を用いて `ConfigManager_Window.cpp` へ委譲し、コードを純化する。

*   **要件1: `LoadWindowSettings()` の実装 (`src/ConfigManager_Window.cpp`)**
    *   `ConfigManager_Window.cpp` 内に `void ConfigManager::LoadWindowSettings()` を実装する。
    *   `ConfigManager.cpp` の `LoadSettings()` 内で行われていた Window 関連（`WindowX`, `WindowY`, `WindowWidth`, `WindowHeight`, `ZOrder`, `SavePositionOnExit`, `EnableResize`, `LockWindowPosition`, `BackgroundArtMode`, `EnableShadow`, `BgDarkenOpacity`, `BgOpacity` など）の読み込み処理をこちらへ移行する。
    *   その際、タスク1で作成した `LoadOrWriteInt` や `LoadOrWriteFloat` 等のオートフィルヘルパー関数を使用する形にリファクタリングし、「メモリ変数の初期化」と「INIからの読み書き」を1行で完結させる構造とする。

*   **要件2: 大元からの処理削除と司令塔化 (`src/ConfigManager.cpp`)**
    *   大元の `ConfigManager.cpp` の `LoadSettings()` 内から、Window関連の読み込みロジック（`GetPrivateProfileIntW` 等）を全て削除し、代わりに `LoadWindowSettings();` を呼び出すように変更する。
    *   大元の `ConfigManager.cpp` の `ResetToDefaults()` 内から、Window関連の初期値代入ロジック（`m_windowX = CW_USEDEFAULT;` など）を全て削除する（ヘルパー関数へデフォルト値が統合されるため不要となる）。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は「Window設定」に関する部分のみを委譲すること。PlaylistやPlayback等の他の設定は今後のタスクで移行するため、大元の `LoadSettings` や `ResetToDefaults` にそのまま残しておくこと。
*   **動作の維持**: オートフィルヘルパー関数に渡すデフォルト値は、元々 `ResetToDefaults` や `LoadSettings` のフォールバックで使われていた値と完全に一致させること。

--------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-5 Task 3 : Playlist設定の委譲
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPlaylist設定の委譲の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md）の「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク3（Playlist設定の委譲）(Phase 20-5)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`ConfigManager.cpp` の `LoadSettings` や `ResetToDefaults` に残存している Playlist 関連の初期値設定およびINI読み込み処理を、タスク1で作成したヘルパー関数を用いて `ConfigManager_Playlist.cpp` へ委譲し、コードを純化する。

*   **要件1: `LoadPlaylistSettings()` の実装 (`src/ConfigManager_Playlist.cpp`)**
    *   `ConfigManager_Playlist.cpp` 内に `void ConfigManager::LoadPlaylistSettings()` を実装する。
    *   `ConfigManager.cpp` の `LoadSettings()` 内で行われていた Playlist 関連の設定項目（`DefaultPlaylistPath`, `PlaylistWidth`, `PlaylistHoverWidth`, `PlaylistPosition`, `IsPlaylistPinned`、ツールバー設定、各種フォント・色設定、オフセット等）の読み込み処理をこちらへ移行する。
    *   タスク1で作成した `LoadOrWriteInt` や `LoadOrWriteFloat`, `LoadOrWriteString` 等のオートフィルヘルパー関数を使用し、「メモリ変数の初期化」と「INIからの読み書き」を1行で完結させる構造へリファクタリングする。
    *   ※ `DefaultPlaylistPath` のように、exeのパスを取得して文字列を組み立てる必要がある特殊なフォールバック処理も、すべてこのメソッド内にカプセル化して完結させること。

*   **要件2: 大元からの処理削除と司令塔化 (`src/ConfigManager.cpp`)**
    *   大元の `ConfigManager.cpp` の `LoadSettings()` 内から、Playlist関連の読み込みロジックを全て削除し、代わりに `LoadPlaylistSettings();` を呼び出すように変更する。
    *   大元の `ConfigManager.cpp` の `ResetToDefaults()` 内から、Playlist関連の初期値代入ロジックを全て削除する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は「Playlist設定」に関する部分のみを委譲すること。Playback等の他の設定は今後のタスクで移行するため、大元の `LoadSettings` や `ResetToDefaults` にそのまま残しておくこと。
*   **動作の維持**: オートフィルヘルパー関数に渡すデフォルト値は、元々 `ResetToDefaults` や `LoadSettings` のフォールバックで使われていた値と完全に一致させること。
--------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-5 Task 4 : Playback設定の委譲
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPlayback設定の委譲の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1644_RES_Phase20-5_ConfigManagerDelegation.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク4（Playback設定の委譲）(Phase 20-5)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`ConfigManager.cpp` の `LoadSettings` や `ResetToDefaults` に残存している Playback（再生コントロール）、Volume（音量コントロール）、SeekBar（シークバー）関連の初期値設定およびINI読み込み処理を、オートフィルヘルパー関数を用いて `ConfigManager_Playback.cpp` へ委譲し、コードを純化する。

*   **要件1: `LoadPlaybackSettings()` の実装 (`src/ConfigManager_Playback.cpp`)**
    *   `ConfigManager_Playback.cpp` 内に `void ConfigManager::LoadPlaybackSettings()` を実装する。
    *   `ConfigManager.cpp` の `LoadSettings()` 内で行われていた以下の設定項目の読み込み処理をこちらへ移行する。
        *   再生コントロール (`[Layout_PlaybackControls]`)
        *   音量コントロール (`[Layout_VolumeControl]`) および デフォルト音量 (`[Audio] DefaultVolume`)
        *   シークバー (`[Layout_SeekBar]`)
    *   オートフィルヘルパー関数（`LoadOrWriteInt`, `LoadOrWriteFloat` 等）を使用し、「メモリ変数の初期化」と「INIからの読み書き」を1行で完結させる構造へリファクタリングする。

*   **要件2: 大元からの処理削除と司令塔化 (`src/ConfigManager.cpp`)**
    *   大元の `ConfigManager.cpp` の `LoadSettings()` 内から、Playback、Volume、SeekBar 関連の読み込みロジックを全て削除し、代わりに `LoadPlaybackSettings();` を呼び出すように変更する。
    *   大元の `ConfigManager.cpp` の `ResetToDefaults()` 内から、関連する初期値代入ロジックを全て削除する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は「Playback設定（SeekBar, Volume含む）」に関する部分のみを委譲すること。LogoMenuやVisualizer等の他の設定は今後のタスクで移行するため、大元の `LoadSettings` や `ResetToDefaults` にそのまま残しておくこと。
*   **動作の維持**: オートフィルヘルパー関数に渡すデフォルト値は、元々 `ResetToDefaults` や `LoadSettings` で使われていた値と完全に一致させること。

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
