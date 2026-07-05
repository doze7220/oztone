# Phase 14: INIファイルのホットリロード機能 実装計画

## 1. 実装目的
アプリケーション実行中に外部エディタ等で `OZtone.ini` が上書き保存されたことを検知し、アプリケーションを再起動することなく、即座に設定（レイアウト、カラー、フォントサイズなど）をUI描画に反映させる「ホットリロード」機能を実装する。

## 2. アーキテクチャ設計

### 2.1 INIファイルのタイムスタンプ監視 (ConfigManager)
`ConfigManager` に `OZtone.ini` の最終更新日時を記録・比較する仕組みを追加し、更新検知の責務を持たせる。
- **追加メンバ変数**:
  - `std::filesystem::file_time_type m_lastIniWriteTime;`
- **追加メソッド**:
  - `bool CheckForUpdates()`: `std::filesystem::last_write_time` を用いて現在のファイルの更新日時を取得し、`m_lastIniWriteTime` と比較する。更新されていれば `true` を返し、`m_lastIniWriteTime` を更新する。※ファイルが他のプロセスによってロックされていたり、一時的に消失している場合（エディタの保存挙動による）に備え、`std::error_code` を用いた例外なしのエラーハンドリングを実装し、クラッシュを防止する。
- **アクセス制御の変更**:
  - `LoadSettings()` を `public` アクセスに変更し、外部（`Application`）から設定の再読み込みをトリガー可能にする。

### 2.2 メインループでの定期監視 (Application)
`Application` クラスが中枢として設定の変更を定期的に監視し、更新を検知した際に再読み込みと再描画のパイプラインをトップダウンで起動する。
- **監視ロジックの追加**:
  - `Application::Run` のメインループ内、または `ForceRender()` 内において、毎フレームのファイルアクセスによるI/O負荷を防ぐため、時刻比較 (`GetTickCount64()` 等) を用いて1秒（1000ms）ごとに1回だけ `m_config.CheckForUpdates()` を実行するインターバル制御を導入する。
- **更新検知時のフロー**:
  1. `m_config.LoadSettings()` を呼び出し、最新の INI の内容をメモリ上のメンバ変数へ再ロードする。
  2. `m_renderer.ReloadResources()` (新設) を呼び出し、レンダリングに必要なリソース・レイアウトの動的再構築を指示する。

### 2.3 キャッシュとリソースの動的再構築 (Renderer / Widgets)
`Renderer` および配下の `Widgets` のキャッシュや依存リソースを安全に再構築し、最新の設定値を反映させる。
- **Renderer の変更点**:
  - `void ReloadResources()` メソッドを新設する。
  - 内部で `m_forceTextLayoutUpdate = true;` をセットし、次回描画時に `UpdateTextLayouts` でテキストのレイアウトキャッシュ（`IDWriteTextLayout`）が確実に再生成されるように状態をクリアする。
  - 自身が保持する固定ブラシ（`m_bgDarkenBrush`, `m_fallbackBlackBrush`）などを、`ConfigManager` の最新値を用いて再生成する処理を実行する。
  - `m_widgets` に登録されたすべての Widget の `ReleaseResources()` を呼び出して古いリソースを解放した後、`CreateResources()` を再実行し、各 Widget が内部に抱えるフォントキャッシュやブラシなどの D2D/DWrite リソースを最新設定値で安全に作り直させるループを実装する。

## 3. 実装タスクリスト
- [x] `ConfigManager.h` に `m_lastIniWriteTime` 変数と `CheckForUpdates()` メソッドの宣言を追加し、`LoadSettings()` を `public` セクションへ移動する。
- [x] `ConfigManager.cpp` の `Initialize()` 内で `m_lastIniWriteTime` の初期化処理（初回ロード時のタイムスタンプ取得）を追加する。
- [x] `ConfigManager.cpp` に `CheckForUpdates()` の実装を追加する（`std::filesystem::last_write_time` の使用とエラーハンドリング）。
- [x] `Application.h` に INI 監視用のタイマー変数（例: `ULONGLONG m_lastConfigCheckTime = 0;`）を追加する。
- [x] `Application.cpp` の `ForceRender()` または `Run()` 内に、1秒間隔で `m_config.CheckForUpdates()` を呼び出す低負荷なインターバル処理を追加する。
- [x] 同インターバル処理内で、更新検知時に `m_config.LoadSettings()` と `m_renderer.ReloadResources()` を呼び出す再構築トリガーロジックを実装する。
- [x] `Renderer.h` に `ReloadResources()` メソッドの宣言を追加する。
- [x] `Renderer.cpp` に `ReloadResources()` の実装を追加する（テキストレイアウト更新フラグのセット、自身のリソース再生成、全 Widget の `ReleaseResources()` / `CreateResources()` 呼び出しループ処理）。

## 4. 詳細作業内容
- `ConfigManager.h` に `<filesystem>` をインクルードし、`m_lastIniWriteTime` と `CheckForUpdates()` を追加。`LoadSettings()` を public に変更。
- `ConfigManager.cpp` の `Initialize()` の末尾で `std::filesystem::last_write_time` （エラーコード付き）を用いてタイムスタンプの初期化処理を実装。
- `ConfigManager::CheckForUpdates()` において、ファイルアクセスエラー時にクラッシュしないよう例外無しのオーバーロードで実装し、更新チェックを行うようにした。
- `Application.h` に `ULONGLONG m_lastConfigCheckTime = 0;` を追加。
- `Application.cpp` の `Run()` ループの先頭に、`GetTickCount64()` を用いて 1000ms 間隔で `CheckForUpdates()` を呼び出し、更新があった場合に `m_config.LoadSettings()` と `m_renderer.ReloadResources()` を実行するロジックを実装。
- `Renderer.h` / `Renderer.cpp` に `ReloadResources()` を新設。内部で `m_forceTextLayoutUpdate` を true にし、`UpdateTextLayouts` 時にキャッシュクリアが行われるよう改修。さらに固定ブラシの再生成と全 Widget の `ReleaseResources` および `CreateResources` 呼び出しループを実装し、動的な UI リソースの再構築機能を提供した。
