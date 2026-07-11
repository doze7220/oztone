# RES:実装計画・作業レポート Phase X-X: Hotfix 背景クリックによるUIの能動的フェードアウト

## 1. 実装目的
UI要素が何もない背景部分を左クリック、または右クリックした際に、ホバー展開中（離脱ディレイ中）のUI（プレイリスト、ロゴ拡張メニュー、下部コントロール等）の格納アニメーションを即座に開始させることで、より直感的な操作感とUXの向上を図る。

## 2. アーキテクチャ設計
### 要件1: 背景クリック検知とコールバック連携
    - Windowクラス側で背景左クリック（HTCAPTION送出直前）と右クリック（フレーミング開始直前）のタイミングでコールバックを発火。
    - ApplicationクラスのInitialize処理にて、Window側で発火したコールバックからRendererの格納ディレイ初期化メソッドを呼び出す。

### 要件2: ホバーディレイの強制クリア
    - IWidgetインターフェースに `ForceClearHoverDelay()` 仮想関数を追加。
    - PlaylistWidget, LogoMenuWidget にて同関数をオーバーライドし、固有のディレイタイマーを即座に `0.0f` にリセットする。
    - Rendererクラスに `ForceClearHoverDelays()` メソッドを追加し、自身が管理するディレイタイマーのリセットおよび各Widgetへの伝播を行う。

## 3. 実装タスクリスト
[x] タスク1: IWidgetへのインターフェース追加と各Widgetでの実装
    - `Widget.h` に `ForceClearHoverDelay()` 仮想関数を追加。
    - `PlaylistWidget` および `LogoMenuWidget` のヘッダーおよび実装部にタイマーリセット処理を追加。
[x] タスク2: Rendererクラスへの強制クリアメソッドの追加
    - `Renderer.h` / `Renderer.cpp` に `ForceClearHoverDelays()` を追加。
    - 内部で `m_controlLeaveTimer` などを 0.0f にし、`m_widgets` のすべてに対して強制クリアを呼び出す処理を実装。
[x] タスク3: Windowクラスでの背景クリック検知とコールバック実装
    - `Window.h` に `m_onBackgroundClickCallback` コールバックとセッターを追加。
    - `Window.cpp` の `WM_LBUTTONDOWN` （ドラッグ移動前）および `WM_RBUTTONDOWN` （フレーミング前）にてコールバックを発火するよう修正。
[x] タスク4: Applicationクラスでの連携処理の実装
    - `Application.cpp` の `Initialize` 内で `SetBackgroundClickCallback` を登録し、`m_renderer.ForceClearHoverDelays()` を実行するラムダ式を設定。

## 4. 詳細作業内容
### タスク1～4: 実装実行
    - IWidgetおよび各Widgetの実装部にてタイマーをリセットする仕組みを構築。
    - RendererおよびWindowの処理をアップデートし、Applicationにて両者を接続。
    - 全て滞りなく実装を完了した。
