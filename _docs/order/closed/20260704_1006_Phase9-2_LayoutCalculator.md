### Phase 9-2: Renderer レイアウト計算の外部化（作業指示書）

#### 【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（_docs/logs/YYMMDD_HHMM_Phase9-2_LayoutCalculator.md）として新規作成する。
2. 作業計画をタスクリストに分割し、作業レポート末尾に記載する。
3. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、ユーザーの承認を待つこと。

#### 1. 目的
Phase 9-1で分割された各描画メソッド（`DrawBackground` や `DrawTrackInfo` など）の内部には、未だに「X座標は幅の何％で…」「Y座標は下端から何ピクセルで…」といった座標計算処理（ビジネスロジック）が混在している。
本フェーズでは、これらの「レイアウト計算」を純粋な計算クラス（`LayoutCalculator`）へと外部化し、Rendererの各描画メソッドを「渡された座標情報に従ってピクセルを打つだけの純粋な描画関数」へと昇華させる。

---
#### 2. 対象ファイル
*   `src/LayoutCalculator.h` (新規作成)
*   `src/LayoutCalculator.cpp` (新規作成)
*   `src/Renderer.h`
*   `src/Renderer.cpp`
*   `CMakeLists.txt`

#### 3. 実装ステップ

**Step 1: `LayoutCalculator` クラスの作成**
*   `src/LayoutCalculator.h` と `src/LayoutCalculator.cpp` を新規作成する。
*   このクラスは状態を持たず、入力値（ウィンドウサイズ、DPIスケール、INI設定値など）を受け取り、描画に必要な座標や矩形（`D2D1_RECT_F` や `D2D1_POINT_2F` など）を算出・返却するだけの純粋な静的メソッド（またはconstメソッド）群として定義する。
*   *例:* `static D2D1_RECT_F GetAlbumArtRect(float logicalWidth, float logicalHeight, ...);`

**Step 2: 座標計算ロジックのお引越し**
*   `Renderer.cpp` の各描画メソッド内に散らばっている座標計算コード（`logicWidth - rightMargin` や `currentY + PlaylistTimeOffsetY` など）を、`LayoutCalculator` の各メソッド内へ完全に移行する。
*   計算に必要な INI設定値（`ConfigManager` からの取得値）は、計算の入力パラメータとして `LayoutCalculator` に渡すか、あるいは `LayoutCalculator` に `ConfigManager` への参照を持たせて算出する。

**Step 3: Renderer 描画メソッドのリファクタリング**
*   `Renderer.cpp` の各描画メソッドは、最初に `LayoutCalculator` を呼び出して座標の構造体（`D2D1_RECT_F`等）を受け取り、それを使って `DrawBitmap` や `DrawTextLayout` などのDirect2D APIを呼び出すだけのクリーンな処理に書き換える。

**Step 4: ビルドと結合テスト**
*   `CMakeLists.txt` に新規作成した `LayoutCalculator.h/cpp` を追加する。
*   ビルドを行い、レイアウト（UIの配置やサイズ）がPhase 9-1の時点と1ピクセルも狂わずに完全に一致していることを確認する。

#### 4. 懸念点・注意事項

* **描画順序の維持:** 本フェーズの目的は「計算の外部化」のみである。UI要素が描画される順番や条件分岐（`if (isHovered)` など）は絶対に変更しないこと。

* **DPIスケーリングの境界:** 現在のRendererアーキテクチャでは描画の根元で `SetTransform` によるDPIスケーリングを行っているため、`LayoutCalculator` が算出する座標はすべて「論理ピクセル」を基準としたままでよい。DPI計算を二重に掛けないこと。

* **テキストレイアウト:** `IDWriteTextLayout` は生成時に幅・高さを必要とするため、テキスト領域の最大幅や配置矩形などのレイアウト計算は `LayoutCalculator` が担当すること。

* **責務の分離:** `LayoutCalculator` が担当するのはレイアウト計算のみとする。描画APIの呼び出し、色・ブラシの選択、描画条件の判定、アニメーション状態の更新などは行わないこと。

* **戻り値の設計:** レイアウト結果は UI要素単位の構造体として返却すること。矩形を個別に返すメソッドを大量に作成せず、`TrackInfoLayout` や `PlaylistLayout` のように関連する情報をまとめた構造体で返すこと。
* 