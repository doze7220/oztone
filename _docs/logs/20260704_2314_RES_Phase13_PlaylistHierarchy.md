# Phase 13: プレイリストの階層化UIとツールバーの実装 計画書

## 1. 実装目的
現在のプレイリストUI（`PlaylistWidget`）を拡張し、「曲一覧モード」と「プレイリスト（.ozlファイル）一覧モード」の2階層の表示を可能にする。また、最上部に固定のツールバーを配置し、各モードに応じた操作（階層移動、曲削除、プレイリストの新規作成・削除など）を直感的に行えるようにする。

## 2. アーキテクチャ設計と変更点

### 2.1 LayoutCalculator の拡張
プレイリスト領域を「固定ツールバー領域」と「スクロール可能なリスト領域」に分割するため、レイアウト計算式を更新する。
* **新規構造体**: `PlaylistToolbarLayout` を追加し、ツールバー全体の領域、各ボタンの当たり判定領域（矩形の配列）、およびホバー説明テキスト表示用の領域（`textRect`）を定義する。
* **`CalculatePlaylistLayout` の修正**: ツールバーの高さ（例: 60px）を確保し、スクロール領域のクリッピング矩形（`clipRect`）と開始Y座標（`startY`）をツールバーの下から開始するようにオフセットを適用する。

### 2.2 Window クラスの入力制御拡張（責務の分離）
描画と入力の分離アーキテクチャを維持するため、`Window` クラスにてツールバーへのホバーとクリックを独立して検知する。
* **ホバー状態の管理**: `m_playlistToolbarHoveredIndex` を追加。`WM_MOUSEMOVE` 時にマウス座標からツールバー内のどのボタン（0〜2）にホバーしているかを判定し、インデックスを保持する（ホバー外は -1）。
* **コールバックの追加**: `SetPlaylistToolbarClickCallback(std::function<void(int)>)` を追加。`WM_LBUTTONDOWN` 時、マウスがツールバーのボタン上にあればこのコールバックを発火させる。
* リスト領域へのクリック判定（インデックス逆算）もツールバーの高さを加味して開始位置をずらすよう修正する。

### 2.3 WidgetContext の拡張
`Window` で取得したホバー状態を描画層（Widget）へ伝達する。
* `WidgetContext` に `int playlistToolbarHoveredIndex;` を追加し、毎フレーム `Application::ForceRender` 内で `Window` から取得した値をセットする。

### 2.4 PlaylistWidget の状態管理と描画拡張
* **状態管理**: `bool m_isListViewMode = false;` をメンバ変数として追加。`Renderer` にこれらを操作・取得するパススルーメソッド（`SetPlaylistListViewMode`, `IsPlaylistListViewMode`）を追加し、`Application` から制御可能にする。
* **ツールバー描画**:
    * モード（`m_isListViewMode`）に応じて、3つのボタンアイコンを描画する。
    * モードと `ctx.playlistToolbarHoveredIndex` に基づき、指定された日本語の説明テキストを `textRect` 領域へ描画する。
    * プレイリスト一覧モード時の「上の階層へ」ボタンは、グレーアウト処理（透明度を下げる、または斜線を引く）を行い、クリック無効であることを視覚的に明示する。
* **リスト領域描画**:
    * `m_isListViewMode` が `true` の場合、`config->GetAvailablePlaylists()` を呼び出し、ファイル名（.ozl等）の一覧を描画する。
    * 現在設定されているデフォルトプレイリストパスと一致する項目をハイライト（再生中と同じ色など）で描画する。

