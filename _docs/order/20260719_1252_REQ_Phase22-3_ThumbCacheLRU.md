### 作業指示書 REQ: Phase 22-3: データベースのオンデマンド読み出しとLRU完成（計画立案）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-3_ThumbCacheLRU.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
`ThumbnailDatabase` に、保存されたパックファイル（img）からJPEGバイナリをオンデマンドで吸い出してWICでデコードし、`ID2D1Bitmap` としてメモリ上に保持・提供する機能と、そのVRAM使用量を管理するための LRU (Least Recently Used) キャッシュ機構を実装する。

*   **要件1: LRUキャッシュの基盤構築**
    *   サムネイルID(`uint32_t`)と `Microsoft::WRL::ComPtr<ID2D1Bitmap>` のペアを管理するLRUリスト(`std::list`)と、そのイテレータを保持するマップ(`std::unordered_map`)を実装し、O(1)でアクセスと順序更新を行えるようにする。
*   **要件2: パックファイルからのオンデマンド吸い出し**
    *   `GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget)` 等のメソッドを実装する。
    *   指定されたIDがキャッシュに存在すれば、LRUリストの先頭（最新）に移動させて、そのビットマップを返す。
    *   存在しない場合、メモリ上のセクタ情報から `img` ファイル上のオフセットとサイズを取得。
    *   `m_ioMutex` でスレッドセーフを担保した上で、`img` ファイルの該当位置にシークし、バイナリデータを吸い出す。
*   **要件3: WICデコードとLRU押し出し機構**
    *   吸い出したJPEGバイナリをメモリ上で WIC デコードし、D2Dが扱えるフォーマット（`GUID_WICPixelFormat32bppPBGRA`等）へ変換した後、`renderTarget->CreateBitmapFromWicBitmap` で `ID2D1Bitmap` を生成する。
    *   生成したビットマップをLRUキャッシュの先頭に登録する。
    *   登録後、キャッシュの要素数が `ConfigManager::GetMaxThumbnailCache()` を超えた場合は、LRUリストの末尾の要素（最も古く使われたもの）をリストとマップから削除し、安全にVRAMを解放する。

#### 【絶対遵守ルール (Constraints)】
*   **UIへの非干渉**: 本フェーズの責務は「データベースからの読み出しとVRAMキャッシュ管理」のみである。UIウィジェット等からの呼び出し（結線）はPhase 22-4で行うため、`Widget_TrackInfo` などの既存描画コードは書き換えないこと。
*   **ファイルロックの最小化**: `img` ファイルからのバイナリ吸い出し処理中は必要最小限の期間だけファイルをオープンし、WICデコードなどの重い処理を実行している最中はミューテックスのロック範囲外とする（または適切に管理する）ことで、サムネイル生成スレッドとのデッドロックやハングを防ぐこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-3 計画書の懸念事項解決と仕様確定
* 対象ファイル: D:\ozlab\oztone\_docs\logs\20260719_1253_RES_Phase22-3_ThumbCacheLRU.md

#### 【作業手順（厳守事項）】
1. 対象ファイル（RES）の「4. 懸念事項・留意点」に記載された内容と、以下の【解決方針】を照らし合わせ、アーキテクチャ上の妥当性を検討すること。
2. 妥当であると判断した場合、対象ファイルを直接編集し、計画書から不確実性（懸念事項）を完全に排除して100%の確定仕様へとアップデートすること。
3. 作業完了後、チャットにて検討結果の所感と、計画書の更新が完了した旨を報告すること。

#### 【解決方針（アンサー）】
*   **懸念1: WICファクトリの管理について**
    *   WICファクトリをメソッド内で都度生成するのはパフォーマンス上深刻な問題を引き起こすため却下する。Phase 2の段階で `Renderer` クラスは既に自身の初期化時に `IWICImagingFactory` を生成して保持しているため、それを `GetThumbnailBitmap` や `DrawThumbnail` の引数としてバケツリレーで渡す設計を**確定仕様**とする。
*   **懸念2: UI非干渉の厳守について**
    *   これはプロジェクトにおける「絶対遵守ルール」そのものであり、不確実性（懸念）として計画書に残しておくべきものではない。

