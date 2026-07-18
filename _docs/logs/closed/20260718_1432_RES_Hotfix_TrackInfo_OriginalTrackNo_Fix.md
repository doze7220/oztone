# RES:HOTFIX作業レポート: TrackInfo本来表示化の連携漏れ修正

## 1. 実装目的
TrackInfoWidgetのテキストレイアウト更新時にシャッフルインデックス配列が正しく渡されておらず、トラックナンバーの本来表示が機能していない不具合を修正する。

## 2. 調査内容
1. `src/Renderer_Context.cpp` の `BuildLayoutContext()` 内で、`WidgetContext` 構築時に `ctx.shuffleIndices` への代入が漏れていた。
2. `src/Application_Render.cpp` の `Application::ForceRender()` 内で、`m_renderer.SetShuffleIndices(...)` が `m_renderer.UpdateTextLayouts()` よりも後に呼ばれており、テキスト更新時に最新のインデックスが反映されていなかった。

## 3. 対象ファイル
* `D:\ozlab\oztone\src\Renderer_Context.cpp`
* `D:\ozlab\oztone\src\Application_Render.cpp`

## 4. 実装タスクリスト
[x] タスク1: `BuildLayoutContext` および `BuildAnimationContext` への `shuffleIndices` 代入追加
[x] タスク2: `Application::ForceRender` 内での `SetShuffleIndices` 呼び出し順序の適正化

## 5. 詳細作業内容
* タスク1: `BuildLayoutContext` および `BuildAnimationContext` への `shuffleIndices` 代入追加
    - `src/Renderer_Context.cpp` を修正し、`ctx.shuffleIndices = m_shuffleIndices;` を追加した。
* タスク2: `Application::ForceRender` 内での `SetShuffleIndices` 呼び出し順序の適正化
    - `src/Application_Render.cpp` を修正し、`m_renderer.SetShuffleIndices(m_playlistManager.GetShuffleIndices());` をメソッドの先頭付近（`UpdateTextLayouts`の呼び出し前）に移動させた。
