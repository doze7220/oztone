# RES:実装計画・作業レポート Phase 22-7: プレイリスト一覧のアルバムジャケット化

## 1. 実装目的
プレイリスト一覧モード（.ozlファイル一覧）の左側余白を活用し、各プレイリストの「1曲目」のサムネイルをアルバムジャケットとして表示する。また、曲一覧と同様の「数字なしのCD帯」を描画することで、UI全体のデザイン統一感を向上させる。この際、サムネイルの生成発注（エンキュー）は描画ループ内で行わず、Application層で完結させる「単方向依存の原則」を厳格に維持する。

## 2. アーキテクチャ設計
### 要件1: PlaylistSummary の拡張 (1曲目のパス保持)
    - `WidgetContext.h` の `PlaylistSummary` に `std::wstring firstTrackPath;` を追加し、各プレイリストの先頭曲のパスを保持させる。
    - `Application_Render.cpp` 内の `UpdatePlaylistSummaries` 関数にて、TSV読み込み時の最初の行からパスを抽出し `firstTrackPath` にセットする。
    - 同時に、抽出したパスに対して `ThumbnailDatabase::GetOrGenerateThumbId` を呼び、未生成(`isNew == true`)であれば `ThumbCacher::EnqueueTrack` を用いてバックグラウンドワーカーに生成を依頼する。これにより、描画層からの非同期エンキューを完全に排除する。

### 要件2: LayoutCalculator と Renderer Context の再調整
    - プレイリスト一覧描画モード時にも描画コンテキスト (`WidgetContext`) へ画像を渡すため、`Renderer_Context.cpp` において `isPlaylistListViewMode` 時のサムネイル読み込み（`GetCachedThumbnailBitmap`）および `playlistThumbnails` への登録ロジックを追加する。
    - `Widget_Playlist_DrawItems.cpp` におけるテキスト描画前に、曲一覧モードと同等の「CD帯を回避する右シフトロジック（`bandRightEdge` の計算と矩形のシフト）」を適用する。

### 要件3: Widget_Playlist_DrawItems の改修 (サムネイルと数字なし帯の描画)
    - `DrawPlaylistList` 関数内で、`ctx.playlistThumbnails` から取得した画像をアスペクト比を維持して `thumbRect` 内の中央にフィット描画する。
    - 曲一覧モードで使用しているCD帯の描画ロジック（矩形ボックスとアンダーラインの描画）を移植し、プレイリストインデックスに応じた色ブレンドを適用する。数字テキスト（`DrawTextLayout`）の部分は除外し、デザイン上のカラーバーとして成立させる。

## 厳重注意事項 (監査項目)
本実装にあたり、以下の4点のアーキテクチャ違反が絶対に発生しないように留意し、実装後のセルフコードレビュー（監査）において必ずチェックすること。

*   **🚨 監査項目1: Widget 層の越権行為（インクルード違反）**
    *   **対象:** `src/Widget_Playlist_DrawItems.cpp`
    *   **内容:** `#include "ThumbnailDatabase.h"` や `#include "ThumbCacher.h"` などのバックエンドクラスを直接インクルードしないこと。Widget は必要なデータをすべて `WidgetContext` (`ctx`) から受け取る「究極の受動態」でなければならない。
*   **🚨 監査項目2: 描画構築層での「発注（エンキュー）」違反**
    *   **対象:** `src/Renderer_Context.cpp`
    *   **内容:** `BuildRenderContext` などのコンテキスト構築ループ内で、`m_thumbnailDatabase->GetOrGenerateThumbId` を呼んだ際に、万が一 `isNew == true` であっても、**絶対に `EnqueueTrack` を呼び出さないこと**。発注権限を持つのは司令塔 (Application) のみであり、描画ループ内で発注すると責務違反となる。
*   **🚨 監査項目3: 司令塔の「発注連打（暴走）」違反**
    *   **対象:** `src/Application_Render.cpp` (`UpdatePlaylistSummaries` 等)
    *   **内容:** `UpdatePlaylistSummaries` が「毎フレーム（60FPSで）」呼ばれるようなフローに入っていないか確認すること。プレイリストのパースとサムネイル生成の発注（EnqueueTrack）は、プレイリストファイルが更新された際や、一覧モードを開いた瞬間などの「必要なタイミングのみ」実行されるべきである。描画処理である `ForceRender` や `Render` ループ内から毎フレーム呼び出されてはならない。
