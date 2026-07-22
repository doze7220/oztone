# RES:HOTFIX作業レポート: 仮想スクロールバグ修正1-B (TrackInfoホバー領域の厳格化)

## 1. 実装目的
仮想スクロール実装に伴い、TrackInfo領域のホバー判定範囲が曖昧になり、画面の半分近くをホバー領域として誤認識してしまうバグを解消する。

## 2. 調査内容
`src/Window_Mouse.cpp` の `IsInTrackInfoRegion` メソッドにて、Y座標の判定（`topLimit`, `bottomLimit`）が、単にコントロール領域の高さを差し引いた大雑把な計算になっていた。これが誤判定の原因であった。
解決策として、ConfigManagerの設定値（各要素のオフセットおよびサイズ・フォントサイズ）を用いて、アルバムアート、タイトル、アーティスト名それぞれの実際の表示バウンディングボックス（上端・下端）を算出し、それらの最小値・最大値から正確な判定領域を再計算する。

## 3. 対象ファイル
* `src/Window_Mouse.cpp`

## 4. 実装タスクリスト
[x] タスク1: TrackInfoホバー判定の厳格化 - Y座標判定ロジックの修正と `<algorithm>` のインクルード追加。

## 5. 詳細作業内容
* タスク1: TrackInfoホバー判定の厳格化
    - `src/Window_Mouse.cpp` の先頭に `<algorithm>` をインクルードした。
    - `IsInTrackInfoRegion` 関数にて、`baseY` を `logicalHeight - m_config->GetBaseBottomOffset()` と定義した。
    - アルバムアート、タイトルテキスト、アーティストテキストの3要素について、それぞれの `top` (Y座標上端) と `bottom` (Y座標下端) を算出した。
    - 算出した3組の `top` のうち最小のものを `topLimit`、`bottom` のうち最大のものを `bottomLimit` として `std::min` および `std::max` を用いて定義し、Y座標のヒットテストに適用した。X座標の制限ロジックについては既存のものをそのまま維持した。
