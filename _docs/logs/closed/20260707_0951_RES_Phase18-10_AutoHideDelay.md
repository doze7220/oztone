# RES:実装計画・作業レポート Phase 18-10: 隠れるUIの共通離脱ディレイUX

## 1. 実装目的
「必要な時だけ現れる忍者UI」の操作性を向上・統一するため、ロゴ拡張メニュー、プレイリスト、下部コントロール（シークバー等）の「隠れるUI」すべてに対し、マウスホバーが外れても指定時間展開を維持する「離脱ディレイ（猶予時間）」の仕組みを実装する。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の拡張
    - `[Layout_LogoMenu]` セクションに `MenuLeaveDelay` (float型, デフォルト: 3.0f) を追加する。
    - `[Layout_Playlist]` セクションに `PlaylistLeaveDelay` (float型, デフォルト: 3.0f) を追加する。
    - `[Layout_Window]` セクションに `ControlLeaveDelay` (float型, デフォルト: 3.0f) を追加する。
    - それぞれのゲッターメソッドを `ConfigManager.h` に追加し、`ConfigManager.cpp` にてINIからの読み込みと保存の処理を実装する。

### 要件2: UpdateAnimation における状態管理への遅延タイマー導入
    - `Renderer.h` (および必要に応じて各Widget) にタイマー変数 `m_logoMenuLeaveTimer`, `m_playlistLeaveTimer`, `m_controlLeaveTimer` (各float, 初期値0.0f) を追加する。
    - `UpdateAnimation` 処理内での各UIの更新ロジックを以下のように共通化して改修する。
        1. 現在ホバー中 の場合：該当するタイマーを `ConfigManager` から取得したディレイ値にリセットし、UIを展開状態へ進める（アルファ値加算やスライドイン）。
        2. ホバーから外れた 場合：直ちに格納処理を始めるのではなく、タイマーを `deltaTime` で減算する。
        3. タイマーが 0.0f 以下 になった場合に初めて、UIを格納する（アルファ値減算やスライドアウト）処理へ移行する。
    - プレイリストが「ピン留め (Pinned)」状態 (`ConfigManager::GetIsPlaylistPinned()`) の場合は、タイマーの減少・判定をスキップし常に展開状態を維持するように既存ロジックと統合する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManager の拡張
    - `ConfigManager.h` に変数とゲッターを追加
    - `ConfigManager.cpp` にデフォルト値設定、読み書きロジックを追加
[x] タスク2: 状態管理用タイマー変数の追加
    - `Renderer.h` または各Widgetクラスに `m_controlLeaveTimer`, `m_logoMenuLeaveTimer`, `m_playlistLeaveTimer` を追加
[x] タスク3: UpdateAnimationへのタイマーロジック組み込み
    - コントロール、ロゴ拡張メニュー、プレイリストそれぞれのホバー判定処理を改修し、タイマー減算と0.0f以下でのみ格納アニメーションを行うように修正
    - プレイリストのピン留め状態を考慮
[x] タスク4: ドキュメントの更新
    - 実装完了後、`PROJECT_ARCHITECTURE.md` に離脱ディレイの仕様を追加

## 4. 詳細作業内容
### タスク1: ConfigManager の拡張
    - `ConfigManager.h` および `ConfigManager.cpp` を修正し、`m_menuLeaveDelay`, `m_playlistLeaveDelay`, `m_controlLeaveDelay` を追加しました。INIファイルからの読み込み処理と初期値（3.0f）を設定しました。
### タスク2: 状態管理用タイマー変数の追加
    - `Renderer.h` に `m_controlLeaveTimer` を追加しました。
    - `LogoMenuWidget.h` に `m_logoMenuLeaveTimer` を追加しました。
    - `PlaylistWidget.h` に `m_playlistLeaveTimer` を追加しました。
### タスク3: UpdateAnimationへのタイマーロジック組み込み
    - `Renderer.cpp` の `UpdateAnimation` にて、再生・音量コントロールに関する離脱ディレイ処理（`m_controlLeaveTimer`）を組み込みました。
    - `LogoMenuWidget.cpp` の `UpdateAnimation` にて、ロゴ拡張メニューに関する離脱ディレイ処理（`m_logoMenuLeaveTimer`）を組み込みました。
    - `PlaylistWidget.cpp` の `UpdateAnimation` にて、プレイリストに関する離脱ディレイ処理（`m_playlistLeaveTimer`）を組み込みました。ピン留め状態（`isPinned`）時は常に展開状態を維持し、タイマーをリセットするように既存ロジックと統合しました。
### タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` のUI仕様方針のセクションに「隠れるUIの共通離脱ディレイ」の項目を追記しました。

## 5. HOTFIX 1
### 原因・理由: 
    - Phase 18-10 で `UpdateAnimation` にディレイを導入したことで、UI自体はディレイで展開状態を維持するようになったが、`Window::IsInPlaylistRegion` 等のホバー当たり判定領域の拡張が、純粋な `m_isPlaylistHovered` 等のフラグにのみ依存していたため、離脱ディレイ中にホバー判定領域が縮小し、再ホバーを受け付けなくなる問題があった。
### 対応:
    - `WidgetContext.h` に `outIsPlaylistExpanded`, `outIsLogoMenuExpanded` ポインタを追加し、描画側（`PlaylistWidget`, `LogoMenuWidget`）の `UpdateAnimation` 内で、アニメーションの現在状態を基に実際の展開維持状態を出力するように修正。
    - `Application::ForceRender` の描画ループ内でポインタを経由して状態を取得し、`Window` クラスの新しいメンバ変数 `m_isPlaylistExpanded`, `m_isLogoMenuExpanded` にフィードバックする仕組みを構築。
    - `Window::IsInPlaylistRegion` および `Window` のメッセージループのホバー判定ロジックを修正し、`m_isPlaylistHovered` だけでなく `m_isPlaylistExpanded` も考慮して当たり判定領域を維持するように改修。
