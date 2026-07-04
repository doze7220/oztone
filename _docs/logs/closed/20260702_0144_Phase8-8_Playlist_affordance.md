# 20260702_0144_Phase8-8_Playlist_affordance.md

## 実装目的
プレイリストのアフォーダンス向上。ウィンドウ右端にプレイリスト展開用の「引き出しグリップ（縦線と◀アイコン）」を描画し、直感的なUIへの誘導を実装する。すべてのレイアウトとカラーはINIファイルで管理する。

## 懸念点・確認事項
実装計画書にも記載したが、指示書の要件「`gripX = m_playlistSlideX - PlaylistGripRightOffset`」については、実際の画面右端からのスライド挙動を考慮すると、`m_playlistSlideX` をそのまま絶対X座標として扱うのは誤りであり、「`gripX = (renderTargetSize.width - playlistWidth + m_playlistSlideX) - PlaylistGripRightOffset`」となる。実装上はこちらを正として進める。

## タスクリスト
- [x] Task 1: `ConfigManager` の拡張 (`src/ConfigManager.h`, `src/ConfigManager.cpp`)
  - [Layout_Playlist] セクションにグリップ用の設定項目を追加し、初期値とゲッター・ローダーを実装。
- [x] Task 2: `Renderer` へのブラシ・ジオメトリ初期化追加 (`src/Renderer.h`, `src/Renderer.cpp`)
  - `Initialize` 時にグリップ用のブラシおよび左向き三角形のジオメトリを生成。
- [x] Task 3: `Renderer` へのグリップ描画処理追加 (`src/Renderer.cpp`)
  - `Render` 処理内で、プレイリスト背景の直前または直後に、グリップの影、縦線、矢印本体の描画を追加。
  - プレイリストが閉じている状態でも右端に見えるようにする。

### タスク1 詳細作業内容
- `src/ConfigManager.h`: `Layout_Playlist` にグリップ用プロパティ（`m_playlistGripRightOffset`、`m_playlistGripLineWidth` など9項目）の変数宣言と、それぞれに対応するゲッターを追加しました。
- `src/ConfigManager.cpp`: `DEFAULT_INI_CONTENT` にグリップ設定のデフォルト値を追加し、コンストラクタでの初期化処理、および `LoadSettings` での読み込み処理（`GetPrivateProfileStringW` と `std::stof` 等を使用）を実装しました。

### タスク2 詳細作業内容
- `src/Renderer.h`: `m_playlistGripLineBrush`, `m_playlistGripArrowBrush`, `m_playlistGripArrowGeometry` を宣言しました。
- `src/Renderer.cpp`: `Initialize` メソッド内で、`ConfigManager` から取得した色設定（`HexToD2D1ColorF` 経由）をもとにブラシを生成し、`GetPlaylistGripArrowWidth/Height` を使って左向きの三角形のパスジオメトリを生成・構築する処理を追加しました。

### タスク3 詳細作業内容
- `src/Renderer.cpp`: `Render` メソッド内、プレイリストの背景を描画する直前（`isPlaylistHovered` による `m_playlistManualScrollY` リセット処理の直後）に、グリップ（縦線と左向き矢印）の描画処理を追加しました。描画時は、プレイリストの開閉状態（`m_playlistSlideX`）と連動する `playlistX` を基準に `PlaylistGripRightOffset` 分内側にずらした座標を計算し、`DrawLine` と `FillGeometry` を用いて、本体と影をそれぞれ描画しました。これにより、プレイリストが閉じている状態でも右端から引き出しが見えるアフォーダンスを実現しています。
