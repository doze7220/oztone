# Phase 12-2: ロゴ拡張メニューのUX改善とアフォーダンス向上 (実装計画)

本ドキュメントは、Phase 12-2のロゴ拡張メニューに関するUX改善の実装計画をまとめた作業レポートである。

## 目的
前回実装したロゴ拡張メニューの操作性・視認性の課題を解消するため、個別のホバー演出（画像・グロー）、説明テキストの動的切り替え、プレイリストとの干渉排除、およびメニュー展開中のロゴホバー状態維持を実装する。

## 設計とアプローチ (責務分離に基づく)

### 1. プレイリストホバー判定の干渉排除 (Windowクラス)
- **対象**: `Window::IsInPlaylistRegion`
- **問題**: プレイリスト左配置時、ロゴメニュー操作中に誤ってプレイリストが展開してしまう。
- **解決策**: Y座標が「ロゴと拡張メニューが存在する高さ領域」以下である場合、プレイリストのホバー判定から完全に除外する。
  - 具体的には `logicalY <= m_config->GetLogoY() + m_config->GetLogoHeight() + マージン` 等の条件を追加し、その領域では左右配置に関わらず `IsInPlaylistRegion` を false と返すように修正する。

### 2. 個別アイコンのホバー判定と状態伝達 (Window -> WidgetContext)
- **対象**: `Window` クラス、`WidgetContext`
- **設計**:
  - `Window` クラスに新しくマウスカーソルがどのロゴメニューアイコン上にあるかを判定する関数（例：`int GetLogoMenuButtonAt(int x, int y, float progress)`）を追加する。
  - `WM_MOUSEMOVE` 処理内でこの関数を呼び出し、ホバー中のインデックスを `Window` のメンバ変数 (`m_logoMenuHoveredIndex`) として保持する。
  - `Renderer::Render` の呼び出し等を通じて、このホバー中インデックス状態を `WidgetContext` に追加し (`int logoMenuHoveredIndex`)、各Widgetへ伝達する。ホバーしていない場合は `-1` となる。

### 3. 個別アイコンのホバーフィードバック演出 (LogoMenuWidget)
- **対象**: `LogoMenuWidget` (`Widgets.cpp`)
- **設計**:
  - リソース初期化時に `app_logo_back.png` を `ID2D1Bitmap` としてロードする（`LoadBitmapResource`を使用）。
  - `WidgetContext` から受け取った `logoMenuHoveredIndex` を元に、ホバーされているアイコンの背面に `app_logo_back.png` を不透明度 0.6 等で描画する。
  - さらにアイコン自体にグロー効果（`ID2D1Shadow` の色変更や描画パラメータの変更）を加え、アフォーダンスを明示する。
  - なめらかな視覚的遷移を実現するため、個別のアイコンに対するホバーフェード値（アニメーション状態）を持たせる。

### 4. アイコン説明テキストの動的表示 (LogoMenuWidget)
- **対象**: `LogoMenuWidget` (`Widgets.cpp`)
- **設計**:
  - 既存の「OZtone Menu」が1文字ずつタイピングされるアニメーション処理を廃止する。
  - 代わりに、`logoMenuHoveredIndex` に応じて以下のように表示テキストを切り替える。
    - `-1` (非ホバー時): `OZtone Menu` または空白
    - ❌ (0): `OZtoneの終了`
    - 🗑️ (1): `再生中の曲をプレイリストから削除する`
    - 📜 (2): `プレイリスト固定表示`
    - 📽️ (3): `ビジュアライザ表示切り替え`
    - 🔀 (4): `シャッフル再生ON/OFF`
  - どのアイコンにもホバーしていない状態のデフォルトテキストと、ホバー時のテキスト切り替えを美しく行う。

### 5. メニュー操作中のロゴホバー状態維持 (AppLogoWidget)
- **対象**: `AppLogoWidget` (`Widgets.cpp`)
- **設計**:
  - 現在の `AppLogoWidget::Draw` では、ロゴアイコン自身のホバー状態 (`ctx.isHovered`) に基づいて画像を切り替えている。
  - これを `ctx.isHovered || ctx.isLogoMenuHovered` の条件に変更し、ロゴ拡張メニュー全体がホバーされている間も常に `app_logo_hover.png` を描画し続けるように修正する。

## タスクリスト

- [x] `WidgetContext` に `int logoMenuHoveredIndex` を追加する。
- [x] `Window` クラスに `int GetLogoMenuButtonAt(int x, int y, float progress)` を実装する。
- [x] `Window::WindowProc` の `WM_MOUSEMOVE` 内で `GetLogoMenuButtonAt` を呼び出し、インデックスを更新して保持する。
- [x] `Application` / `Renderer` の引数パスを更新し、保持したインデックスを `WidgetContext` に設定してWidgetへ渡す。
- [x] `Window::IsInPlaylistRegion` にて、プレイリストホバー判定時にロゴ・メニュー領域（Y座標の条件）を完全に除外するロジックを追加する。
- [x] `LogoMenuWidget::CreateResources` にて `app_logo_back.png` の読み込み処理を追加する。
- [x] `LogoMenuWidget` の描画ロジックを改修し、ホバー中のアイコン背面に `app_logo_back.png` を描画し、グロー効果を付与する。
- [x] `LogoMenuWidget` のテキスト描画ロジックを改修し、タイピング演出を廃止してホバー中のアイコンに応じた説明テキストを描画する。
- [x] `AppLogoWidget::Draw` の画像切り替え判定を修正し、`isLogoMenuHovered` が true の場合もホバー画像を表示するようにする。

