# RES:実装計画・作業レポート Phase 22-5: サムネイルDBのID駆動復元とバグ修正

## 1. 実装目的
サムネイルインデックスファイル (`.idx`) における「IDとファイルパスの紐付け喪失」および「オフセットがすべて0で記録される」致命的欠陥、ファイルシークの不具合を修正し、堅牢なID駆動データベースを復元する。

## 2. アーキテクチャ設計
### 要件1: .idx ファイルへのファイルパス保存と紐付けマップの構築
    - `ThumbnailDatabase::Initialize` にて、`.idx` のロード処理を改修し、`thumbId \t offset \t size \t filepath` として読み込み、`m_sectorMap` と `m_pathToId` を同時に構築する。
    - `ThumbnailDatabase::StoreCookedData` に `filepath` 引数を追加し、`.idx` へ追記する際、末尾に `\t filepath` を出力する。

### 要件2: ID発番・エンキュー窓口の統合 (GetOrGenerateThumbId)
    - `ThumbnailDatabase` から `ThumbCacher` への依存を完全にパージし、純粋な辞書管理メソッドとして `uint32_t GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew)` を新設する。
    - 内部で `m_pathToId` を検索し、未発番なら新規IDを発行してマップへ登録し、`out_isNew = true` を返す。
    - `ThumbCacher::EnqueueTrack` および内部キューを、`thumbId` と `filepath` のペアを受け取るように改修する。
    - `Application` 層（曲が追加される箇所）を司令塔とし、`GetOrGenerateThumbId` を呼び出して `out_isNew == true` の場合のみ、自ら `m_thumbCacher.EnqueueTrack(thumbId, filepath)` を発注する設計とする。

### 要件3: オフセット0バグの修正 (StoreCookedData)
    - `.img` ファイルへのバイナリ書き込み直前に、`m_imgFile.seekp(0, std::ios::end);` を実行し、直後の `tellp()` で正しい末尾オフセットを取得するよう修正する。

### 要件4: ファイルシーク不具合の修正 (GetThumbnailBitmap 等)
    - `RequestThumbnailLoad` および `GetThumbnailBitmap` 内での `.img` 読み出し時、`ifs.seekg` を呼ぶ直前に `ifs.clear();` を実行し、ストリームのエラー状態を確実にリセットする。

## 3. 実装タスクリスト
- [x] タスク1: `ThumbnailDatabase` ヘッダおよび基本定義の改修
    - `uint32_t GetOrGenerateThumbId(const std::wstring& filepath, bool& out_isNew)` の新設と `GetThumbnailId` の削除/置換
    - `StoreCookedData` 引数に `filepath` を追加
- [x] タスク2: `ThumbCacher` キュー構造の改修
    - `EnqueueTrack` 引数を `(uint32_t thumbId, const std::wstring& filepath)` に変更
    - タスクキューを `std::queue<std::pair<uint32_t, std::wstring>>` に変更
- [x] タスク3: `ThumbnailDatabase.cpp` の不具合修正と要件実装
    - `Initialize` で `filepath` の読み込みと `m_pathToId` の構築を実装
    - `GetOrGenerateThumbId` の実装（新規発番時は `out_isNew = true` を返す）
    - `StoreCookedData` での `.idx` 書き込みフォーマット修正と、`.img` への `seekp(0, std::ios::end)` 追加
    - `GetThumbnailBitmap` および `RequestThumbnailLoad` での `ifs.clear()` 追加
- [x] タスク4: `ThumbCacher.cpp` のワーカー処理改修
    - `WorkerLoop` にてキューからペアを取り出し、不要になった `GetThumbnailId` 呼び出しを削除
    - `m_db->StoreCookedData` に `filepath` も渡すよう修正
- [x] タスク5: `Application` 層の司令塔化
    - `Application_Initialize.cpp`, `Application_Playlist.cpp`, `Application_FileDrop.cpp` にて、`GetOrGenerateThumbId` を呼び出し、`isNew == true` の場合のみ `m_thumbCacher.EnqueueTrack(thumbId, filepath)` を呼び出すよう改修
