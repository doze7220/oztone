# RES:実装計画・作業レポート Phase 16-3: プレイリスト項目のダブルクリック選択化

## 1. 実装目的
プレイリスト内の項目（曲やプレイリストファイル）の操作において、「シングルクリック＝即時再生・切り替え」となっていた仕様を改め、「シングルクリック＝項目の選択（フォーカス）」「ダブルクリック＝決定（再生・切り替えの実行）」という挙動に変更し、誤操作の防止と直感的な操作感の向上を図る。

## 2. アーキテクチャ設計
### 要件1:ダブルクリックイベントの捕捉 (Window)
    - `Window::Initialize` にて、ウィンドウクラス（`WNDCLASSEXW`）の `style` に `CS_DBLCLKS` を追加し、ダブルクリックメッセージをOSから受け取れるようにする。
    - `Window::WindowProc` にて `WM_LBUTTONDBLCLK` メッセージを捕捉する。
    - プレイリスト領域でのダブルクリック時に発火する専用コールバック `SetPlaylistDoubleClickCallback` を `Window` クラスに新設し、`WM_LBUTTONDBLCLK` 受信時にこれを呼び出す。

### 要件2:選択（フォーカス）状態の管理と描画 (Application / Widget)
    - `Application` クラスに、現在フォーカスされている項目のインデックスを保持するメンバ変数（例: `std::optional<size_t> m_focusedPlaylistIndex;`）を追加する。
    - 既存のシングルクリックコールバック（`WM_LBUTTONDOWN`経由）では再生を行わず、クリックされた項目のインデックスを `m_focusedPlaylistIndex` に設定し描画更新を行うのみとする（曲一覧・プレイリスト一覧の両モードに対応）。
    - 保持したフォーカスインデックスを `WidgetContext` などを経由して `PlaylistWidget` に渡す。
    - `PlaylistWidget::Draw` 内で、描画対象のインデックスがフォーカスインデックスと一致する場合、再生中ハイライトとは異なる色・透明度で背景を薄くハイライト表示する処理を追加する。

### 要件3:決定ロジックのダブルクリックへの移行 (Application)
    - これまでシングルクリックコールバックで実行していた決定処理（曲一覧モードでの `JumpToIndex` や、プレイリスト一覧モードでの `SwitchPlaylist`）を、新設したダブルクリックコールバック側の処理として移行する。
    - 既存の排他制御（背面UIのクリック判定スキップ等）のロジックをダブルクリック判定時にも同様に適用する。

## 3. 実装タスクリスト
## 3. 実装タスクリスト
[x] タスク1: Windowクラスへのダブルクリック対応追加
    - `Window::Initialize` の `WNDCLASSEXW.style` に `CS_DBLCLKS` を追加する。
    - `Window.h` にコールバック関数ポインタ `m_playlistDoubleClickCallback` とセッター `SetPlaylistDoubleClickCallback` を追加する。
    - `Window::WindowProc` に `WM_LBUTTONDBLCLK` の処理を追加し、プレイリスト領域内であればコールバックを呼び出し、背面の判定をスキップ（`return 0;`）する。
[x] タスク2: Applicationクラスでの状態管理とコールバック設定
    - `Application.h` にフォーカス状態を保持する変数 `m_focusedPlaylistIndex` を追加する。
    - `Application::Initialize` 等で `Window` にダブルクリックコールバックを登録する。
    - シングルクリック時の処理を「フォーカスインデックスの更新」に変更する。
    - ダブルクリック時の処理として「対象インデックスでの曲再生(`JumpToIndex`)、またはプレイリストの切り替え(`SwitchPlaylist`)」を実装する。
[x] タスク3: PlaylistWidgetでのフォーカスハイライト描画
    - `WidgetContext` 等を利用して `PlaylistWidget` にフォーカスインデックスの情報を伝達する。
    - `PlaylistWidget::Draw` にて、フォーカスされている項目の背景にハイライト（例: `D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.1f)` 等）を描画する処理を追加する。
[x] タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の Window クラスや Application クラス、WidgetContext の説明部分を、ダブルクリック対応・フォーカス管理に関する内容に更新する。

## 4. 詳細作業内容
### タスク1: Windowクラスへのダブルクリック対応追加
    - `Window.h` に `SetPlaylistDoubleClickCallback` および `m_onPlaylistDoubleClick` を定義。
    - `Window.cpp` の `Window::Initialize` で `WNDCLASSEXW` の `style` に `CS_DBLCLKS` フラグを追加。
    - `Window.cpp` に `WM_LBUTTONDBLCLK` メッセージの処理を追加し、`m_isPlaylistHovered` が true の場合に `m_onPlaylistDoubleClick` を呼び出すように変更。

### タスク2: Applicationクラスでの状態管理とコールバック設定
    - `Application.h` に `<optional>` をインクルードし、`std::optional<size_t> m_focusedPlaylistIndex` を追加。
    - `Application::Initialize` 内で `m_window.SetPlaylistDoubleClickCallback` を設定し、従来のシングルクリック時の処理（リスト切り替えや再生処理）をこちらに移動。
    - `m_window.SetPlaylistClickCallback` 内部の処理を、クリックされた項目のインデックスを `m_focusedPlaylistIndex` に設定するのみに変更。
    - `Application::ForceRender` の先頭で `m_renderer.SetFocusedPlaylistIndex` を呼び出し描画システムに状態を伝達。
    - リストのクリア時 (`ClearPlaylist`)、切り替え時 (`SwitchPlaylist`)、新規作成時 (`CreateNewPlaylist`) に `m_focusedPlaylistIndex.reset()` を呼び出しフォーカスを解除するよう変更。

### タスク3: PlaylistWidgetでのフォーカスハイライト描画
    - `Widget.h` の `WidgetContext` 構造体に `std::optional<size_t> focusedPlaylistIndex` を追加。
    - `Renderer.h`/`Renderer.cpp` に `SetFocusedPlaylistIndex` メソッドを追加し、`Render` メソッド内で `WidgetContext` に情報を渡すよう変更。
    - `Widgets.cpp` の `PlaylistWidget::Draw` 内で、プレイリスト一覧モードおよび曲一覧モードの両方において、`ctx.focusedPlaylistIndex` が現在描画中のインデックスと一致する場合に `m_playlistHighlightBrush` にて透過度 0.1f のハイライトを描画するように修正。

### タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` 内の `ForceRender`、プレイリスト連携、ホバー制御の解説部分に、`m_focusedPlaylistIndex` と `WM_LBUTTONDBLCLK` による処理の分離についての説明を追記。

## 5. HOTFIX1
### 原因・理由:
    - 曲一覧モードでは直感的にシングルクリックで即時再生される挙動が好ましく、プレイリスト一覧モードでは誤操作を防ぐためにシングルクリックでフォーカス・ダブルクリックで決定の挙動が好ましいため、クリック挙動の個別最適化が必要となった。

### 対応:
    - `Application::Initialize` のシングルクリックコールバックにおいて、`m_isPlaylistListViewMode` が false (曲一覧モード) の場合は、フォーカス更新に加えて即座にジャンプ再生処理を実行するよう変更。
    - ダブルクリックコールバックにおいて、曲一覧モードの場合は既にシングルクリックで再生が開始されているため、何も処理を行わないように変更。
