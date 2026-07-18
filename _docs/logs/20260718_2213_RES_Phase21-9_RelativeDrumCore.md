### RES:実装計画・作業レポート Phase 21-9: 画像・テキストのフリップサイクル（ダブルバッファリング）化
#### 1. 実装目的
現在、曲切り替え時にテキストや画像を別変数へ退避コピーしている「バケツリレー方式」を完全に廃止する。
代わりに、曲情報（テキスト、画像、トラックナンバー）を内包した `DrumSlot` 構造体を2つ用意し、曲切り替えのタイミングで参照インデックスをフリップ（0 ⇔ 1）させる「ダブルバッファリング（ピンポンバッファ）方式」へとアーキテクチャを刷新する。
これにより、重いGPUリソースである画像のコピー/破棄やライフサイクルの狂いを根本から解決し、最もスマートでストイックな状態管理を実現する。

#### 2. アーキテクチャ設計
##### 要件1: `DrumSlot` 構造体の新設とバケツリレー変数のパージ
*   `src/WidgetContext.h` に、曲情報をひとまとめにした構造体 `DrumSlot` を定義する。
    （保持データ: `trackTitle`, `trackArtist`, `artBitmap`, `trackNumber`）
*   `Renderer` および `WidgetContext` から、既存のバケツリレー用退避変数（`oldTrackTitle`, `oldTrackArtist`, `oldArtBitmap`, `currentArtBitmap` など）をすべて削除し、代わりに `DrumSlot drumSlots[1];`（または std::array）と `int currentDrumSlotIndex = 0;` を追加する。

##### 要件2: フリップサイクルの実装 (Renderer / Application層)
*   曲が切り替わるタイミング（または新しい目標スロットが決定するタイミング）で、インデックスをフリップする処理（`m_currentDrumSlotIndex = 1 - m_currentDrumSlotIndex;`）を実行する。
*   フリップ直後、新しい「現在のスロット（`drumSlots[m_currentDrumSlotIndex]`）」に対して、最新のテキスト情報（トラックナンバー含む）と画像をセットする。
*   非同期ロードで画像が遅れて到着した場合も、常に「現在のスロット」の `artBitmap` を更新するように配線する。

##### 要件3: `Widget_TrackInfo` のハイブリッド描画の適応
*   描画ループ内での参照先をフリップサイクルに対応させる。
*   目標スロット（`i == 0`）を描画する際は、現在のスロット（`ctx.drumSlots[ctx.currentDrumSlotIndex]`）からテキスト、画像、トラックナンバーを参照する。
*   直前までNOWだったOLDスロット（`i == 1` または `i == -1`）を描画する際は、過去のスロット（`ctx.drumSlots[1 - ctx.currentDrumSlotIndex]`）からすべての情報を参照し、残像として完璧な状態で描画する。
*   それ以外の中間スロット（大ジャンプ時）は、テキストとトラックナンバーについてはオンデマンドでプレイリスト（TrackDatabase）を直接覗きに行き、画像はフォールバック（ガラス板）を描画する既存のハイブリッド方式を維持する。

#### 3. 実装タスクリスト
* [ ] タスク1: `DrumSlot` 構造体の定義と、古いバケツリレー変数の削除 (`src/WidgetContext.h`, `src/Renderer.h`, `src/Renderer.cpp`)
* [ ] タスク2: Renderer層でのフリップ機構とスロット更新処理の実装 (`src/Renderer.cpp`, `src/Renderer_Context.cpp`, `src/Application_Render.cpp`等)
* [ ] タスク3: `Widget_TrackInfo` 描画ロジックの `DrumSlot` 参照への切り替え (`src/Widget_TrackInfo.cpp`)

#### 4. 詳細作業内容
##### タスク1: `DrumSlot` 構造体の定義と古い変数の削除
（作業後に追記）
##### タスク2: Renderer層でのフリップ機構とスロット更新の実装
（作業後に追記）
##### タスク3: `Widget_TrackInfo` 描画ロジックの参照切り替え
（作業後に追記）