- [x] タスク6: ドキュメント（PROJECT_ARCHITECTURE.md）の更新
    - `Application` 層による `ThumbnailDatabase` と `ThumbCacher` の単一方向依存・司令塔アーキテクチャについて追記

## 4. 詳細作業内容
### タスク1: `ThumbnailDatabase` ヘッダおよび基本定義の改修
    - `ThumbnailDatabase.h` の `GetThumbnailId` を削除し、`GetOrGenerateThumbId` に置換した。
    - `StoreCookedData` の引数に `const std::wstring& filepath` を追加した。
    - （`m_pathToId` は既に定義されていたことを確認済み）
### タスク2: `ThumbCacher` キュー構造の改修
    - `ThumbCacher.h` の `EnqueueTrack` の引数を `(uint32_t thumbId, const std::wstring& filepath)` に変更した。
    - タスクキュー `m_taskQueue` を `std::queue<std::pair<uint32_t, std::wstring>>` に変更した。
### タスク3: `ThumbnailDatabase.cpp` の不具合修正と要件実装
    - `Initialize` にて、`.idx` ファイルからの読み込みに `filepath` カラムを追加し、`m_pathToId` のマップを同時に構築する処理を追加した。
    - `GetOrGenerateThumbId` メソッドを実装し、既存IDを返す場合は `out_isNew = false`、新規ID発番時はマップへ登録し `out_isNew = true` を返すようにした。
    - `StoreCookedData` で `.idx` へ書き込むフォーマット末尾に `\t filepath` を追加。また、`.img` 書き込み直前に `seekp(0, std::ios::end)` と `tellp()` を実行して正確な末尾オフセットを取得する修正を適用した。
    - `GetThumbnailBitmap` および `RequestThumbnailLoad` 内でバイナリを読み込む際、`seekg` 呼び出し直前に `ifs.clear()` を追加し、ストリームのエラー状態（EOF等）をリセットするようにした。
### タスク4: `ThumbCacher.cpp` のワーカー処理改修
    - `WorkerLoop` にて `m_taskQueue` から `std::pair<uint32_t, std::wstring>` を取り出すよう修正した。
    - 不要になった `ThumbnailDatabase::GetThumbnailId` の呼び出しを完全に削除し、受け取ったペアのIDをそのまま利用するようにした。
    - `m_db->StoreCookedData` の呼び出しで、第2引数として `filepath` を正しく渡すように修正した。
### タスク5: `Application` 層の司令塔化
    - `Application_Playlist.cpp`、`Application_Initialize.cpp`、`Application_FileDrop.cpp` における楽曲読み込み時の処理を更新した。
    - `m_thumbCacher.EnqueueTrack(path)` を直接呼び出していた箇所を廃止し、`m_thumbnailDatabase.GetOrGenerateThumbId(path, isNew)` でIDの確認および発番を行うように変更。
    - 取得した `isNew` が `true` の場合のみ、`m_thumbCacher.EnqueueTrack(thumbId, path)` で画像生成を発注する司令塔ロジックを実装し、不必要な処理の削減と単方向依存を確立した。
### タスク6: ドキュメント（PROJECT_ARCHITECTURE.md）の更新
    - `PROJECT_ARCHITECTURE.md` 内の `Application` クラスの説明文に、ThumbnailDatabaseとThumbCacherを仲介する「司令塔」としての責務を追記。
    - `ThumbnailDatabase` クラスを、IDとファイルパスの紐付け管理を行う「純粋な辞書管理・窓口」という表現へ更新。
    - `ThumbCacher` クラスを、外部からの発注に従って黙々とサムネイルを生成する「純粋な工場（ワーカー）」という表現へ更新。

### HOTFIX: Renderer_Contextの古いメソッド呼び出し修正
- `src/Renderer_Context.cpp` におけるコンパイルエラー (C2039) を修正。
- 描画構築時にサムネイルIDを取得する箇所で、削除済みの `GetThumbnailId` 呼び出しを `GetOrGenerateThumbId` に置換した。
- ※描画層であるため、新規発番（`isNew == true`）の場合でも `ThumbCacher` への発注処理（`EnqueueTrack`）は行わない仕様を遵守した。
