# RES:実装計画・作業レポート Phase 18-11: 極上のボタンリアクションとレーダー走査線UX

## 1. 実装目的
OZtoneの「極上の手触り」をさらに強化するため、各UI要素をクリックした際の視覚的なリアクション（アニメーション）を実装する。これにより、ユーザーの操作に対する即時かつ高品質なフィードバックを提供する。

## 2. アーキテクチャ設計
### 要件1: ワンショット・クリック検知機構の導入
    - `Window` クラスにてクリックイベント（`WM_LBUTTONDOWN` または `WM_LBUTTONDBLCLK`）を捕捉し、対象となるUI要素（プレイリストのインデックス、ロゴメニューのID、再生コントロールのボタン種別など）を特定する。
    - これらのクリック情報を1フレームだけ保持し、`Application::ForceRender` 呼び出し時に引数として追加することで、毎フレーム構築される `WidgetContext` へ「今クリックされた（ワンショット）」状態として伝達する。
    - 各Widgetは自身の `UpdateAnimation` 内で `WidgetContext` を参照し、自身へのクリックイベントであればアニメーションの進行度（0.0fなど）をリセットし開始フラグを立てる。

### 要件2: アイコン波紋（残像広がり）エフェクト
    - 対象: `AppLogoWidget`, `LogoMenuWidget`, `PlaybackControlsWidget`
    - 各Widgetのメンバ変数に波紋アニメーションの進行度と状態を管理する変数を追加する。
    - `UpdateAnimation` にて、進行度を更新（例: 0.0fから1.0fへ進行）する。
    - `Draw` にて、通常アイコンの描画直後に波紋効果を描画する。中心を基準とした `D2D1::Matrix3x2F::Scale` (進行度に応じて1.0倍〜1.5倍程度に拡大) を適用し、不透明度をフェードアウト（例: `0.5f * (1.0f - progress)`）させて元の画像と同形状の残像を描画する。

### 要件3: プレイリストのレーダー走査線＆再生色染めエフェクト
    - 対象: `PlaylistWidget`
    - `PlaylistWidget` のアイテムごとにクリックアニメーションの進行状態（X座標または進行度）を管理する状態を追加。
    - 曲がクリックされた際、左端から右端へ向けて進行する走査線（X座標）のアニメーションを `UpdateAnimation` で処理する。
    - `Draw` での描画順序:
        1. 通常の色でテキストを描画する。
        2. 走査線の現在位置から左側に向けて、透明へとフェードする線形グラデーション（LinearGradientBrush）を用いて「レーダーの尾」の矩形を描画する。
        3. 走査線位置より左側の領域のみを `PushAxisAlignedClip` でクリッピングし、その領域内においてのみ PlayingItemColor の単色ブラシでテキストを再描画（上書き）する。
        4. 最後に `PopAxisAlignedClip` でクリッピングを解除する。これにより、走査線が通過した部分のみが再生色に染まるサイバーな演出を実現する。

## 3. 実装タスクリスト
[x] タスク1: ワンショット・クリック検知機構の導入
    - `WidgetContext` にクリック情報を保持する変数を追加。
    - `Application` の `ForceRender`、`Renderer` の `UpdateAnimation` にクリック情報を渡すよう引数を変更。
    - `Window` でクリック状態を検知し、`ForceRender` に渡して1フレーム後にクリアする処理を追加。
[x] タスク2: アイコン波紋（残像広がり）エフェクトの実装 (AppLogoWidget, LogoMenuWidget)
    - `AppLogoWidget` に波紋アニメーション管理変数を追加し、`UpdateAnimation`、`Draw` を修正。
    - `LogoMenuWidget` に各メニューアイテムごとの波紋アニメーション管理変数を追加し、`UpdateAnimation`、`Draw` を修正。
[x] タスク3: アイコン波紋エフェクトの実装 (PlaybackControlsWidget)
    - `PlaybackControlsWidget` に各ボタンごとの波紋アニメーション管理変数を追加し、`UpdateAnimation`、`Draw` を修正。
[x] タスク4: プレイリストのレーダー走査線＆再生色染めエフェクトの実装 (PlaylistWidget)
    - `PlaylistWidget` にクリックされたアイテムと進行度を管理する変数を追加。
    - `UpdateAnimation` にて走査線の進行を計算する処理を追加。
    - `Draw` にて、通常テキスト描画、グラデーションの尾の描画、およびクリッピングを利用した再生色テキストの上書き描画を実装。
[x] タスク5: アーキテクチャドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` のUI/ビジュアル仕様方針およびクリック・アニメーション管理の仕組みについて追記・更新を行う。

## 4. 詳細作業内容
### タスク1: ワンショット・クリック検知機構の導入
    - `WidgetContext`に`isLogoClicked`等の変数を追加し、`Window::WindowProc`から`Renderer`経由で伝達する仕組みを構築した。
### タスク2: アイコン波紋（残像広がり）エフェクトの実装 (AppLogoWidget, LogoMenuWidget)
    - `m_isRippling`と`m_rippleProgress`を各Widgetに導入。クリック時にスケール拡大とフェードアウトを行う波紋エフェクトを実装した。
### タスク3: アイコン波紋エフェクトの実装 (PlaybackControlsWidget)
    - 各再生ボタン（Prev, SkipBack, Play, SkipFwd, Next）に対して独立した波紋エフェクトを描画する`DrawRipple`ラムダ関数を導入した。
### タスク4: プレイリストのレーダー走査線＆再生色染めエフェクトの実装 (PlaylistWidget)
    - 仕様変更要件に従い、クリック時ではなく「`currentTrackIndex` が切り替わった瞬間」をトリガーとして走査線アニメーションを開始するように実装した。`ID2D1LinearGradientBrush`を用いてレーダーの尾を描画し、クリッピングを用いてテキストを再生色に染め上げる処理を実装した。
### タスク5: アーキテクチャドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` 内の `WidgetContext` の説明に「ワンショット・クリック検知とアニメーション機構」の項目を追加し、波紋エフェクトやイベント伝播の仕組みを追記した。

### Hotfix: 走査線の描画領域とアニメーション終端の修正
    - 走査線のグラデーション矩形およびテキスト色染めのクリッピング範囲を、タイトル領域(`titleRect`)ベースからアイテム全体のハイライト領域(`hlRect`)ベースに修正し、上下への描画はみ出しを防止した。
    - 走査線アニメーションの現在位置（`currentX`）の計算にレーダーの尾の長さ（`tailLength`）を加味させ、アニメーション終端で残像も含めて完全に右へ通り抜けて消えるように修正した。
    - 同時にエフェクト全体を `hlRect` の幅でクリッピング（`PushAxisAlignedClip`）することで、プレイリストUIの領域外（左配置時のメインウィンドウ側など）への描画はみ出しを防止した。
