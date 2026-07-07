# RES:実装計画・作業レポート Phase 18-6: UIホバー時の非対称カラーフェードUX

## 1. 実装目的
再生コントロール、ロゴメニュー、プレイリストの各要素において、ホバー時に即座に色が変わり、ホバーアウト時にフェードしながら元の色に戻る非対称トランジションを持つ極上のUXを実装する。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の拡張
    - `[Layout_Window]` セクションに `HoverIconColor` (デフォルト: "#88CCFF") と `HoverFadeOutSpeed` (デフォルト: 3.0f) を追加する。
    - `[Layout_Playlist]` セクションに `PlayingItemColor` (デフォルト: "#FFA500") と `HoverItemColor` (デフォルト: "#FFFF99") を追加する。
    - HEX文字列 ("#RRGGBB" など) をパースし `D2D1::ColorF` に変換するヘルパー処理を実装する。

### 要件2: WidgetContextとホバー状態のトラッキング
    - `Window` クラス内で、マウスイベントから現在ホバー中の再生コントロールのインデックス (`playbackHoveredIndex`) やプレイリストのアイテムインデックス (`playlistHoveredItemIndex`) を取得・保持する処理を追加する。
    - `WidgetContext` を拡張し、これらのホバーインデックスを含め、各描画層 (Widget) へ伝達する。

### 要件3: 各 Widget へのアニメーション状態 (HoverAlpha) の導入
    - `PlaybackControlsWidget`, `LogoMenuWidget`, `PlaylistWidget` (ツールバーとリストアイテム) において、要素ごとのホバー進行度 (0.0f〜1.0f) を保持するデータ構造を追加する。
    - 各 Widget の `UpdateAnimation` 内で非対称な更新を行う。現在ホバー中なら即座に `1.0f` とし、それ以外は `deltaTime * HoverFadeOutSpeed` で減衰させる。

### 要件4: Lerp(線形補間)と不透明度を用いた描画ロジックの拡張
    - 各要素の描画時 (`Draw` 内) に、ベースカラーとターゲットカラーを `HoverAlpha` で線形補間 (Lerp) する処理を追加する。
    - 計算した色を共通ブラシの `SetColor()` に適用して描画を行う。パフォーマンス維持のためブラシの再生成は行わない。
    - プレイリストにおいては、通常は白ベースだが、再生中・選択中のアイテムは `PlayingItemColor` (オレンジ) をベースカラーとしてハイライト状態を維持し、ホバー時は `HoverItemColor` (薄黄色) へフェードさせる。既存の半透明背景ハイライト機能は維持する。
    - ロゴメニュー等で背面に画像ハイライト (`app_logo_back.png` 等) を描画している場合、画像の不透明度 (Opacity) にも `HoverAlpha` を乗算し、文字色フェードと完全に連動して背景画像も滑らかにフェードアウトする処理を実装する。

### 1. タスクリスト

- [x] **ConfigManager の拡張**
  - `HoverIconColor`, `HoverFadeOutSpeed`, `PlayingItemColor`, `HoverItemColor` の読み書き処理を実装済み
- [x] **WidgetContext の拡張**
  - ヘックスカラー変換ユーティリティ関数 (`ParseHexColor`) を実装済み
  - `WidgetContext` 構造体にホバー状態を伝達するメンバ（`playbackHoveredIndex`, `playlistHoveredItemIndex`, `logoMenuItems`等）を追加済み
- [x] **UpdateAnimation の実装 (各 Widget)**
  - `Window::GetPlaybackHoveredIndex`, `Window::GetPlaylistHoveredItemIndex` を新設し、マウス追跡から算出したインデックスを取得
  - `Application::ForceRender` にてプレイリストのホバー位置を論理座標から算出し、Renderer 経由で各ウィジェットへ伝達
  - `PlaybackControlsWidget`: `m_hoverAlpha[5]` 配列を用意し、対象インデックスごとに Lerp 値を更新
  - `LogoMenuWidget`: 動的な要素数に合わせて `std::vector<float> m_hoverAlpha` を用意し更新
  - `PlaylistWidget`: トラック数の多さを考慮し `std::unordered_map<int, float> m_hoverAlpha` にて対象アイテムのみ更新
