# RES:HOTFIX作業レポート: プレイリストのトラックナンバー本来表示化

## 1. 実装目的
シャッフルモード時においても、プレイリストに表示されるトラックナンバー（CD帯）を「現在のキュー順（i+1）」ではなく「プレイリストに登録された本来の行番号」で表示するように改修する。

## 2. 調査内容
PlaylistManagerから現在のシャッフルインデックス配列を取得し、RendererおよびWidgetContextを経由してWidget_Playlist_DrawItemsへ伝達するデータフローの構築が必要。描画時には伝達された本来のインデックスを利用してトラックナンバーを計算する。

## 3. 対象ファイル
* `src/WidgetContext.h`
* `src/Renderer.h`
* `src/Renderer.cpp`
* `src/Renderer_Context.cpp`
* `src/Application_Render.cpp`
* `src/Widget_Playlist_DrawItems.cpp`

## 4. 実装タスクリスト
[x] タスク1: PlaylistManager へのゲッター追加 (既存の `GetShuffleIndices` をそのまま活用)
[x] タスク2: Renderer および WidgetContext へのデータ伝達ルート構築
[x] タスク3: Application からのデータ同期
[x] タスク4: Widget_Playlist_DrawItems での描画ロジック改修

## 5. 詳細作業内容
* タスク1: PlaylistManager へのゲッター追加
    - `src/PlaylistManager.h` および `.cpp` を確認し、スレッドセーフにシャッフルインデックス配列のコピーを返す `GetShuffleIndices` メソッドが既に存在するため、これを活用した。
* タスク2: Renderer および WidgetContext へのデータ伝達ルート構築
    - `src/WidgetContext.h` に `std::vector<size_t> shuffleIndices;` を追加した。
    - `src/Renderer.h` に `m_shuffleIndices` メンバ変数と `SetShuffleIndices` メソッドを追加し、`src/Renderer.cpp` に実装した。
    - `src/Renderer_Context.cpp` にて、コンテキスト構築時に `ctx.shuffleIndices = m_shuffleIndices;` を代入するよう修正した。
* タスク3: Application からのデータ同期
    - `src/Application_Render.cpp` の `ForceRender` メソッド内で、描画処理の直前に `m_renderer.SetShuffleIndices(m_playlistManager.GetShuffleIndices());` を呼び出すように追加した。
* タスク4: Widget_Playlist_DrawItems での描画ロジック改修
    - `src/Widget_Playlist_DrawItems.cpp` 内の CD 帯のトラックナンバー文字列生成処理にて、`ctx.shuffleIndices` から本来のインデックスを取得し、表示に利用するよう変更した。安全のため `ctx.shuffleIndices.size() > i` の境界チェックも実装した。
