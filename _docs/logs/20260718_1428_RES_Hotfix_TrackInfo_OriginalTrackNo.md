# RES:HOTFIX作業レポート: TrackInfoのトラックナンバー本来表示化

## 1. 実装目的
現在再生中の曲情報（`TrackInfoWidget`）に表示されるCD帯風のトラックナンバーを、シャッフルモード時においても「現在のキュー順」ではなく「プレイリストに登録された本来の行番号」で表示するように改修する。

## 2. 調査内容
`src/Widget_TrackInfo.cpp` 内の `UpdateLayout` メソッドにおいて、トラックナンバーが `ctx.currentTrackIndex + 1` で生成されていた。
シャッフル時は本来のインデックスを表示する必要があるため、`ctx.shuffleIndices` を参照するように修正方針を立てた。

## 3. 対象ファイル
* `D:\ozlab\oztone\src\Widget_TrackInfo.cpp`

## 4. 実装タスクリスト
[x] タスク1: 文字列生成ロジックの修正 - `ctx.shuffleIndices`から本来のインデックスを取得するように修正

## 5. 詳細作業内容
* タスク1: 文字列生成ロジックの修正
    - `src/Widget_TrackInfo.cpp` の `UpdateLayout` 内で、`ctx.shuffleIndices` をチェックし、シャッフル配列が存在し、且つ `currentTrackIndex` が範囲内の場合は、`ctx.shuffleIndices[ctx.currentTrackIndex] + 1` を用いてディスプレイ番号とするように安全なロジックへ修正した。
