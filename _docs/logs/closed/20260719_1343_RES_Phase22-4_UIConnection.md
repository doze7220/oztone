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
[x] タスク1: Application層のエンキュー処理の実装 (対象: Application_Initialize.cpp, Application_Playlist.cpp, Application_FileDrop.cpp)
    - プレイリストへ楽曲が追加されるタイミングで `m_thumbCacher.EnqueueTrack()` を追加。

[x] タスク2: LayoutCalculatorの拡張 (対象: LayoutCalculator.h, LayoutCalculator.cpp)
    - `PlaylistItemLayout` に `thumbRect` を追加し、`CalculatePlaylistItemLayout` でサムネイル領域の確保とテキスト矩形のシフトを計算する。

[x] タスク3: ThumbnailDatabaseの非同期化インターフェース追加 (対象: ThumbnailDatabase.h, ThumbnailDatabase.cpp)
    - キャッシュのみを参照する `GetCachedThumbnailBitmap()` と、バックグラウンドでの読み込み・デコードを管理する非同期ロードの仕組みを追加する（UIスレッドでWICデコードを行わない）。

[x] タスク4: Renderer層での可視範囲計算とWidgetContextへの結線 (対象: WidgetContext.h, Renderer_Update.cpp, Renderer_Context.cpp)
    - `WidgetContext.h` に `std::unordered_map<size_t, ID2D1Bitmap*> playlistThumbnails;` を追加する。
    - `Renderer_Update.cpp` にて表示領域（スクロール位置）から可視インデックス範囲を求め、`ThumbnailDatabase` にロードを要求する。
    - `Renderer_Context.cpp` にてキャッシュ取得を行い、取得できたビットマップをマップに詰めて `WidgetContext` に渡す。

[x] タスク5: Widget_Playlist_DrawItemsの純粋な描画対応 (対象: Widget_Playlist_DrawItems.cpp)
    - 描画ループ内にて `WidgetContext` の `playlistThumbnails` を参照し、画像があれば描画、なければガラス板を描画する。レイアウトは `LayoutCalculator` の矩形に完全に従う。

## 4. 詳細作業内容
### タスク1: Application層のエンキュー処理の実装
    - 起動時のプレイリスト復元時（`Application_Initialize.cpp`）、プレイリスト切り替え時（`Application_Playlist.cpp`）、およびドラッグ＆ドロップによるファイル追加時（`Application_FileDrop.cpp`）において、追加されたトラックを `m_trackAnalyzer.AddTrackToQueue()` と同時に `m_thumbCacher.EnqueueTrack()` でキューへ流し込むように処理を追加した。
### タスク2: LayoutCalculatorの拡張
    - `LayoutCalculator.h` の `PlaylistItemLayout` にサムネイル描画用の `thumbRect` を追加した。
    - `LayoutCalculator.cpp` の `CalculatePlaylistItemLayout` にて、行の左端に `thumbPadding` を考慮したサムネイル用矩形を計算し、既存のテキスト類（タイトル、アーティスト）をサムネイルの幅（`itemHeight`相当）分だけ右へシフトさせるロジックを実装した。
### タスク3: ThumbnailDatabaseの非同期化インターフェース追加
    - `ThumbnailDatabase.h` / `.cpp` にて、UIスレッドブロック防止のため `GetCachedThumbnailBitmap()` を追加。このメソッドはインメモリのキャッシュ(`m_cache`)から画像が存在すれば返し、存在しなければ即座に `nullptr` を返す。
    - 取得ミス時に画像を非同期ロードするための `RequestThumbnailLoad()` を実装した。
    - 重複してスレッドが立ち上がるのを防ぐため、`m_loadingSet` によるガード機構を設けた。
    - 実際のロード処理は `std::thread` によるバックグラウンド実行（detach）で行い、WICによるデコード完了後に `m_mutex` で排他制御をかけながら `ID2D1Bitmap` を生成・キャッシュに登録するロジックを構築した。
### タスク4: Renderer層での可視範囲計算とWidgetContextへの結線
    - `WidgetContext.h` にサムネイル画像を渡すためのお弁当箱として `std::unordered_map<size_t, ID2D1Bitmap*> playlistThumbnails;` を追加した。
    - `Renderer.h` および `Renderer.cpp` に `ThumbnailDatabase` のポインタを受け取る `SetThumbnailDatabase()` を追加し、`Application_Initialize.cpp` で結線した。
    - `Renderer_Context.cpp` の `BuildRenderContext()` にて、プレイリストの現在の可視範囲を割り出し、算出した範囲の各インデックスに対して `ThumbnailDatabase::GetCachedThumbnailBitmap()` で画像を取得。画像がなければ `RequestThumbnailLoad()` で非同期ロードを要求する処理を実装し、取得できたビットマップを `ctx.playlistThumbnails` マップへ格納した。
### タスク5: Widget_Playlist_DrawItemsの純粋な描画対応
    - `Widget_Playlist_DrawItems.cpp` の `DrawTrackList` 内にて、データ層のインターフェースを叩かず、コンテキストから渡された `ctx.playlistThumbnails` のマップを参照する純粋な受動態描画を実装した。
    - 対象インデックスの画像が存在すれば `LayoutCalculator` が算出した `thumbRect` 領域に描画し、存在しなければ `m_playlistHighlightBrush` を用いたガラス板（透過度0.05f）をフォールバックとして美しく描画する処理を実装した。
    - キャッシュに画像がない場合でも描画ループを一切ブロックしない（非ブロック描画の維持）仕様を満たした。
