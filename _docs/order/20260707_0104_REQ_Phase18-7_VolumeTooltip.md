### 作業指示書 REQ: Phase 18-8: OSDリアクションとフライテキストUX
以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-8_OSD_FlyText.md`）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
グローバルホットキー等によるバックグラウンド操作時のアフォーダンスとして、一定時間でフェードアウトする OSD (On-Screen Display) リアクションを実装する。

* **要件1: ConfigManager の拡張**
  * `[Layout_OSD]` セクションを新設する。
  * 画面中央のフライテキスト用に `OsdFontFamily`, `OsdFontSize` (デフォルト: 48.0f), `OsdTextColor` (デフォルト: "#FFFFFF"), `OsdShadowColor`, `OsdShadowOpacity` などのレイアウトパラメータと、減衰速度 `OsdFadeSpeed` (デフォルト: 1.0f) を追加し、読み書きとゲッターを実装する。

* **要件2: 状態管理とトリガー (Renderer / Application / WidgetContext)**
  * `Renderer` クラスに、OSD起動トリガーとなる `TriggerVolumeOsd()` および `TriggerFlyText(const std::wstring& text)` メソッドを新設する。
  * `Renderer` 内部にアニメーション状態 `m_osdVolumeAlpha`, `m_flyTextAlpha` (各0.0f〜1.0f) と `m_flyTextString` を追加し、トリガー時に Alpha を 1.0f にセットする。
  * `Renderer::UpdateAnimation` 内で、各 Alpha 値を `deltaTime * OsdFadeSpeed` でゼロに向けて減衰させる。
  * これらの値を `WidgetContext` に追加し、各 Widget に伝達する。
  * `Application` クラス内のグローバルホットキー・メディアキーのイベントハンドラにて、音量が変更された場合は `TriggerVolumeOsd()` を呼び出し、Z-Order変更等が行われた場合は `TriggerFlyText(L"TopMost")` 等を呼び出すようにロジックをフックする。

* **要件3: VolumeControlWidget の再利用によるスマートなOSD化**
  * 既存の `VolumeControlWidget` の描画ロジックを改修する。
  * 描画時の不透明度（ブラシや `SetOpacity` の値）を `std::max(ctx.controlAlpha, ctx.osdVolumeAlpha)` とする。
  * これにより、UIにホバーしていなくても、音量操作が行われた瞬間に既存の音量コントロールが画面下部にフワッと浮かび上がり、自然に消えるリアクションを実現する。

* **要件4: LayoutCalculator と OsdWidget の新設 (フライテキスト)**
  * `LayoutCalculator` に画面中央を基準とした `OsdLayout` のテキスト配置矩形計算ロジックを追加する。
  * `src/Widgets.h/cpp` (または独立したファイル) に `OsdWidget` クラスを新設する。
  * `OsdWidget::Draw` では、`ctx.flyTextAlpha > 0.0f` の場合にのみ、キャッシュしたテキストレイアウトを用いて `ctx.flyTextString` を画面中央に大きく描画し、`ctx.flyTextAlpha` に応じてフェードアウトさせる。
  * `Renderer` の初期化時に `OsdWidget` を生成して `m_widgets` の末尾（最前面）に登録する。

#### 【作業終了後】
1. 作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-8_OSD_FlyText.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **アーキテクチャの厳守**: Renderer に状態を持たせすぎず、情報伝達は必ず `WidgetContext` を経由すること。
* **パフォーマンス**: `OsdWidget` 内での `IDWriteTextLayout` は毎フレーム生成するのではなく、`ctx.flyTextString` が変化した時のみ再生成するキャッシュ機構を設けること。
