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

**要件3: Widget_TrackInfo 描画ロジックの完全純化（オンデマンドの廃止）**
    *   `src/Widget_TrackInfo.cpp` の描画ループから、プレイリストやデータベースを直接参照するオンデマンド取得処理を **完全に削除** する。
    *   Widget はデータの取得を行わず、コンテキストから渡される `ctx.drumSlots[ctx.currentDrumSlotIndex]`（カレント）と `ctx.drumSlots[1 - ctx.currentDrumSlotIndex]`（過去）の2つの情報のみを使用し、相対座標 `i` に従ってテキストと画像を描画するだけに徹する。

#### 【絶対遵守ルール (Constraints)】
*   **ゼロコピー原則**: 画像やテキストを古い変数へ「コピー（代入）」するバケツリレー処理は絶対に記述しないこと。現在と過去の切り替えは、常にインデックス（`0` ⇔ `1`）のフリップのみで解決すること。
*   **Widgetの責務厳守**: Widget 内部で TrackDatabase などのデータ層へアクセスしてはならない。必要なデータはすべて事前にカレントの DrumSlot へ読み込んでおくこと。

#### 3. 実装タスクリスト
* [x] タスク1: `DrumSlot` 構造体の定義と、古いバケツリレー変数の削除 (`src/WidgetContext.h`, `src/Renderer.h`, `src/Renderer.cpp`)
* [x] タスク2: Renderer層でのフリップ機構とスロット更新処理の実装 (`src/Renderer.cpp`, `src/Renderer_Context.cpp`, `src/Application_Render.cpp`等)
* [x] タスク3: `Widget_TrackInfo` 描画ロジックの完全純化

#### 4. 詳細作業内容
##### タスク1: `DrumSlot` 構造体の定義と古い変数の削除
- `WidgetContext.h` に `DrumSlot` 構造体を追加。`std::array<DrumSlot, 2> drumSlots;` と `int currentDrumSlotIndex = 0;` を定義し、`oldTrackTitle`, `oldTrackArtist`, `currentArtBitmap` 等のバケツリレー変数を削除。
- `Renderer.h` の変数定義を更新し、`m_currentTrackTitle` などの一時退避用変数を `m_drumSlots` と `m_currentDrumSlotIndex` に置き換え。
- `Renderer.cpp` にて、古い変数への代入処理を削除し、`m_drumSlots[m_currentDrumSlotIndex].artBitmap` を参照するように修正。
##### タスク2: Renderer層でのフリップ機構とスロット更新の実装
- `Renderer::SetDrumTarget` にて、曲変更時（相対距離入力時だけでなく常に）に `m_currentDrumSlotIndex = 1 - m_currentDrumSlotIndex;` を実行し、参照先をフリップする処理を追加。同時にフェッチ待ち用に `artBitmap` をクリア。
- `Renderer::UpdateTextLayouts` にて、旧変数の更新処理を、現在の `m_drumSlots[m_currentDrumSlotIndex]` に対するテキスト情報（トラックタイトル、アーティスト、計算済みのトラックナンバー文字列）の更新へ変更。毎フレーム最新情報で上書きすることで、タグ更新等にもシームレスに対応。
- `Renderer_Context.cpp` にて、`WidgetContext` へパッキングしていたレガシー変数（`oldTrackTitle` 等）を削除し、代わりに `drumSlots` 配列と `currentDrumSlotIndex` を渡すように修正。
##### タスク3: `Widget_TrackInfo` 描画ロジックの参照切り替え
- `Widget_TrackInfo.cpp` の `UpdateLayout` から、`ctx.shuffleMetadataList` と `ctx.totalTracks` を直接参照するオンデマンドのメタデータ取得・テキスト生成処理を完全に削除。
- `UpdateLayout` での更新判定を `m_lastTotalTracks` 等への依存から `ctx.drumSlots[ctx.currentDrumSlotIndex]` 内の文字列との比較依存に変更。
- `Draw` 内の `drawDrumItem` にて、`relativeIndex == 0` の場合は `ctx.drumSlots[ctx.currentDrumSlotIndex]` のカレント情報を、`relativeIndex == m_animatingOldIndexOffset` の場合は `ctx.drumSlots[1 - ctx.currentDrumSlotIndex]` の過去情報を参照して描画するように純化。
- 中間スロットの場合のオンデマンド取得を削除し、常にフォールバックとなる空のガラス（または何も描画しない）状態として扱うように変更。これにより Widget は提供された2つのバッファにのみ従う完全な受動態となった。


