##### 作業指示書 REQ: Hotfix / ジョグダイヤリング中のツールチップ非表示化 (アーキテクチャ準拠版) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは、ジョグダイヤリング操作時にTrackInfo領域のツールチップを非表示にするUX改善のHotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_JogDialTooltipFade.md）として新規作成すること。
4. チャットにて「ジョグダイヤリング中のツールチップ非表示化が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
TrackInfo領域へのホバー時に表示されるツールチップ（アフォーダンス）について、マウスホイールによるジョグダイヤリング操作を開始した瞬間に非表示へと遷移（フェードアウト）させるUXを実装する。Rendererに状態を持たせないよう、既存のアーキテクチャに従いメソッドの引数経由で WidgetContext へ状態を伝達する。

*   **要件1: WidgetContext への状態追加**
    *   `src/WidgetContext.h` の `WidgetContext` 構造体に `bool isJogDialing = false;` を追加する。
*   **要件2: Renderer メソッドのシグネチャ拡張（状態非保持の徹底）**
    *   `src/Renderer.h`、`src/Renderer_Update.cpp`、`src/Renderer_Draw.cpp` における `UpdateAnimation` と `Render` メソッドの引数に `bool isJogDialing` を追加する。
    *   ※絶対に `Renderer` クラスのメンバ変数として状態を保持しないこと。
    *   `src/Renderer_Context.cpp` のコンテキスト構築処理（`BuildAnimationContext` 等）にて、引数で受け取った値を `ctx.isJogDialing = isJogDialing;` として代入する。
*   **要件3: Application層からの直接伝達**
    *   `src/Application_Render.cpp` の `ForceRender` メソッド内で、`m_renderer.UpdateAnimation(...)` および `m_renderer.Render(...)` を呼び出す際、新たな引数として現在ジョグダイヤリング中であるかどうかの判定（`m_virtualScrollTimer > 0.0f`）を渡す。
*   **要件4: TrackInfoWidget でのツールチップ退避ロジック**
    *   `src/Widget_TrackInfo.cpp` の `UpdateAnimation` メソッドを改修する。
    *   ツールチップの不透明度（`m_tooltipAlpha` 等）を増加させるフェードインの条件を、従来の「ホバー中」から「ホバー中 かつ ジョグダイヤリング中でない（`ctx.isHovered && !ctx.isJogDialing`）」へと厳格化する。
    *   ジョグダイヤリング中（`ctx.isJogDialing == true`）の場合は、マウスが領域内にあっても不透明度を 0.0f に向かって減衰（フェードアウト）させる引き算のUIを実装する。

#### 【絶対遵守ルール (Constraints)】
*   **Rendererの状態非保持** : RendererクラスにUIの状態（メンバ変数やセッター）を絶対に追加してはならない。必ず引数によるバケツリレーで「そのフレーム限りの状態」としてWidgetContextへ伝達するアーキテクチャを厳守すること。