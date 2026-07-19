# [RES] Phase 22-3: データベースのオンデマンド読み出しとLRU完成

## 1. 概要
本フェーズでは、`ThumbnailDatabase` クラスを拡張し、保存されたサムネイルのパックファイル (`.img`) からオンデマンドでJPEGバイナリを吸い出し、WICでデコードして `ID2D1Bitmap` としてVRAM上に保持・提供する機構を実装する。また、VRAM使用量を適切に管理するため、LRU (Least Recently Used) アルゴリズムを用いたキャッシュ押し出し機構を構築する。

## 2. アーキテクチャ設計・方針
*   **LRUキャッシュ基盤**: `std::list<uint32_t>` を用いてIDのアクセス順序を管理し、`std::unordered_map<uint32_t, ComPtr<ID2D1Bitmap>>` と連携させることで、O(1) でのキャッシュアクセスと順序の更新を実現する。
*   **オンデマンド抽出とWICデコード**: 要求されたサムネイルがキャッシュに存在しない場合、`.idx` ファイルロード時に構築された `m_sectorMap` を参照し、`.img` ファイルから該当バイナリをシークして読み出す。読み出したバイナリをメモリ上でWICデコードし、`GUID_WICPixelFormat32bppPBGRA` 形式へ変換した後、D2Dビットマップを生成する。
*   **ファイルロックの最小化とスレッドセーフ**: バイナリデータの吸い出し（ファイルI/O）時は `m_ioMutex` で排他制御を行うが、その後のWICデコードやD2Dビットマップ生成といった重い処理はミューテックスのロック範囲外で実行し、描画スレッドのハングを防ぐ。
*   **LRU押し出し**: キャッシュ追加後、要素数が `ConfigManager::GetMaxThumbnailCache()` の設定値を超過した場合、LRUリスト末尾（最古）の要素をマップおよびリストから削除し、D2Dリソースを解放する。

## 3. タスクリスト
*   [x] タスク1: `ThumbnailDatabase::Initialize` の修正
    *   `.idx` ファイルのヘッダ検証後、残りの行（`thumbId \t offset \t size`）をパースし、`m_sectorMap` にロードする処理を実装する。
    *   次回発番用の `m_nextId` を、ロードしたIDの最大値 + 1 に初期化する。
*   [x] タスク2: `ThumbnailDatabase` ヘッダ定義の更新
    *   `Microsoft::WRL::ComPtr<ID2D1Bitmap> GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory)` メソッドを宣言する（WICファクトリは外部から受け取る確定仕様）。
    *   既存の `DrawThumbnail` の引数に `IWICImagingFactory* wicFactory` を追加する。
*   [x] タスク3: `GetThumbnailBitmap` メソッドの実装
    *   キャッシュ (`m_cache`) のヒット判定と、ヒット時のLRUリスト先頭への要素移動を実装する。
    *   キャッシュミス時、`m_sectorMap` を確認し、`m_ioMutex` 下で `.img` ファイルを該当の `offset` へシークし、`size` バイト分のバイナリを `std::vector<BYTE>` へ読み出す。
    *   読み出したバイナリをWICでデコード（`IWICStream` -> `IWICBitmapDecoder` -> `IWICFormatConverter`）し、`renderTarget->CreateBitmapFromWicBitmap` でD2Dビットマップを生成する。
    *   生成したビットマップを `m_cache` および `m_lruList` の先頭に登録する。
    *   キャッシュサイズが上限（`GetMaxThumbnailCache()`）を超えた場合、`m_lruList.back()` を用いて最古の要素を破棄（VRAM解放）する処理を実装する。
*   [x] タスク4: `ThumbnailDatabase::DrawThumbnail` の改修
    *   新設した `GetThumbnailBitmap` を呼び出してビットマップを取得し、描画するフローへリファクタリングする。
    *   `DrawThumbnail` のシグネチャに `IWICImagingFactory* wicFactory` を追加し、呼び出し元である `Renderer` から WICファクトリを受け取って `GetThumbnailBitmap` へバケツリレーで渡す確定仕様とする。

## 4. 作業詳細 (Phase 22-3 実装完了報告)

本フェーズにおける各タスクの実装内容を以下の通り詳述する。

