### 作業指示書 REQ: Phase 18-7: 音量ホバー時のツールチップ（吹き出し）UX
以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-7_VolumeTooltip.md`）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
音量コントロール（VolumeControlWidget）へのマウスホバー時、操作方法のアフォーダンスとして「マウスホイール操作を示すアイコン付きの吹き出し（ツールチップ）」をフワッと上部に表示させる極上のUXを実装する。

* **要件1: ConfigManager の拡張**
  * `[Layout_VolumeControl]` セクションに以下の吹き出し用パラメータを追加する。
    * `TooltipText` (デフォルト: "🖱️↕" )
    * `TooltipFontSize`, `TooltipFontFamily`
    * `TooltipOffsetY` (ボリュームアイコンからの上方向への距離)
    * `TooltipBgColor`, `TooltipBgOpacity`, `TooltipTextColor`

* **要件2: ホバー状態の精密なトラッキング (Window / WidgetContext)**
  * 現在の `Window` クラスにある `IsInVolumeControlRegion` を活用し、現在マウスが音量コントロール領域にいるかどうかのフラグを追跡する。
  * この状態を `WidgetContext` に `isVolumeHovered` として追加し、Rendererから伝達する。

* **要件3: 非対称フェードアニメーションの導入 (VolumeControlWidget)**
  * `VolumeControlWidget` に吹き出し専用のアルファ値 `m_tooltipAlpha` (0.0f〜1.0f) を追加。
  * `UpdateAnimation` 内で、`ctx.isVolumeHovered` が true なら `1.0f` に即時設定（パキッと現れる）、false なら一定速度で減衰（フワッと消える）させる非対称フェードを実装する。

* **要件4: LayoutCalculator と吹き出しの描画 (VolumeControlWidget / LayoutCalculator)**
  * `LayoutCalculator` に吹き出しの矩形やテキストレイアウトを計算する処理を追加。
  * `VolumeControlWidget::Draw` にて、`m_tooltipAlpha > 0.0f` の場合のみ以下を描画する。
    1. キャッシュした `ID2D1PathGeometry` 等を用いて、角丸矩形の下部中央に「下向きの三角形（しっぽ）」を結合した美しい吹き出しの背景（面）を描画する。
    2. 吹き出しの中に、`TooltipText` の文字列を中央揃えで描画する。
  * この時、描画ブラシの `SetOpacity` に `m_tooltipAlpha`（および全体フェードの `ctx.controlAlpha` の乗算値）を適用し、なめらかなフェードアウトを実現する。

#### 【作業終了後】
1. 作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-7_VolumeTooltip.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **アーキテクチャの厳守**: Renderer に状態を持たせず、`VolumeControlWidget` 内部で自己完結させること。描画ループ内での動的リソース（ブラシやレイアウト）の生成は行わないこと。
