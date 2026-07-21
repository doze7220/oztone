# RES:HOTFIX作業レポート: サムネイル発注のゾンビ化解消（ロールバック＆自律リトライ機能）

## 1. 実装目的
ファイルロック競合等の理由で `ThumbCacher` がサムネイルの抽出・保存（`StoreCookedData`）を完遂できなかった場合、メモリ上に発番情報だけが残り、サムネイルが二度とエンキューされなくなる（ゾンビ化する）バグを修正し、自律的なリトライが行われるようにする。

## 2. 調査内容
`ThumbnailDatabase::GetOrGenerateThumbId` にて発行された ID が `m_pathToId` に残存しつつ、`m_sectorMap` にはデータが登録されない状態（抽出失敗・例外・空データなど）に陥ると、次回UI描画時に `out_isNew` が `false` と判定され、タスクが再キューイングされないことが原因。
`ThumbCacher` 側で処理を完了できなかったパスにおいて、明示的に発番を取り消すロールバック処理を呼び出すことで、UIからの次回の取得要求時に `out_isNew` が `true` となり、自動的にリトライされる機構を実装する。

## 3. 対象ファイル
* `src/ThumbnailDatabase.h`
* `src/ThumbnailDatabase.cpp`
* `src/ThumbCacher.cpp`
* `PROJECT_ARCHITECTURE.md`

## 4. 実装タスクリスト
[x] タスク1: ロールバックメソッドの追加 - `ThumbnailDatabase` に `RollbackThumbId` を実装し、メモリ上の辞書から該当パスを安全に消去する処理を追加。
[x] タスク2: 例外・失敗時のロールバック呼び出し - `ThumbCacher` のワーカー処理において、抽出失敗・変換失敗・保存失敗、および各種例外発生時に `RollbackThumbId` を呼び出すように修正。
[x] タスク3: 開発資料の更新 - `PROJECT_ARCHITECTURE.md` に自律リトライ・ロールバックの機構を追記。

## 5. 詳細作業内容
* タスク1: ロールバックメソッドの追加
    - `ThumbnailDatabase.h` に `void RollbackThumbId(const std::wstring& filepath);` を宣言。
    - `ThumbnailDatabase.cpp` にミューテックス `m_mutex` で保護された上で `m_pathToId.erase(filepath);` を行う実装を追加。
* タスク2: 例外・失敗時のロールバック呼び出し
    - `ThumbCacher.cpp` の `WorkerLoop` 内部にて `try-catch` ブロックを導入。
    - `FileManager::ExtractAlbumArtBinary` で空バイナリが返った場合、および `CookThumbnailImage` 失敗時、さらに `StoreCookedData` が `false` を返した場合のいずれの「放棄パス」においても `m_db->RollbackThumbId(filepath);` が呼び出されるよう修正。
* タスク3: 開発資料の更新
    - `PROJECT_ARCHITECTURE.md` の `ThumbCacher` の項目に、失敗時のロールバックによるUI主導の自律リトライループ機構について追記。