#### 【ファイルの修正要件】
*   対象ファイル内の「タスク4」等の記述を、WICファクトリを引数から受け取る確定仕様として断定的に書き換えること。
*   対象ファイル末尾の「4. 懸念事項・留意点」のセクションを、ブロックごと完全に削除（パージ）すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-3 Task 1-4 : データベースのオンデマンド読み出しとLRU完成
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1253_RES_Phase22-3_ThumbCacheLRU.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルのオンデマンド読み出しとLRUキャッシュ機構の実装である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、実装したコードと既存の作業レポート（20260719_1253_RES_Phase22-3_ThumbCacheLRU.md）の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスク1」から「タスク4」までのチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「タスク1からタスク4(Phase 22-3)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **タスク1: ThumbnailDatabase::Initialize の修正**
    *   `.idx` ファイルのロード処理を実装し、マジックナンバーとヘッダを検証後、各行(`thumbId \t offset \t size`)をパースして `m_sectorMap` にセクタ情報を構築する。
    *   読み込んだ最大の `thumbId` を元に、次回発番用の `m_nextId` を初期化する。
*   **タスク2: ThumbnailDatabase ヘッダ定義の更新**
    *   `GetThumbnailBitmap(uint32_t thumbId, ID2D1RenderTarget* renderTarget, IWICImagingFactory* wicFactory)` を宣言する。
    *   既存の `DrawThumbnail` メソッドの引数に `IWICImagingFactory* wicFactory` を追加する。
    *   LRUキャッシュ用の `std::list<uint32_t> m_lruList` と `std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID2D1Bitmap>> m_cache` 等のメンバ変数を定義する。
*   **タスク3: GetThumbnailBitmap メソッドの実装**
    *   キャッシュにヒットした場合はLRUリストの先頭へ移動させて返す。
    *   キャッシュミス時は `m_sectorMap` からオフセットとサイズを取得し、`m_ioMutex` で保護しつつ `.img` ファイルから対象バイナリを吸い出す。
    *   ファイルロック解放後、WICを用いてバイナリをデコードし、`ID2D1Bitmap` を生成する。
    *   生成したビットマップをLRUキャッシュの先頭に登録し、`ConfigManager::GetMaxThumbnailCache()` を超過した場合は末尾の要素を破棄する。
*   **タスク4: ThumbnailDatabase::DrawThumbnail の改修**
    *   `DrawThumbnail` の実装を、新設した `GetThumbnailBitmap` を呼び出してビットマップを取得し描画するフローへ変更する。
    *   呼び出し元の `Renderer` クラス（`DrawThumbnail` を呼んでいる箇所）の引数に `m_wicFactory.Get()` などのファクトリポインタを渡すように修正する。

#### 【絶対遵守ルール (Constraints)】
*   **ファイルロックの最小化**: `.img` ファイルからのバイナリ吸い出し処理中は必要最小限の期間だけファイルをオープン・ロックし、WICデコードやD2Dリソース生成などの重い処理を実行している最中は絶対にミューテックス (`m_ioMutex`) を握ったままにしないこと。
*   **WICファクトリのバケツリレー**: `IWICImagingFactory` は絶対にメソッド内で都度生成せず、必ず引数として渡されたインスタンスを使用すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Hotfix ThumbnailDatabase のWICヘッダインクルード漏れ修正
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1253_RES_Phase22-3_ThumbCacheLRU.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはビルドエラーの修正（Hotfix）である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1253_RES_Phase22-3_ThumbCacheLRU.md）の末尾に、HOTFIXとして原因と対応内容を追記すること。
3. チャットにて「インクルード漏れの修正が完了しました。再度ビルド確認をお願いします」と報告すること。

#### 【実装要件】
`ThumbnailDatabase` クラスの実装において、WIC関連の識別子（`IWICStream`, `IWICImagingFactory`, `GUID_WICPixelFormat32bppPBGRA` 等）が未定義であるためのビルドエラーが発生している。

*   **要件1: ヘッダの追加**
    *   `src/ThumbnailDatabase.cpp` の先頭のインクルード領域に `#include <wincodec.h>` を追加し、WICのAPI群を認識できるように修正すること。
    *   また、必要であれば `src/ThumbnailDatabase.h` 側での前方宣言やインクルード状況も確認し、適切な状態に保つこと。
