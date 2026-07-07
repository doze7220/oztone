# RES:HOTFIX作業レポート: Phase 18-1 プレイリストピン留め時の下部UI判定ズレ修正

## 1. 実装目的
Phase 18-1にてプレイリストのピン留め機能を追加した際、描画側（キャンバスの動的圧縮）には対応したが、入力判定側（下部UIのヒットテスト）に補正が漏れており、クリックやホバーの判定がズレる不具合が発生していた。これを修正し、描画側の圧縮ロジックと当たり判定を完全に同期させる。

## 2. 調査内容
- 描画ロジック(`LayoutCalculator`)におけるキャンバス圧縮処理を確認し、同等の仮想補正が判定側にも必要であると特定。
- 対象となる判定メソッド(`IsInPlaybackControlRegion`, `GetPlaybackButtonAt`, `IsInVolumeControlRegion`)において、X座標(`logicalX`)と横幅(`logicalWidth`)を用いた判定が行われている箇所を特定。

## 3. 詳細作業内容
- `src/Window.cpp` の以下のメソッドに対して、プレイリストのピン留め状態(`m_config->GetIsPlaylistPinned()`)が有効な場合の座標および幅の補正ロジックを追加した。
  - `IsInPlaybackControlRegion`
  - `IsInVolumeControlRegion`
  - `GetPlaybackButtonAt`
- 左配置の場合、`logicalX` および `logicalWidth` からプレイリスト幅(`playlistWidth`)を減算。補正後の `logicalX < 0` であれば領域外のため早期リターン。
- 右配置の場合、`logicalWidth` から `playlistWidth` を減算。補正後の `logicalX >= logicalWidth` であれば領域外のため早期リターン。
- `PROJECT_ARCHITECTURE.md` に、ピン留め時の下部UI判定にオフセット補正が適用されている旨を追記した。
- これにより、プレイリストがピン留めされた状態でも、UIの見た目とマウスクリック/ホバーの判定が完全に一致するようになった。
