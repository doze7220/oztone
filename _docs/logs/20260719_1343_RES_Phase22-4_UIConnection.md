# RES:実装計画・作業レポート Phase 22-4: UIアーキテクチャの結線（修正版）

## 1. 実装目的
完成したサムネイルエンジン（`ThumbCacher` および `ThumbnailDatabase`）をアプリケーション層のイベントおよびWidget層の描画ループと結線し、プレイリストの全曲サムネイル表示を稼働させる。その際、UIスレッドのブロッキングを防ぐ完全非同期なオンデマンド読み出し機構を構築し、各クラスの責務分離（究極のカプセル化）を厳守する。

## 2. アーキテクチャ設計
### 要件1:サムネイル生成キューへのエンキュー (Application層)
    - `Application` クラスにて、プレイリストへ楽曲が追加されるタイミング（起動時のリスト復元時、およびドラッグ＆ドロップによる追加時など）で、追加されたファイルパスを `m_thumbCacher.EnqueueTrack()` を用いてキューへ流し込む処理を実装する。
    - `Application_Initialize.cpp` (起動時), `Application_Playlist.cpp` (プレイリスト切り替え), `Application_FileDrop.cpp` (ドロップ追加) を修正。

### 要件2:サムネイル用のレイアウト計算純化 (LayoutCalculator層)
    - `LayoutCalculator.h / .cpp` の `PlaylistItemLayout` 構造体に `D2D1_RECT_F thumbRect` を追加する。
    - `CalculatePlaylistItemLayout` 内でサムネイルの矩形座標を算出し、同時に `titleRect` および `artistRect` をサムネイル幅＋マージン分右にシフトする。これにより Widget の描画処理内でのアドホックな座標シフトを排除する。

### 要件3:非同期オンデマンド読み出し機構と可視範囲ロード (Renderer / ThumbnailDatabase層)
    - `ThumbnailDatabase` に、UIスレッドをブロックしないキャッシュ参照メソッド `GetCachedThumbnailBitmap()` と、バックグラウンドでのディスク読み込み・WICデコードを予約する `RequestLoadThumbnail()` 機構を追加する。
    - `Renderer_Update.cpp` において、`LayoutCalculator` や `Widget_Playlist` のスクロール状態を利用して「現在可視状態にあるプレイリストのアイテム範囲」を割り出し、その範囲のトラックに対してのみ先行して `RequestLoadThumbnail()` を発行する。

### 要件4:究極の受動態としての描画処理 (Widget層)
    - `WidgetContext.h` に、そのフレームで描画可能なサムネイル画像を格納するマップ（例: `std::unordered_map<size_t, ID2D1Bitmap*> playlistThumbnails;`）を追加する。
    - `Renderer_Context.cpp` にて、`ThumbnailDatabase::GetCachedThumbnailBitmap()` を用いて取得できたビットマップのみをこのマップに詰め込み、純粋なデータとしてWidgetへ渡す設計とする（能動的なインターフェースやコールバックの受け渡しは禁止）。
    - `Widget_Playlist_DrawItems.cpp` では、`LayoutCalculator` から受け取った `thumbRect` に従い、渡されたマップ内に画像ポインタが存在すれば無条件で描画し、無ければフォールバック（ガラス板）を描画するだけの純粋な受動態処理に徹する。

## 3. 実装タスクリスト
[ ] タスク1: Application層のエンキュー処理の実装 (対象: Application_Initialize.cpp, Application_Playlist.cpp, Application_FileDrop.cpp)
    - プレイリストへ楽曲が追加されるタイミングで `m_thumbCacher.EnqueueTrack()` を追加。

[ ] タスク2: LayoutCalculatorの拡張 (対象: LayoutCalculator.h, LayoutCalculator.cpp)
    - `PlaylistItemLayout` に `thumbRect` を追加し、`CalculatePlaylistItemLayout` でサムネイル領域の確保とテキスト矩形のシフトを計算する。

[ ] タスク3: ThumbnailDatabaseの非同期化インターフェース追加 (対象: ThumbnailDatabase.h, ThumbnailDatabase.cpp)
    - キャッシュのみを参照する `GetCachedThumbnailBitmap()` と、バックグラウンドでの読み込み・デコードを管理する非同期ロードの仕組みを追加する（UIスレッドでWICデコードを行わない）。

[ ] タスク4: Renderer層での可視範囲計算とWidgetContextへの結線 (対象: WidgetContext.h, Renderer_Update.cpp, Renderer_Context.cpp)
    - `WidgetContext.h` に `std::unordered_map<size_t, ID2D1Bitmap*> playlistThumbnails;` を追加する。
    - `Renderer_Update.cpp` にて表示領域（スクロール位置）から可視インデックス範囲を求め、`ThumbnailDatabase` にロードを要求する。
    - `Renderer_Context.cpp` にてキャッシュ取得を行い、取得できたビットマップをマップに詰めて `WidgetContext` に渡す。

[ ] タスク5: Widget_Playlist_DrawItemsの純粋な描画対応 (対象: Widget_Playlist_DrawItems.cpp)
    - 描画ループ内にて `WidgetContext` の `playlistThumbnails` を参照し、画像があれば描画、なければガラス板を描画する。レイアウトは `LayoutCalculator` の矩形に完全に従う。

## 4. 詳細作業内容
### タスク1: Application層のエンキュー処理の実装
    - (未実施)
### タスク2: LayoutCalculatorの拡張
    - (未実施)
### タスク3: ThumbnailDatabaseの非同期化インターフェース追加
    - (未実施)
### タスク4: Renderer層での可視範囲計算とWidgetContextへの結線
    - (未実施)
### タスク5: Widget_Playlist_DrawItemsの純粋な描画対応
    - (未実施)
