# RES:実装計画・作業レポート Phase 22-2: サムネイル工場の稼働

## 1. 実装目的
`ThumbCacher` クラスのワーカースレッド内で、画像バイナリをサムネイルサイズにリサイズし、指定された品質のJPEGとして `ThumbnailDatabase` のパックファイル（idx / img）へスレッドセーフに追記保存する「サムネイル工場」の処理を実装する。本フェーズではD2D等の描画関連ロジックは混入させず、純粋なバイナリデータの生成と永続化基盤の確立に専念する。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の拡張 (JPEG品質の追加)
    - `[System]` セクションに `ThumbnailJpegQuality` を追加する。
    - デフォルト値は `0.8f` とし、設定範囲は `0.0f` 〜 `1.0f` とする。
    - `ConfigManager.h`, `ConfigManager_System.cpp`, `ConfigManager_DefaultIni.h` に対し、ゲッター・セッター・INI読み書き処理を追加する。

### 要件2: ThumbnailDatabase の追記API追加
    - `ThumbnailDatabase::StoreCookedData(uint32_t thumbId, const std::vector<BYTE>& data)` メソッドを新規追加する。
    - `img` ファイルの末尾にクック済み画像バイナリを追記し、そのオフセット位置とサイズを `idx` ファイルの末尾に追記する。
    - 複数スレッド（ワーカースレッド群）からの同時書き込み競合を防ぐため、メソッド内部で `std::mutex` を用いてスレッドセーフ化する。

### 要件3: TagManager によるバイナリ抽出機能
    - `TagManager` クラスに、アルバムアートのバイナリ（APICフレーム等）をデコードせずに生の状態で抽出して返す静的メソッド（例: `static std::vector<BYTE> ExtractRawAlbumArt(const std::wstring& filePath)`）を追加する。
    - これにより、サムネイル生成時に不要なデコード処理を通さず、WICへ直接バイナリを渡すためのパイプラインを確立する。

### 要件4: ThumbCacher のWIC画像処理ヘルパーの実装 (新規分離)
    - `ThumbCacher` クラス内に、抽出された生バイナリを受け取り、WICを用いてデコード・スケーリング・JPEGエンコードを行う純粋な画像処理メソッド（例: `CookThumbnailImage`）を実装する。
    - `ConfigManager::GetThumbnailSize` のサイズへ `IWICBitmapScaler` でリサイズする。
    - リサイズ後の画像を、`ConfigManager::GetThumbnailJpegQuality()` の品質値（`IPropertyBag2` で `ImageQuality` を設定）を用いてWIC JPEGエンコーダでメモリ上にエンコードし、クック済みバイナリ（`std::vector<BYTE>`）として返す。

### 要件5: ThumbCacher のワーカースレッドループの完成 (配線)
    - `ThumbCacher::WorkerLoop` の処理本体を実装する。
    - スレッド開始時に `CoInitializeEx(nullptr, COINIT_MULTITHREADED)`、終了時に `CoUninitialize()` を必ず呼び出す。
    - 処理フロー:
        1. キューからパス取得
        2. `ThumbnailDatabase::GetThumbnailId(path)` を呼び出し。既にidxに登録済みならスキップ。
        3. 未登録なら `TagManager` で生バイナリ抽出 (要件3)。
        4. 抽出したバイナリを `CookThumbnailImage` に渡し、JPEG画像に変換 (要件4)。
        5. 変換されたバイナリを `ThumbnailDatabase::StoreCookedData` へ渡し、パックファイルへ永続化する (要件2)。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManager の拡張
    - `ThumbnailJpegQuality` の追加と関連処理の実装。
- [x] タスク2: ThumbnailDatabase の追記API実装
    - スレッドセーフな `StoreCookedData` の実装。
- [x] タスク3: TagManager のバイナリ抽出機能実装
    - 生アルバムアートバイナリ抽出メソッドの追加。
- [ ] タスク4: ThumbCacher - WIC画像処理ヘルパーの実装
    - WICを用いたデコード・スケーリング・JPEGエンコード（品質指定）を行う純粋な画像処理メソッドの実装。
- [ ] タスク5: ThumbCacher - ワーカースレッドループの完成
    - COMマルチスレッド初期化と、各モジュール（DB、TagManager、画像処理、保存）の連携処理の実装。

## 4. 詳細作業内容
### タスク1: ConfigManager の拡張
    - `ConfigManager.h`, `ConfigManager_System.cpp`, `ConfigManager_DefaultIni.h` に `ThumbnailJpegQuality` (デフォルト0.8f) を追加し、読み書きやゲッター・セッターを実装。
### タスク2: ThumbnailDatabase の追記API実装
    - `src/ThumbnailDatabase.h` に `SectorInfo` 構造体と `m_sectorMap` を定義し、追記処理を保護する `std::mutex m_ioMutex` を追加。
    - `src/ThumbnailDatabase.cpp` に `StoreCookedData` を実装。メソッド内でファイルハンドルを都度オープン・クローズし、imgへのバイナリ書き込み、idxへのメタデータ追記、メモリマップへの同期をアトミックに行うよう修正。
    - 【事前監査対応】後続フェーズでのファイル共有違反（クラッシュ）を防ぐためファイルハンドルは都度クローズとし、データ破損を防ぐため必ず `img` → `idx` の順で書き込む安全な設計を厳守した。
### タスク3: TagManager のバイナリ抽出機能実装
    - `TagManager::ExtractAlbumArtBinary` 静的メソッドを実装。
    - `TagLib::MPEG::File` のみを用いてファイルを開き、二重オープンによるロック問題を回避。
    - `APIC` フレームを検索し、生バイナリを `std::vector<BYTE>` として抽出・返却（WIC等の描画APIは不使用）。
### タスク4: ThumbCacher - WIC画像処理ヘルパーの実装
    - （未実施）
### タスク5: ThumbCacher - ワーカースレッドループの完成
    - （未実施）