### 2.5 Application クラスのロジック連携
`Window` からのコールバックを受け、モードに応じたバックエンド処理を振り分ける。
* **ツールバークリックコールバック (`OnPlaylistToolbarClick`)**:
    * `buttonIndex == 0` (上の階層へ):
        * 曲一覧モード時のみ `Renderer->SetPlaylistListViewMode(true)` を呼び出し、リスト一覧モードへ遷移する。
    * `buttonIndex == 1`:
        * 曲一覧モード時 (曲削除): 現在再生中の曲をプレイリストから削除するロジックを実行する。
        * プレイリスト一覧モード時 (新規作成): 既存の `CreateNewPlaylist()` を呼び出し、曲一覧モードへ自動復帰させる。
    * `buttonIndex == 2`:
        * 曲一覧モード時 (全曲削除): 既存の `ClearPlaylist()` を呼び出す。
        * プレイリスト一覧モード時 (リスト削除): 現在選択されている（アクティブな）プレイリストファイルを物理削除し、UIを更新する。必要に応じて次のプレイリストへフォールバックする。
* **リストアイテムクリックコールバック (`OnPlaylistClick`)**:
    * `Renderer->IsPlaylistListViewMode()` が `true` の場合：
        * 渡されたインデックスから対象のプレイリストファイルパスを取得し、既存の `SwitchPlaylist(path)` を呼び出す。
        * その後、`Renderer->SetPlaylistListViewMode(false)` で曲一覧モードへ戻す。
    * `false` の場合：
        * 既存の処理通り、対象の曲へジャンプ再生（`PlaylistManager::JumpToIndex`）を実行する。

---

## 3. タスクリスト

- [x] **Task 1: レイアウトおよびコンテキストの拡張**
    - `LayoutCalculator.h / cpp` に `PlaylistToolbarLayout` 構造体と計算関数を追加し、`CalculatePlaylistLayout` をツールバー対応（Yオフセット追加）に修正する。
    - `Widget.h` の `WidgetContext` に `playlistToolbarHoveredIndex` を追加する。
- [x] **Task 2: 入力制御の拡張 (Windowクラス)**
    - `Window.h / cpp` にツールバーの当たり判定ロジックと `m_playlistToolbarHoveredIndex` を追加する。
    - ツールバー用のクリックコールバック `SetPlaylistToolbarClickCallback` を実装し、マウスメッセージ（`WM_MOUSEMOVE`, `WM_LBUTTONDOWN`）を修正する。
- [x] **Task 3: PlaylistWidgetの拡張 (状態管理と描画)**
    - `PlaylistWidget` に `m_isListViewMode` フラグを追加し、`Renderer` にアクセサを設ける。
    - ツールバー領域の描画（アイコン、ホバー時の日本語説明テキスト、プレイリスト一覧モード時のグレーアウト表現）を実装する。
    - `m_isListViewMode == true` 時のプレイリスト一覧表示（ファイル名描画、現在アクティブなリストのハイライト）を実装する。
- [x] **Task 4: Applicationのロジック結合**
    - `Application` の初期化処理にて、新しいツールバークリックコールバックをバインドする。
    - コールバック内で、モードに応じた処理分岐（モード遷移、曲削除、全曲削除、新規作成、リスト削除）を実装する。
    - リストクリックコールバックを分岐させ、プレイリスト一覧モード時は `SwitchPlaylist` を実行してモードを戻すように修正する。

## 4. 詳細作業内容
* `Widgets.h`, `Widgets.cpp` を修正し、ツールバー用のテキストフォーマット（`m_toolbarTextFormat`, `m_toolbarIconFormat`）を追加・初期化。
* `PlaylistWidget::Draw` 内に、`ctx.isPlaylistListViewMode` の状態に応じたツールバー（ボタンアイコン、ホバー時の日本語ツールチップ）の描画処理を実装。
* `PlaylistWidget::Draw` 内のリスト描画ループを分岐させ、一覧モード時は `ConfigManager::GetAvailablePlaylists()` で取得したプレイリスト一覧を描画するように対応。現在選択中のプレイリストはハイライト表示。
* `PlaylistManager.h`, `PlaylistManager.cpp` に `RemoveCurrentTrack()` メソッドを新規追加し、現在の再生曲のシャッフルリストインデックスを再構築しながら削除する処理を実装。
* `Application.cpp` にて、`SetPlaylistToolbarClickCallback` を登録。インデックスとモードに応じたロジック（フォルダ階層の移動、曲/リストの削除、新規作成など）を実装。
* `Application.cpp` の `SetPlaylistClickCallback` を拡張し、一覧モード時にアイテムがクリックされた場合は `SwitchPlaylist()` を呼んで一覧モードを終了するように実装。