## 4. 詳細作業内容
1. **Window クラスの改修**:
   - `Window.h` に `m_logoMenuHoveredIndex` を追加し、`GetLogoMenuButtonAt` を宣言した。
   - `Window.cpp` にて `GetLogoMenuButtonAt` を実装し、マウス座標からどのアイコンがホバーされているか判定するロジックを追加。
   - `WM_MOUSEMOVE` 処理内で、`GetLogoMenuButtonAt` を呼び出し `m_logoMenuHoveredIndex` を更新するように修正。
   - `IsInPlaylistRegion` にて、Y座標がロゴ・メニュー領域以下である場合、プレイリスト領域判定を無効化する干渉排除ロジックを追加。
   - `m_logoMenuItems` の初期化リストに英語大文字ラベル (`EXIT`, `CLEAR PLAYLIST`, `PIN PLAYLIST`, `VISUALIZER`, `SHUFFLE`) を追加し、動的テキスト切り替えに対応させた。

4. **追加修正内容**:
   - メニュー上部の説明テキストを計画書の日本語に戻し、テキストの言語設定を `ja-jp` に再設定した。
   - ビジュアライザアイコンのホバー時に、現在のモード (`PRISM BEAT`, `HALO DUST`, `OFF`) に応じてテキストを動的に切り替えるように修正した。
   - ビジュアライザアイコンの斜線（無効化表示）判定を、`toggleState` ではなく `GetVisualizerMode() == 0` の場合にのみ描画されるように修正した。
   - アイコン右下に表示される『1』『2』のインジケーターテキストについて、専用の太字フォーマット (`m_indicatorTextFormat`) を新設した。スケーリングを正確に行うため絵文字フォントではなくテキスト用フォント (`TypingFontFamily`) を適用し、サイズをアイコンの1/4に縮小。視認性向上のため白文字＋黒フチで描画するように調整した。

2. **Renderer および Application クラスの改修**:
   - `Renderer::UpdateAnimation` と `Renderer::Render` の引数に `logoMenuHoveredIndex` を追加。
   - `Application::ForceRender` から `m_window.GetLogoMenuHoveredIndex()` の値を `Renderer` に渡すように変更。
   - `Widget.h` の `WidgetContext` に `logoMenuHoveredIndex` を追加。

3. **Widgets クラスの改修**:
   - `resource.h` に `IDI_APP_LOGO_BACK` (105) を追加。
   - `LogoMenuWidget::CreateResources` にて `app_logo_back.png` のロード処理と `m_shadowEffect` の生成処理を追加。
   - `LogoMenuWidget::Draw` にて、`ctx.logoMenuHoveredIndex` に一致するアイコンの背面に `app_logo_back.png` と影を描画し、アフォーダンスを明示。
   - `LogoMenuWidget::Draw` の説明テキスト部分をタイピングから英語大文字 (`labelText`) の表示に切り替え。
   - ビジュアライザアイコンの右下に、現在のモードに応じて『1』または『2』を描画するロジックを追加。
   - `AppLogoWidget::Draw` にて、`ctx.isLogoMenuHovered` が true のときもロゴを `app_logo_hover.png` に維持するように変更。

## 5. Hotfix: ロゴ拡張メニューのデザイン・レイアウトパラメータの外部化

ハードコードされていた以下の表示パラメータをINI設定 (`[Layout_LogoMenu]` セクション) に移行し、外部からの柔軟な調整を可能とした。

1. **ホバー背景と説明テキストの設定**:
   - `IconHoverBgAlpha`: アイコンホバー時の背景画像の不透明度 (デフォルト 0.6)
   - `DescShadowOffsetX`, `DescShadowOffsetY`, `DescShadowOpacity`: 説明テキスト専用のドロップシャドウ設定。
   - これに伴い `LogoMenuWidget` の描画処理において、シャドウ用の黒ブラシを新設しテキスト描画時に影を適用するよう修正。
2. **ビジュアライザインジケーターの設定**:
   - `VisualizerIconFontSize`: アイコン右下の状態インジケーター（1, 2）のフォントサイズ (デフォルト 12.0)
   - `VisualizerIconOffsetX`, `VisualizerIconOffsetY`: 自身のアイコン座標からの表示位置オフセット。従来はアイコンサイズベースの割合計算であったものを、直接的なオフセット加算方式に変更し位置調整を容易にした。
