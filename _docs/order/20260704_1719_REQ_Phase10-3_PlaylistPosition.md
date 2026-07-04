### REQ: Phase 10-3: プレイリストの配置（左右）オプション化

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase10-3_PlaylistPosition.md`）として新規作成する。
2. 作業計画をタスクリストに分割し、作業レポート末尾に記載する。
3. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、ユーザーの承認を待つこと。

---

#### 【実装要件】
本タスクの目的は、現在ウィンドウ右端固定となっているプレイリストUIを、INIファイルの設定によって「左端」または「右端」のどちらからでもスライドインできるように改修することである。

* **要件1: 設定の拡張 (ConfigManager)**
  * `[Layout_Playlist]` セクションに `PlaylistPosition` (整数型) を追加する。
  * `0`: 左端 (Left), `1`: 右端 (Right: デフォルト) とする。
  * 既存の `PlaylistGripRightOffset` は左右共通で使えるよう `PlaylistGripOffset` に名称を変更するか、そのまま流用する方針を計画書に記載すること。
* **要件2: ホバー判定の動的化 (Window)**
  * `IsInPlaylistRegion` メソッド内にて、ConfigManager の `PlaylistPosition` を評価し、左端設定時は「X座標が0〜PlaylistHoverWidth（展開時はPlaylistWidth）の範囲」、右端設定時は「既存の右端判定」となるように条件分岐を実装する。
* **要件3: レイアウト計算の動的化 (LayoutCalculator)**
  * `CalculatePlaylistLayout` にて、`PlaylistPosition` に応じてプレイリストの全体の描画矩形（`baseRect`）とクリッピング領域を算出する。
  * 左端設定時は、画面外（左側）から `m_playlistSlideX` に応じてスライドインしてくるような座標計算に変更すること。
* **要件4: 描画の適応 (Widgets/Renderer)**
  * アフォーダンス（グリップ）の描画において、左端設定時はリストの「右側」にグリップ線を描画し、矢印の向きを「▶（右向き）」にする。右端設定時は従来通り「◀（左向き）」とする。

#### 【対象ファイル】
* `src/ConfigManager.h`, `src/ConfigManager.cpp`
* `src/Window.cpp`
* `src/LayoutCalculator.cpp`
* `src/Widgets.cpp` (または `Renderer.cpp` の初期化部分)

#### 【絶対遵守ルール (Constraints)】
* **アーキテクチャの絶対制約**: Renderer が状態を持たないこと、座標計算は LayoutCalculator で行うこと。
