# RES:HOTFIX作業レポート: 離脱ディレイ中のUIホバー判定抜け完全修正

## 1. 実装目的
Phase 18-10で実装された「離脱ディレイ」機能において、マウスがホバー領域外に出た際の判定ロジックに問題があり、ディレイにより描画上は表示が維持されている状態でも、UI（ロゴ拡張メニュー、プレイリスト等）がクリック等の操作に反応しなくなってしまう不具合を修正するため。

## 2. 調査内容
- `Window` クラスの入力判定（ヒットテスト）フラグが、`Renderer` や `Widget` 側のディレイを伴うアニメーション状態（進行度）を考慮せず、即座に `false` に切り替わっていることが原因であることを確認。
- `Renderer` クラスから最新の展開状態（`isPlaylistExpanded`、`isLogoMenuExpanded`）を各フレームで取得できる構造であるものの、それが `Window` クラスにフィードバックされていない状態であった。

## 3. 詳細作業内容
- `src/Window.h` において、`m_isLogoMenuExpanded` を外部から設定可能にするための `SetLogoMenuExpanded(bool)` メソッドを追加。
- `src/Application.cpp` の `ForceRender` メソッド内にて、`m_renderer.UpdateAnimation()` から取得した `isPlaylistExpanded` と `isLogoMenuExpanded` を `m_window` クラスのセッターを用いて設定する処理を追加。
- これにより、`Window::IsInLogoMenuRegion` や `Window::IsInPlaylistRegion` 内の判定処理において最新のUI展開状態フラグが適用され、ディレイ（AutoHideDelay）適用中であっても領域拡張が維持され、正常に操作が可能となるように修正。
- 修正後、ビルドと動作確認を実施し、正常に機能することを確認。
