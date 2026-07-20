# RES:HOTFIX作業レポート: ThumbnailManagerの内部結線・委譲漏れの修復

## 1. 実装目的
Phase 23-4で新設した `ThumbnailManager` において、ビルドは通るものの実行時にサムネイルが生成・表示されない不具合の修復。

## 2. 調査内容
原因は、`ThumbnailManager::GetOrGenerateThumbId` などの窓口メソッドが呼び出された際に、新規発番であること（`isNew == true`）を検知しても、画像生成を担当する `ThumbCacher` へ `EnqueueTrack` を発注する処理がどこからも呼び出されていなかったため。
修正方針として、外部クラス（Application等）に再び `isNew` の判定や `ThumbCacher` を意識させるような逆行した修正は行わず、`ThumbnailManager` 内部にロジックを完結（カプセル化）させ、`GetOrGenerateThumbId` メソッド内で自身で `EnqueueTrack` を呼び出すように実装を変更した。
なお、初期化（`Initialize`）、終了（`Uninitialize`）、およびコンストラクタにおける `ThumbCacher` への `ThumbnailDatabase` 参照渡しは、既に正しく実装されていることを確認した。

## 3. 対象ファイル
* `src/ThumbnailManager.cpp`

## 4. 実装タスクリスト
[x] タスク1: EnqueueTrack (新規発注) ロジックの修正 - `ThumbnailManager::GetOrGenerateThumbId` 内で `isNew == true` 時に `m_cacher->EnqueueTrack` を発注するよう修正

## 5. 詳細作業内容
* タスク1: EnqueueTrack (新規発注) ロジックの修正
    - 【作業ファイル】: `src/ThumbnailManager.cpp`
    - 【作業内容】: `GetOrGenerateThumbId` メソッド内で、`m_database->GetOrGenerateThumbId` の戻り値 `out_isNew` を確認し、trueの場合に `m_cacher->EnqueueTrack(id, filepath);` を呼び出すように処理を追加。これにより、Facadeとしてのカプセル化を保ちつつ確実なサムネイル生成発注ルートを構築した。
