### REQ: Phase 10-1: TRACK NOの移動とフォーマット変更

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase10-1_TrackNoMove.md`）として新規作成する。
2. 作業計画をタスクリストに分割し、作業レポート末尾に記載する。
3. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、ユーザーの承認を待つこと。

---

#### 【実装要件】
本タスクの目的は、プレイリストウィジェットに属していた「トラック番号表示（TRACK XXX/XXX）」を再生中曲情報ウィジェットへ移動させ、文字フォーマットを数字のみ（XXX/XXX）に変更することである。

* **要件1: 設定の移行 (ConfigManager)**
  * `[Layout_Playlist]` セクションに存在している `TrackCount...` 系の設定項目（FontFamily, FontSize, LetterSpacing, Shadow関連等）を `[Layout_NowPlaying]` セクションへ移動する。
  * `TrackCountRightOffset`, `TrackCountBottomOffset` を廃止し、左上基準の `TrackCountOffsetX`, `TrackCountOffsetY`（デフォルト値は任意で良い）へ変更する。
* **要件2: レイアウト計算の移行 (LayoutCalculator)**
  * `LayoutCalculator` にて、トラック番号の矩形計算を `PlaylistLayout` から `TrackInfoLayout` の計算へ移動する。
  * 新しい `TrackCountOffsetX/Y` を用いて、ウィンドウまたは要素の左上を基準とした絶対座標で矩形を算出する。
* **要件3: 描画とロジックの移行 (Widgets)**
  * `PlaylistWidget` からトラック番号に関連する変数（`m_lastCurrentTrackIndex` 等）および生成・描画処理を完全に削除する。
  * `TrackInfoWidget` にそれらを追加し、テキスト生成時のフォーマットを `TRACK %zu/%zu` から `%zu/%zu` （数字のみ）へと変更する。
  * テキストのアライメントを右寄せ（TRAILING）から左寄せ（LEADING）に変更する。

#### 【対象ファイル】
* `src/ConfigManager.h`, `src/ConfigManager.cpp`
* `src/LayoutCalculator.h`, `src/LayoutCalculator.cpp`
* `src/Widgets.h`, `src/Widgets.cpp`

#### 【絶対遵守ルール (Constraints)】
* **スコープの厳守**: 今回はトラック番号表示の移動とフォーマット変更のみを行う。
* **アーキテクチャの絶対制約**: `Renderer` が状態を持たないこと、ウィジェット間の情報伝達は `WidgetContext` を経由するルールの維持。

#### 【出力成果物の構成】
AIが作成する作業レポート（RESファイル）は以下の構成とすること。
1. **アーキテクチャの絶対制約** (後続の作業AIへの念押し)
2. **実装方針・手順** (対象ファイルと具体的な変更内容の概要)
3. **タスクリスト** (すべて「未対応 [ ]」のチェックボックスを用意して分割すること)
