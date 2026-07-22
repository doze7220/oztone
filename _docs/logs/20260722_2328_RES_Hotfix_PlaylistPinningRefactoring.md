# RES:HOTFIX作業レポート: プレイリストのピン留め機能のアーキテクチャ適正化と負の遺産パージ

## 1. 実装目的
プレイリストピン留め時の強制ホバーによる他UIのホバー判定阻害の解消、ドラムアニメーションにおけるスクロール方向の反転不具合の修正、およびタイトルカラーがホバー時に変化しない問題の修正を行う。

## 2. 調査内容
- **ピン留め機能の適正化**: Window層で `GetIsPlaylistPinned()` を参照して無条件でホバー判定を拡張していた部分が他UIの阻害要因だった。Window層は純粋な座標ベースの判定に戻し、展開状態の維持はWidget層（既存実装）に一任する。併せて、背景クリック等に埋め込まれていたピン留め用の迂回ロジック(`!IsInPlaylistRegion`等)を本来の `!m_isPlaylistHovered` へと適正化する。
- **仮想スクロール方向の反転**: `Application_Initialize.cpp` 内のホイール入力（delta）に伴うインデックス加減算が逆に定義されていたため（UP方向で+1）、符号を反転させて修正。
- **TrackInfoのホバー色**: `Widget_TrackInfo.cpp` 内でテキスト描画ブラシ（`m_textBrush`）のカラーが常に白(`D2D1::ColorF::White`)で固定されていた。他Widget同様、`m_hoverAlpha` を用いて設定色(`GetFocusColor`)との間で線形補間する処理を追加。

## 3. 対象ファイル
* `src/Window_Mouse.cpp`
* `src/Application_Initialize.cpp`
* `src/Widget_TrackInfo.cpp`

## 4. 実装タスクリスト
[x] タスク1: Window層の純化 (強制ホバー専有の廃止)とワークアラウンドのパージ
[x] タスク2: 仮想スクロール方向の反転修正
[x] タスク3: TrackInfoテキストのホバー時カラー補間追加

## 5. 詳細作業内容
* タスク1: Window層の純化 (強制ホバー専有の廃止)とワークアラウンドのパージ
    - `src/Window_Mouse.cpp` の `m_isPlaylistHovered` 設定処理から `isPinned ||` のロジックを削除し、純粋な座標判定（`IsInPlaylistRegion`）のみに純化。
    - `HandleLButtonDown` および `HandleRButtonDown` の背景クリック判定において、`!IsInPlaylistRegion` 等に迂回されていた条件式を `!m_isPlaylistHovered` へとクリーンアップ。
* タスク2: 仮想スクロール方向の反転修正
    - `src/Application_Initialize.cpp` において、`delta > 0` (UP) の際にインデックスを減算、`delta < 0` (DOWN) の際に加算するように修正。
* タスク3: TrackInfoテキストのホバー時カラー補間追加
    - `src/Widget_TrackInfo.cpp` の描画処理(`Draw`)において、テキスト描画用の `m_textBrush` に対して、`m_hoverAlpha` を用いたベースカラー(白)とフォーカスカラーの線形補間処理を追加。対象曲（`absoluteIndex == ctx.animatingTargetIndex`）にのみ適用。