*   **タスク1: ThumbnailDatabase::Initialize の修正**
    *   `m_idxPath` に対して `std::ifstream` をバイナリモードで開き、1行目のヘッダ（マジックナンバーとサムネイルサイズ）を検証した。
    *   検証通過時、2行目以降のインデックス情報（`thumbId \t offset \t size`）を `std::getline` と `std::stringstream` で逐次パースし、`m_sectorMap` (`std::unordered_map<uint32_t, SectorInfo>`) にロードする処理を実装した。
    *   パース中に最大の `thumbId` をトラッキングし、次回発番用の `m_nextId` を `maxId + 1` として初期化することで、再起動時のID競合を防止した。

*   **タスク2: ThumbnailDatabase ヘッダ定義の更新**
    *   `ThumbnailDatabase.h` において、`Microsoft::WRL::ComPtr<ID2D1Bitmap> GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory)` をパブリックメソッドとして新規宣言した。
    *   既存の `DrawThumbnail` のシグネチャを更新し、引数に `IWICImagingFactory* wicFactory` を追加した。
    *   LRUキャッシュ管理用に `std::list<uint32_t> m_lruList` および `std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID2D1Bitmap>> m_cache` をプライベートメンバとして定義した。

*   **タスク3: GetThumbnailBitmap メソッドの実装**
    *   **LRUヒット判定**: 最初に `m_mutex` をロックし、要求された `thumbId` が `m_cache` に存在するか確認。存在する場合は `m_lruList` 内の該当IDを一度削除し、`push_front` で先頭へ移動させた上でキャッシュされたビットマップを返す。
    *   **セクタマップ参照**: キャッシュミス時、`m_ioMutex` をロックして `m_sectorMap` を検索。存在しない（まだクックされていない）場合は `nullptr` を返す。
    *   **バイナリ吸い出し (ファイルロック最小化)**: 存在する場合、`m_ioMutex` 下で `.img` ファイルを `sector.offset` へシークし、`sector.size` 分のデータを `std::vector<BYTE>` へ読み出す。読み出し完了時点で直ちにスコープを抜け、ファイルロックを解放した。
    *   **WICデコードとD2Dリソース生成 (UI非干渉)**: ミューテックスのロック外で、読み出したバイナリを `IWICStream` に乗せ、`IWICBitmapDecoder` -> `IWICBitmapFrameDecode` -> `IWICFormatConverter` (フォーマット: `GUID_WICPixelFormat32bppPBGRA`) を経て、最終的に `CreateBitmapFromWicBitmap` にて `ID2D1Bitmap` を生成する重い処理を実行した。
    *   **LRU登録と押し出し (VRAM解放)**: ビットマップ生成後、再度 `m_mutex` をロックして `m_cache` と `m_lruList` の先頭に登録。その後 `m_lruList.size()` が `ConfigManager::GetMaxThumbnailCache()` を超過した場合、末尾の `thumbId` を用いてリストとマップの両方から最古の要素を削除し、VRAMリソースを解放するロジックを組み込んだ。

*   **タスク4: ThumbnailDatabase::DrawThumbnail の改修**
    *   描画の開始処理として、内部で `GetThumbnailBitmap` を呼び出し、戻り値のビットマップを評価するフローへ完全に差し替えた。
    *   キャッシュから有効なビットマップが返却された場合は `context->DrawBitmap` で指定矩形へ描画し、`nullptr` （処理中またはデータなし）の場合は `CreateSolidColorBrush` を用いて透明度0.1の白でガラス板（プレースホルダー）を描画する処理を維持・統合した。

### HOTFIX (2026-07-19)
* **原因**: `ThumbnailDatabase.cpp` にてWIC関連の識別子（`IWICStream`、`IWICImagingFactory`、`GUID_WICPixelFormat32bppPBGRA` 等）を使用しているにもかかわらず、`<wincodec.h>` のインクルードが漏れていたためビルドエラーが発生していた。
* **対応内容**: `src/ThumbnailDatabase.cpp` に `#include <wincodec.h>` を追加し、`src/ThumbnailDatabase.h` に `IWICImagingFactory` の前方宣言を追加してビルドエラーを修正した。
* **HOTFIX (アーキテクチャ更新)**: Phase 22-3 実装に伴い、`PROJECT_ARCHITECTURE.md` の `ThumbnailDatabase` クラスリファレンスを更新。必要な時にのみパックファイルからJPEGバイナリを吸い出してWICデコードする「オンデマンド読み出し機能」と、VRAM使用量を管理して古い画像を破棄する「LRUキャッシュ機構」が備わっている旨を追記した。