*   **🚨 監査項目4: LayoutCalculator への「状態」の混入**
    *   **対象:** `src/LayoutCalculator.cpp`
    *   **内容:** `CalculatePlaylistItemLayout` などの計算関数内に、「前回計算時の値を保持する (`static` 変数)」などの「状態」を持たせないこと。LayoutCalculator は入力された引数からただ矩形を返すだけの完全に状態を持たない純粋関数（ピクセル計算機）でなければならず、副作用を生んではならない。

## 3. 実装タスクリスト
- [x] タスク1: PlaylistSummary 拡張とサムネイル非同期エンキュー (Application層)
    - `src/WidgetContext.h`: `PlaylistSummary` 構造体に `firstTrackPath` を追加。
    - `src/Application_Render.cpp`: `UpdatePlaylistSummaries` で 1曲目のパスを抽出し、サムネイル生成をエンキューする処理を追加。
- [x] タスク2: Renderer Context の拡張 (描画層へのサムネイル提供)
    - `src/Renderer_Context.cpp`: `BuildRenderContext` を修正し、プレイリスト一覧モード時にも `firstTrackPath` を用いてサムネイルをデコード・コンテキストへ登録する。
- [x] タスク3: プレイリスト一覧UIの描画改修 (Widget層)
    - `src/Widget_Playlist_DrawItems.cpp`: `DrawPlaylistList` にて、CD帯回避のレイアウトシフト、アスペクト比維持のサムネイル描画、および数字なしCD帯の描画ロジックを追加。
- [x] タスク4: ドキュメントの更新
    - `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を必要に応じて更新し、状態管理・UIアーキテクチャの変更点を記載する。

## 4. 詳細作業内容
### タスク1: PlaylistSummary 拡張とサムネイル非同期エンキュー (Application層)
    - `src/WidgetContext.h` の `PlaylistSummary` に `firstTrackPath` と `firstTrackThumbId` を追加。
    - `src/Application_Render.cpp` の `UpdatePlaylistSummaries` で TSV の1曲目読み込み時にパスと ID を取得し、未生成時のみ `m_thumbCacher.EnqueueTrack` を非同期発注。
### タスク2: Renderer Context の拡張 (描画層へのサムネイル提供)
    - `src/Renderer_Context.cpp` の `BuildRenderContext` において、`isPlaylistListViewMode == true` かつ `availablePlaylistsCache` 有効時の分岐を追加。
    - キャッシュからの `GetCachedThumbnailBitmap` 取得のみを行い、ID未発行の場合は処理をスキップ（発注禁止ルールを厳守）。
### タスク3: プレイリスト一覧UIの描画改修 (Widget層)
    - `src/Widget_Playlist_DrawItems.cpp` の `DrawPlaylistList` 関数において、`thumbRect` 内へのアスペクト比維持のサムネイルフィット描画を追加した。
    - 描画コンテキスト (`ctx`) 内の画像データのみを利用し、Widget層における `ThumbnailDatabase` 等のバックエンドクラスの直接インクルードを厳重に回避した（監査項目1）。
    - プレイリスト一覧においても曲一覧と同様のデザイン統一感を持たせるため、「数字なしのCD帯」を描画し、文字描画処理との干渉を防ぐための右シフトレイアウト調整を適用した。
### タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の `Widget_Playlist_DrawItems.cpp` の説明箇所へ、プレイリスト一覧モードにて1曲目のサムネイルがアルバムジャケットとして表示され、デザインの統一感を出すための数字なしCD帯が追加された旨を追記した。
    - `LayoutCalculator.cpp` の修正指示（以前のHotfixによる右シフト無効化の撤廃）については、現状のコードで既に無条件に右シフトが適用される形へ純化（状態を持たない関数化）されていたため、副作用を持たせないように現状を維持（監査項目4をクリア）とした。

### HOTFIX 2: 1曲目サムネイルのオンデマンド読み出し復旧
- **対象ファイル:** `src/Renderer_Context.cpp`
- **内容:** `BuildRenderContext` のプレイリスト一覧モード（`isPlaylistListViewMode == true`）のループ内において、`m_thumbnailDatabase->GetCachedThumbnailBitmap` でキャッシュミス（`nullptr`）となった場合、バックグラウンドでのVRAM読み込みを要求する `m_thumbnailDatabase->RequestThumbnailLoad` の呼び出し処理を追加した。これにより、キャッシュに画像が存在しない場合にサムネイルが永遠にロードされない不具合を解消した。なお、サムネイル自体の新規生成発注は追加しておらず、描画層からの発注禁止のアーキテクチャルールは維持されている。
