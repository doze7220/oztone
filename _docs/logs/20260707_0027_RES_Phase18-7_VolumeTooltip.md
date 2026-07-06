# RES:実装計画・作業レポート Phase 18-7: 音量ホバー時のツールチップ（吹き出し）UX

## 1. 実装目的
音量コントロールへのマウスホバー時、操作方法のアフォーダンスとして「マウスホイール操作を示すアイコン付きの吹き出し（ツールチップ）」をフワッと上部に表示させるUXを実装する。これにより、UIの使い勝手とビジュアルの高級感を向上させる。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の拡張
- `OZtone.ini` の `[Layout_VolumeControl]` セクションに吹き出し用パラメータを追加する。
- 追加する設定項目: `TooltipText` (デフォルト "🖱️↕"), `TooltipFontSize`, `TooltipFontFamily`, `TooltipOffsetY`, `TooltipBgColor`, `TooltipBgOpacity`, `TooltipTextColor`。
- 上記のゲッター・セッターを実装し、デフォルト設定保存処理（`SaveDefaultSettings`）に反映する。

### 要件2: ホバー状態の精密なトラッキング (Window / WidgetContext)
- `Window::WindowProc` 内のマウスイベント処理などにおいて、`IsInVolumeControlRegion` 等を利用して音量コントロールのホバー状態を追跡する（既存の全体のホバー状態と区別、または同時に管理）。
- `WidgetContext` 構造体に `bool isVolumeHovered` を追加し、描画更新時に `Renderer::UpdateAnimation` や `Renderer::Render` へこの状態を各Widgetに伝達するルートを構築する。

### 要件3: 非対称フェードアニメーションの導入 (VolumeControlWidget)
- `VolumeControlWidget` にツールチップ専用の不透明度変数 `m_tooltipAlpha` (0.0f〜1.0f) を追加する。
- `UpdateAnimation` メソッド内で、`ctx.isVolumeHovered` が true の場合は `m_tooltipAlpha` を即座に `1.0f` に設定（パキッと表示）。
- false の場合は、`deltaTime` を用いて一定の速度で `0.0f` に向かって減衰させる（フワッと消える非対称フェード）。

### 要件4: LayoutCalculator と吹き出しの描画 (VolumeControlWidget / LayoutCalculator)
- `LayoutCalculator` にツールチップ表示領域（吹き出し背景サイズおよび位置）とテキスト領域を計算するロジックを統合。
- `VolumeControlWidget::CreateResources` または同等のリソース構築部において、ツールチップの吹き出し形状（角丸矩形と下向き三角形を結合した形状）を持つ `ID2D1PathGeometry`、およびテキスト描画用の `IDWriteTextLayout` を生成しキャッシュする。
- `VolumeControlWidget::Draw` において、`m_tooltipAlpha > 0.0f` の場合に吹き出しジオメトリおよびテキストを描画する。この際、ブラシの不透明度には `m_tooltipAlpha * ctx.controlAlpha` などを適用し、全体のUIフェードと連動しつつなめらかなフェードアウトを実現する。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManagerの拡張
    - `ConfigManager.h/cpp` にツールチップ関連のパラメータを追加。
    - ゲッター関数、ロード処理、およびデフォルト設定保存処理の実装。
- [x] タスク2: WidgetContextとホバー状態伝達の拡張
    - `WidgetContext.h` に `isVolumeHovered` を追加。
    - `Application::ForceRender` と `Renderer::Render`, `Renderer::UpdateAnimation` などへホバー状態を伝達する。
- [x] タスク3: 吹き出しジオメトリ生成とテキストレイアウトのキャッシュ (VolumeControlWidget / LayoutCalculator)
    - `VolumeControlWidget.h/cpp` に `m_tooltipAlpha` とキャッシュ用メンバを追加。
    - `CreateResources` および `UpdateLayout` 等で `ID2D1PathGeometry` を用いた吹き出し形状の構築と、`IDWriteTextLayout` の作成処理を追加。
- [x] タスク4: 非対称フェードアニメーションと描画処理の実装 (VolumeControlWidget)
    - `UpdateAnimation` で `ctx.isVolumeHovered` に応じた非対称フェードロジックを実装。
    - `Draw` でツールチップを描画する処理（背景ジオメトリ描画、テキスト描画）を実装。
- [x] タスク5: PROJECT_ARCHITECTURE.md の更新
    - 今回の変更内容（音量コントロールでのホバー時のツールチップ表示UX）をドキュメントに追記。

## 4. 詳細作業内容
### タスク1: ConfigManagerの拡張
- `ConfigManager.h/cpp`に`TooltipText`等のパラメータ追加およびゲッター・ロード・保存処理を実装済み。
### タスク2: WidgetContextとホバー状態伝達の拡張
- `Window.h/cpp`で`m_isVolumeHovered`をトラッキングし、`Application.cpp`から`Renderer`経由で`WidgetContext`へ伝達するフローを実装。
### タスク3: 吹き出しジオメトリ生成とテキストレイアウトのキャッシュ
- `LayoutCalculator.h/cpp`にツールチップ用レイアウトを追加。
- `VolumeControlWidget::CreateResources`で`m_tooltipGeometry`と`m_tooltipTextLayout`等をキャッシュとして生成。
### タスク4: 非対称フェードアニメーションと描画処理の実装
- `VolumeControlWidget::UpdateAnimation`に`m_tooltipAlpha`のフェードロジックを実装。
- `VolumeControlWidget::Draw`にて、キャッシュ済みのジオメトリとテキストレイアウトを`layout.tooltipBoxX`等の座標に基づいて透過度付きで描画。
### タスク5: PROJECT_ARCHITECTURE.md の更新
- `PROJECT_ARCHITECTURE.md`の下部UIに関する記述へ、音量ツールチップの仕様を追記。

## 5. HOTFIX1
### 原因・理由:
- (なし)

### 対応:
- (なし)
