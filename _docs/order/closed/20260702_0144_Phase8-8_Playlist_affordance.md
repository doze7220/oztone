Phase 8-8 ステップ3: プレイリストのアフォーダンス向上。ウィンドウ右端にプレイリスト展開用の「引き出しグリップ（縦線と◀アイコン）」を描画し、直感的なUIへの誘導を実装します。すべてのレイアウトとカラーはINIファイルで管理します。

【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作成する。
2. 作業ログ（_docs/logs/YYMMDD_HHMM_Phase8-8_Playlist_affodance.md）を新規作成し、タスクリストを末尾に記載する。
2. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、私の承認を待つこと。

【実装要件】
1. ConfigManager の拡張
対象: src/ConfigManager.h, src/ConfigManager.cpp
- [Layout_Playlist] セクションに以下のパラメータを追加し、読み書きとゲッターを実装してください（カラーはHEX文字列、それ以外はfloat）。
  - PlaylistGripRightOffset (スライド座標からのオフセット。デフォルト: 10.0f)
  - PlaylistGripLineWidth (縦線の太さ。デフォルト: 2.0f)
  - PlaylistGripLineColor (縦線の色。デフォルト: #FFFFFF)
  - PlaylistGripArrowHeight (三角の高さ。デフォルト: 10.0f)
  - PlaylistGripArrowWidth (三角の底辺の幅。デフォルト: 5.0f)
  - PlaylistGripArrowColor (三角の色。デフォルト: #FFFFFF)
  - PlaylistGripShadowOffsetX（三角と縦線のシャドウXオフセット。デフォルト：2px）
  - PlaylistGripShadowOffsetY（三角と縦線のシャドウIオフセット。デフォルト：2px）
  - PlaylistGripShadowOpacity（三角と縦線のシャドウ透明度。デフォルト：0.7）

2. Renderer の初期化と描画処理追加
対象: src/Renderer.h, src/Renderer.cpp
- Initialize 時に、INIから取得した色情報を用いて、縦線用と矢印用の `ID2D1SolidColorBrush` をそれぞれ生成してください。
- Initialize 時に、INIから取得した `PlaylistGripArrowHeight` と `PlaylistGripArrowWidth` を用いて、左向きの三角形を描画するための `ID2D1PathGeometry` を生成してください。
- Render メソッドにおけるプレイリスト描画処理の冒頭（プレイリスト背景板の描画付近）で、以下の順序でグリップを追加描画してください。
  - 基準X座標: `gripX = m_playlistSlideX - PlaylistGripRightOffset`
  - 影描画: 本体の描画位置に `PlaylistGripShadowOffsetX` と `PlaylistGripShadowOffsetY` を足した座標へ、`PlaylistGripShadowOpacity` を適用した黒の半透明ブラシ（既存のUI影描画の手法を使用）を用いて、縦線と矢印の影を描画する。縦線の影は画面最上部(Y=0)から画面最下部まで引くこと。
  - 縦線本体描画: `(gripX, 0)` （画面最上部）から `(gripX, logicalHeight)` まで、`PlaylistGripLineWidth` の太さと生成した専用ブラシで縦線を描画する。
  - 矢印本体描画: 画面の中央Y座標 (`logicalHeight / 2.0f`) 付近に、生成した矢印ジオメトリを `gripX` を基準に配置（TranslateTransform）し、専用ブラシで塗りつぶし描画する。
- プレイリストが閉じている状態（m_playlistSlideX == logicalWidth）でも、画面右端から PlaylistGripRightOffset 分だけ内側にラインと矢印が視認でき、ホバーによってプレイリスト全体と一緒にスライドしてくる物理的な「引き出しの取っ手」として機能するように描画順と座標を設計すること。