## 5. Hotfix: 階層化UIのレイアウト崩れ・クリック判定・上下2段配置の修正
* `ConfigManager.h`, `ConfigManager.cpp`: `[Layout_Playlist]` にツールバー関連のレイアウト設定項目（`ToolbarHeight`, `ToolbarIconSize`, `ToolbarIconSpacing`, `ToolbarTextOffsetY`, `ToolbarTextFontSize`）を追加し、外部から調整可能に変更。
* `LayoutCalculator.cpp`: `CalculatePlaylistLayout` を修正し、ツールバーを上下2段構成（上段にアイコンを中央配置、下段に説明テキスト）に再構築。クリッピング領域 (`clipRect`) が正しく `ToolbarHeight` を考慮するよう実装。
* `Widgets.cpp`: `m_toolbarTextFormat` に `DWRITE_TEXT_ALIGNMENT_CENTER` を設定し、テキストを中央揃えに修正。各フォントサイズに INI 設定値を反映。
* `Application.cpp`: `SetPlaylistClickCallback` にて、Y座標がツールバー領域の場合は処理を弾き（`SetPlaylistToolbarClickCallback` に委譲）、リスト領域の場合は `ToolbarHeight` を差し引いて正しいスクロール/インデックス計算が行われるよう補正処理を実装。

## 6. Hotfix 2: プレイリスト一覧モードのクリック対応と状態引き上げ
* `Renderer.h`, `Renderer.cpp`: `Renderer` クラス内に持たせていた `m_isPlaylistListViewMode` を削除し、状態管理の責務をアプリケーション層へ移管。
* `Application.h`, `Application.cpp`: モード状態の管理変数 `bool m_isPlaylistListViewMode = false;` を `Application` クラスに引き上げ、`Renderer::Render` へ引き渡すようにシグネチャおよび呼び出し部（`ForceRender` 等）を修正（ビルドエラー修正含む）。
* `Application.cpp`: `SetPlaylistClickCallback` および `SetPlaylistToolbarClickCallback` の分岐処理において、自身の `m_isPlaylistListViewMode` フラグを参照するように変更。リスト項目クリック時に一覧モードであれば `SwitchPlaylist` を実行し、モードを `false` に戻す処理を再徹底。

## 7. Hotfix 3: プレイリスト一覧クリック無反応バグの調査と完全修正
* **バグの原因特定**: プレイリスト一覧モードの描画側 (`PlaylistWidget::Draw`) が、曲一覧と同じ「中央寄せスクロール（`baseScrollY`）」および「上限下限のクランプ」を適用して描画していたのに対し、クリック判定側 (`Application.cpp` の `SetPlaylistClickCallback`) はこれらを考慮せず単純な割り算でインデックスを計算していたため、見た目上の座標と内部の計算座標にズレが生じ、正しいインデックスが取得できず無反応となっていた。
* `Application.cpp`: プレイリスト一覧モード時のクリック判定にて、選択中のプレイリストインデックスとプレイリスト総数を用いた `baseScrollY` と `clamp` 処理を組み込み、描画側と完全に一致するY座標計算ロジックを実装。
* `Renderer.h`, `Renderer.cpp`: `UpdateAnimation` メソッドのシグネチャに `isPlaylistListViewMode` フラグを追加し、`WidgetContext` へ状態を渡すように変更。
* `Widgets.cpp`: `PlaylistWidget::UpdateAnimation` および `PlaylistWidget::Draw` において、プレイリスト一覧モード時には「曲の数・曲のインデックス」ではなく「プレイリストの数・現在選択中のプレイリストインデックス」を用いてレイアウト計算（`CalculatePlaylistLayout`）を行うように修正。これによりモードごとのスクロール挙動とクリック判定が完全に同期。
