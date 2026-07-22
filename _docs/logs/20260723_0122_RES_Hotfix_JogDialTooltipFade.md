# RES:HOTFIX作業レポート: ジョグダイヤリング中のツールチップ非表示化

## 1. 実装目的
ジョグダイヤリング（マウスホイール操作）中にTrackInfo領域のツールチップ（アフォーダンス）を非表示にフェードアウトさせ、UXを向上させること。

## 2. 調査内容
Renderer層に状態を持たせないアーキテクチャを厳守するため、`Application_Render.cpp` のメインループから `m_virtualScrollTimer > 0.0f` をジョグダイヤリング中の判定として抽出し、`Renderer::UpdateAnimation` および `Renderer::Render` の引数として追加して `WidgetContext` まで伝達させる方式を採用した。

## 3. 対象ファイル
* `src/WidgetContext.h`
* `src/Renderer.h`
* `src/Renderer_Update.cpp`
* `src/Renderer_Draw.cpp`
* `src/Renderer_Context.cpp`
* `src/Application_Render.cpp`
* `src/Widget_TrackInfo.cpp`

## 4. 実装タスクリスト
[x] タスク1: WidgetContext への状態追加
[x] タスク2: Renderer メソッドのシグネチャ拡張（状態非保持の徹底）
[x] タスク3: Application層からの直接伝達
[x] タスク4: TrackInfoWidget でのツールチップ退避ロジック

## 5. 詳細作業内容
* タスク1: WidgetContext への状態追加
    - `src/WidgetContext.h` の `WidgetContext` 構造体に `bool isJogDialing = false;` を追加した。
* タスク2: Renderer メソッドのシグネチャ拡張（状態非保持の徹底）
    - `src/Renderer.h`, `src/Renderer_Update.cpp`, `src/Renderer_Draw.cpp` の `UpdateAnimation`, `Render` メソッド引数に `bool isJogDialing = false` を追加した。
    - `src/Renderer_Context.cpp` の `BuildAnimationContext`, `BuildRenderContext` に引数を追加し、`ctx.isJogDialing = isJogDialing;` として代入した。
* タスク3: Application層からの直接伝達
    - `src/Application_Render.cpp` の `ForceRender` メソッド内から `UpdateAnimation` と `Render` を呼ぶ際、末尾引数として `m_virtualScrollTimer > 0.0f` を渡した。
* タスク4: TrackInfoWidget でのツールチップ退避ロジック
    - `src/Widget_TrackInfo.cpp` の `UpdateAnimation` にて、フェードインの条件を `ctx.isTrackInfoHovered && !ctx.isJogDialing` に変更した。これによりジョグダイヤリング中は即座にフェードアウトに移行する。