- [x] **Draw 処理へのアルファ値適用 (各 Widget)**
  - `PlaybackControlsWidget`: 各ボタンごとに `GetBlendedColor` でブレンド色を計算し、`m_controlBrush->SetColor()` で適用
  - `LogoMenuWidget`: アイコンとインジケーターテキスト描画前にブラシ色を `GetBlendedColor` で補間済みの色に設定
  - `PlaylistWidget`: プレイリストのテキスト描画ループ内にて、`m_textBrush->SetColor()` を使い、再生中カラーやホバーカラーを動的にブレンドして描画
- [x] **アーキテクチャドキュメントの更新**
  - （次ステップで `PROJECT_ARCHITECTURE.md` を更新する）

## 4. 詳細作業内容
### タスク1: ConfigManager の拡張
- `ConfigManager.h/cpp` にて、INIファイルから `HoverIconColor`, `HoverFadeOutSpeed`, `PlayingItemColor`, `HoverItemColor` を読み書きする処理を追加した。
- `GetHoverIconColor`, `GetHoverFadeOutSpeed`, `GetPlayingItemColor`, `GetHoverItemColor` の各ゲッターメソッドを実装し、外部のレンダリング処理等で設定値を参照可能とした。

### タスク2: WidgetContext とホバー状態のトラッキング処理の追加
- `WidgetContext.h` にて、ヘックスカラー文字列を `D2D1_COLOR_F` に変換するユーティリティ関数 `ParseHexColor` を実装した。
- `WidgetContext` 構造体に、ホバー対象を特定するための `playbackHoveredIndex`, `playlistHoveredItemIndex` およびロゴメニューのアイテム情報を渡す `logoMenuItems` ポインタを追加した。
- `Window.h/cpp` にてマウス座標の判定を行い、`m_playbackHoveredIndex` や `m_playlistHoveredItemIndex` をリアルタイムでトラッキングして保持する処理を組み込んだ。
- `Application::ForceRender` および `Renderer::UpdateAnimation` のシグネチャを拡張し、トラッキングしたホバーインデックス情報を毎フレーム `WidgetContext` へ供給するようにした。

### タスク3: PlaybackControlsWidget のアニメーション対応
- `PlaybackControlsWidget.h` に、各ボタンのホバーアルファ値を保持する `m_hoverAlpha[5]` を追加。
- `UpdateAnimation` にて、`ctx.playbackHoveredIndex` と照合し、対象ボタンのアルファ値はフェードイン、それ以外はフェードアウト（非対称速度）させる処理を実装した。
- `Draw` にて、`m_hoverAlpha` を用いてベースカラー（通常時の色）からホバー色（`HoverIconColor`）へと補間（Lerp）する `GetBlendedColor` ラムダ関数を定義し、各ボタンの描画時に `m_controlBrush->SetColor` で動的に色を合成して適用した。

### タスク4: LogoMenuWidget のアニメーション対応
- `LogoMenuWidget.h` に、可変長なメニュー項目に対応するため `std::vector<float> m_hoverAlpha` を追加。
- `UpdateAnimation` にて、`ctx.logoMenuItems` の要素数に合わせてリサイズしつつ、`ctx.logoMenuHoveredIndex` と照合してフェード状態を更新する処理を実装した。
- `Draw` にて、アイコン用の白（不透明/半透明）ベースカラーからホバー色（`HoverIconColor`）へ Lerp 補間し、アイコンおよびインジケーターテキストを描画する際にブラシカラーとして適用した。

### タスク5: PlaylistWidget のアニメーション対応
- `PlaylistWidget.h` に、大量のトラック数を考慮し、ホバー状態が有効なアイテムのみをトラッキングする `std::unordered_map<int, float> m_hoverAlpha` を追加。
- `UpdateAnimation` にて、不要になったホバー要素はマップから削除しつつ、現在の `ctx.playlistHoveredItemIndex` に対してフェードインを適用する効率的な更新処理を実装した。
- `Draw` 内のトラック描画ループにて、現在再生中のアイテム（`isPlaying`）であれば `PlayingItemColor` をベースカラーに、通常はテキストカラーをベースカラーにし、そこから `HoverItemColor` へ Lerp 補間する `GetBlendedTextColor` 関数を定義。タイトルなどのテキスト描画に適用した。

### タスク6: PROJECT_ARCHITECTURE.md の更新
- `PROJECT_ARCHITECTURE.md` の「各具象 Widget 実装」セクションにおいて、`LogoMenuWidget`, `PlaybackControlsWidget`, `PlaylistWidget` の説明文に、非対称フェードによる独立した色合成UXの実装と、関連する設定値（`HoverIconColor`, `HoverItemColor`, `PlayingItemColor` 等）についての詳細を追記した。
