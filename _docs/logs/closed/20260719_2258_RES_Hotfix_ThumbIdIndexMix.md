# RES:HOTFIX作業レポート: サムネイルIDへのインデックス混入バグの修正

## 1. 実装目的
`src/Application_Playback.cpp` のドラムスロットへのデータ注入（`dataProvider` ラムダ式）時において、`thumbId` にプレイリストの曲インデックスが誤って代入される致命的なバグを解消し、正規のサムネイルIDがセットされるように修正する。

## 2. 調査内容
`src/Application_Playback.cpp` 内の `StartDrumAnimation` に渡される `dataProvider` ラムダ式を確認し、`slot->thumbId` に `targetIdx` 等のインデックスが代入されるコードを調査した。
該当箇所においては、該当楽曲のファイルパス (`path`) を用いて `m_thumbnailDatabase.GetOrGenerateThumbId(path, isNew)` を呼び出し、その返り値を `slot->thumbId` にセットする正規の実装へと修正・確認を行った。
さらに、【絶対遵守ルール】に基づき、新規発番 (`isNew == true`) であった場合でも、ラムダ式内では絶対に `m_thumbCacher.EnqueueTrack` を呼び出さず、サムネイルの発注処理は司令塔側の専用ロジックに委ねる（カプセル化の維持）実装となっていることを検証した。

## 3. 対象ファイル
* `src/Application_Playback.cpp`

## 4. 実装タスクリスト
- [x] タスク1: `dataProvider` 内の `thumbId` 代入ロジックの修正 - 誤ったインデックス代入を排除し、ファイルパスベースで `GetOrGenerateThumbId` から正規IDを取得して代入。同時にエンキューを行わない処理であることを確認。

## 5. 詳細作業内容
* タスク1: `dataProvider` 内の `thumbId` 代入ロジックの修正
    - `src/Application_Playback.cpp` の `PlayCurrentTrack` メソッド内に定義された `dataProvider` ラムダ式を修正・点検した。
    - `slot->thumbId = targetIdx;` のような誤代入が行われないよう、確実に `slot->thumbId = m_thumbnailDatabase.GetOrGenerateThumbId(path, isNew);` が実行される実装となっていることを確認した。
    - `isNew` フラグは取得するものの、意図的に `m_thumbCacher.EnqueueTrack` によるサムネイル生成発注を行わないことで、アーキテクチャの責務分離を完全に維持した。
