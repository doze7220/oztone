### 作業指示書 REQ: Phase 18-6: UIホバー時の非対称カラーフェードUX
以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-6_HoverColorFade.md`）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
再生コントロール、ロゴメニュー、プレイリストの各要素に対し、「ホバー時は瞬時に色が変わり、ホバーアウト時はフェードしながら元の色に戻る」という非対称トランジションを持つ極上のUXを実装する。

* **要件1: ConfigManager の拡張**
  * `[Layout_Window]` セクションに `HoverIconColor` (デフォルト: "#88CCFF") と `HoverFadeOutSpeed` (デフォルト: 3.0f, ※1秒間に減衰する量) を追加。
  * `[Layout_Playlist]` セクションに `PlayingItemColor` (デフォルト: "#FFA500") と `HoverItemColor` (デフォルト: "#FFFF99") を追加。
  * HEX文字列から D2D1::ColorF への変換処理を用意すること。

* **要件2: WidgetContextとホバー状態のトラッキング**
  * `Window` 側でマウスイベント（WM_MOUSEMOVE 等）から現在ホバー中の再生コントロールのインデックス (`playbackHoveredIndex`) やプレイリストのアイテムインデックス (`playlistHoveredItemIndex`) を適切に取得・保持する。
  * それらを `WidgetContext` に追加し、描画層 (Widget) へ伝達する。

* **要件3: 各 Widget へのアニメーション状態 (HoverAlpha) の導入**
  * `PlaybackControlsWidget`, `LogoMenuWidget`, `PlaylistWidget` (ツールバーとリストアイテム) において、要素ごとのホバー進行度 (0.0f〜1.0f) を保持する配列やMapを追加する。
  * 各 Widget の `UpdateAnimation` 内で非対称な更新を行う。ホバー中なら `1.0f` に即時設定し、それ以外は `deltaTime * HoverFadeOutSpeed` で減衰させる。

* **要件4: Lerp(線形補間)と不透明度を用いた描画ロジックの拡張**
  * 各要素の描画直前（`Draw` 内）に、ベースカラーとターゲットカラーを `HoverAlpha` で線形補間 (Lerp) する。
  * 計算した色を共通ブラシの `SetColor()` に適用して描画を行う。リソースの再生成は行わないこと。
  * プレイリストにおいては、通常は白ベースだが、「再生中・選択中」のアイテムは `PlayingItemColor` (オレンジ) をベースカラーとしてハイライト状態を維持しつつ、ホバー時は `HoverItemColor` (薄黄色) へフェードするように実装すること。既存の半透明背景のハイライト機能は維持すること。
  * **【重要】ロゴメニュー等において背面に画像ハイライト（app_logo_back.png等）を描画している場合、その画像の描画不透明度（Opacity）に対しても `HoverAlpha` を乗算し、文字色のフェードと完全に連動して背景画像も滑らかにフェードアウトして残像のように消えていく処理を実装すること。**

#### 【作業終了後】
1. 作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-6_HoverColorFade.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **パフォーマンスの維持**: 描画ループ内でブラシの再生成等は行わないこと。必ず `SetColor` を用いること。
* **アーキテクチャの厳守**: Renderer に状態を持たせず、各 Widget の `UpdateAnimation` で状態を管理し、`Draw` では描画のみを行うこと。
