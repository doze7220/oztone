### 作業指示書 REQ: Phase 22-2: サムネイル工場の稼働（計画立案）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-2_ThumbCacherCook.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
`ThumbCacher` クラスのワーカースレッド内で、画像バイナリをサムネイルサイズにリサイズし、指定された品質のJPEGとして `ThumbnailDatabase` のパックファイル（idx / img）へスレッドセーフに追記保存する「サムネイル工場」の処理を実装する。

*   **要件1: ConfigManager の拡張 (JPEG品質の追加)**
    *   `[System]` セクションに `ThumbnailJpegQuality` （デフォルト: 0.8f、範囲: 0.0f〜1.0f）を追加し、ゲッター・セッター・INI読み書き処理を実装する。
*   **要件2: ThumbnailDatabase の追記API追加**
    *   クック済みバイナリデータ（`std::vector<BYTE>`）を受け取り、`img` ファイルの末尾に追記し、そのオフセット位置とサイズを `idx` ファイルに追記するメソッド（例: `StoreCookedData(uint32_t thumbId, const std::vector<BYTE>& data)`）を実装する。
    *   複数スレッドからの同時アクセスに備え、ファイル書き込み処理は `std::mutex` 等を用いてスレッドセーフにすること。
*   **要件3: TagManager によるバイナリ抽出機能**
    *   `TagManager` を拡張し、曲ファイルパスからアルバムアートのバイナリ（APICフレーム等）のみを抽出し、デコードせずにそのままのバイナリとして返す静的メソッド（またはそれに準ずる機能）を追加する。
*   **要件4: ThumbCacher のワーカースレッド実装 (WICリサイズとJPEGエンコード)**
    *   `ThumbCacher::WorkerLoop` のTODO部分を実装する。
    *   キューから取得したファイルパスに対し、`ThumbnailDatabase::GetThumbnailId` を呼んで ID を取得する。既に `idx` に登録済みの場合はスキップする。
    *   未登録の場合、要件3の機能でバイナリを抽出する。
    *   抽出したバイナリを WIC (Windows Imaging Component) を用いてデコードし、`ConfigManager::GetThumbnailSize` のサイズへスケーリング（`IWICBitmapScaler`等を使用）する。
    *   スケーリング後の画像を、`ConfigManager::GetThumbnailJpegQuality()` の品質設定を用いて JPEG 形式のバイナリとしてメモリ上に再エンコードする（`IPropertyBag2` を用いて `ImageQuality` を設定すること）。
    *   再エンコードされたバイナリデータを、要件2のAPIへ渡してデータベースに永続化する。

#### 【絶対遵守ルール (Constraints)】
*   **COMのマルチスレッド初期化**: `ThumbCacher` のワーカースレッド内でWICを使用するため、スレッド起動時に必ず `CoInitializeEx(nullptr, COINIT_MULTITHREADED)` を行い、終了時に `CoUninitialize()` を行うこと。
*   **D2D描画ロジックの混入禁止**: 本フェーズでは「バイナリデータの生成と保存」に特化する。`DrawThumbnail` の実装や、`ID2D1Bitmap` への変換・UI描画連携は後続のフェーズで行うため、記述しないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-2 Task 1 : ConfigManager の拡張 (JPEG品質)
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンにおけるJPEG品質設定の追加である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1150_RES_Phase22-2_ThumbCacherCook.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク1が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
サムネイル画像のエンコード品質をユーザーが制御できるよう設定値を追加する。

*   **要件1: ConfigManager の拡張**
    *   `src/ConfigManager.h`, `src/ConfigManager_System.cpp`, `src/ConfigManager_DefaultIni.h` を修正し、`[System]` セクションに以下を追加する。
        *   `ThumbnailJpegQuality` （デフォルト: 0.8f）: 生成するJPEGサムネイルの品質（0.0f 〜 1.0fの浮動小数点数）
    *   ゲッター（`GetThumbnailJpegQuality`）、セッター、およびINI読み書き処理（`LoadSystemSettings`, `Set...` 内での保存処理等）を実装する。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-2 Task 2 : ThumbnailDatabase の追記API実装
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンにおけるデータベースへのスレッドセーフな追記機能の追加である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1150_RES_Phase22-2_ThumbCacherCook.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク2が完了しました。ビルド確認をお願いします」と報告すること。

#### タスク2 実装計画書の重大な欠陥修正（事前監査）
##### 原因・理由:
タスク2の当初の実装要件（単純なファイルへの追記）のまま実装を進めると、以下の3点の重大なアーキテクチャ崩壊を引き起こすことが事前監査により判明した。
1. **ファイルの占有によるクラッシュ**: `std::ofstream` をクラスのメンバとして開きっぱなしにすると、後続フェーズでUIスレッドが画像を読み込もうとした際に「ファイル共有違反」でクラッシュする（これがまずい）。
2. **書き込み順序によるデータ破損**: `idx`（カタログ）に先に書き込み、直後にアプリが落ちた場合、「カタログにはあるが画像の実体がない」という致命的な破損データが生まれ、次回起動時にデコードエラーを起こす（これがまずい）。
3. **メモリ同期の漏れ**: 物理ファイルに追記するだけでは `ThumbnailDatabase` クラスのメモリ側（RAM）が更新を検知できず、UIから描画要求が来ても「キャッシュにない」と判定されてしまう。

##### 対応:
上記の問題を未然に防ぐため、タスク2の実装要件と絶対遵守ルールを以下の通り修正し、アーキテクチャを堅牢化する。
1. **ファイルハンドルの都度開閉**: ファイルストリームはメンバ変数にせず、メソッド内のローカルスコープで都度オープン・クローズを行い、ロック期間を最小限にする。
2. **アトミック性の担保**: 必ず「`img`（画像実体）への書き込みとクローズ」が完全に成功してから「`idx`（カタログ）への追記」を行う順序を厳守させる。
3. **セクタ情報のメモリ管理追加**: オフセットとサイズを保持する `SectorInfo` 構造体と `m_sectorMap` をメンバに追加し、ファイルへの追記成功と同時にメモリ側の管理テーブルも同期させる。

#### 【実装要件】
`ThumbnailDatabase` クラスに、セクタ情報（オフセットとサイズ）を管理するメモリ構造を追加し、画像バイナリをスレッドセーフにファイルへ追記してメモリと同期するAPIを実装する。

*   **要件1: セクタ情報のメモリ管理構造追加**
    *   `src/ThumbnailDatabase.h` に、オフセット(`uint64_t`等)とサイズ(`size_t`)を保持する `SectorInfo` 構造体を定義する。
    *   `std::unordered_map<uint32_t, SectorInfo> m_sectorMap;` をメンバに追加する。
*   **要件2: スレッド保護の導入**
    *   `src/ThumbnailDatabase.h` のメンバに `std::mutex m_ioMutex;` を追加する。
*   **要件3: 追記APIの実装**
    *   `src/ThumbnailDatabase.h` と `src/ThumbnailDatabase.cpp` に、メソッド `bool StoreCookedData(uint32_t thumbId, const std::vector<BYTE>& data)` を追加する。
    *   メソッド内では、まず `std::lock_guard<std::mutex> lock(m_ioMutex);` でスレッドセーフを担保する。
    *   `data` が空でない場合、`std::ofstream` を用いて `img` ファイルを `std::ios::app | std::ios::binary` モードで開く。
    *   `tellp()` で現在の末尾オフセット位置を取得し、`data` の内容を書き込んで明示的に `close()` する。
    *   `img` の書き込み成功後、同様に `std::ofstream` で `idx` ファイルを `std::ios::app` モードで開き、`thumbId \t offset \t size` の形式でセクタ情報を書き込んで `close()` する。
    *   ファイルへの追記が全て成功した後、`m_sectorMap[thumbId] = {offset, data.size()};` としてメモリ上の管理テーブルを更新する。

#### 【絶対遵守ルール (Constraints)】
*   **ファイルハンドルの最小占有**: ファイルはクラスのメンバ変数として開きっぱなしにせず、必ず `StoreCookedData` メソッド内のローカルスコープで都度オープン・クローズを行い、他のスレッドが読み込みを行えるようにすること。
*   **アトミック性の担保**: データ破損を防ぐため、必ず「`img` への書き込みとクローズ」が完全に終わってから「`idx` への追記」を行う順序を厳守すること。
*   **責務分離の原則**: 本タスクは「バイト配列を受け取ってファイルとメモリマップに書き込むこと」のみに専念する。WICを用いた画像のエンコード処理などは絶対に記述しないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-2 Task 4 : ThumbCacher - WIC画像処理ヘルパーの実装
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンにおけるWICを用いた画像処理ヘルパーメソッドの実装である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1150_RES_Phase22-2_ThumbCacherCook.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク4が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
`ThumbCacher` クラスに、生バイナリを受け取り、WICを用いてサムネイルサイズに縮小し、指定品質のJPEGバイナリへと再エンコードする純粋な画像処理メソッドを追加する。

*   **要件1: メソッドの追加**
    *   `src/ThumbCacher.h` と `src/ThumbCacher.cpp` に、メソッド `std::vector<BYTE> CookThumbnailImage(const std::vector<BYTE>& rawBinary, UINT targetSize, float jpegQuality)` を追加する。
*   **要件2: WICによるデコードとスケーリング**
    *   `IWICImagingFactory` をローカル生成（またはメンバで保持）して用いること。
    *   `rawBinary` をもとにメモリ上に `IWICStream` を作成し、デコードする。
    *   アスペクト比を維持しつつ、長辺が引数の `targetSize` となるように縮小（スケーリング）する `IWICBitmapScaler` を作成・適用する。
*   **要件3: JPEGエンコードと品質指定**
    *   スケーリング後の画像を JPEG 形式 (`GUID_ContainerFormatJpeg`) としてエンコードするためのエンコーダとメモリストリームを作成する。
    *   エンコーダの初期化時、`IPropertyBag2` を取得し、`ImageQuality` プロパティに引数の `jpegQuality` (0.0f〜1.0f) を設定して反映させること。
    *   エンコード結果のメモリバイナリを抽出し、`std::vector<BYTE>` として返す。失敗した場合は空のベクターを返す。

#### 【絶対遵守ルール (Constraints)】
*   **責務分離の原則**: 本メソッドは「バイナリデータの変換アルゴリズム」のみに専念する。スレッドのキューからのパス取得や、データベースへの保存 (`StoreCookedData`)、`std::mutex` 等のロック制御は絶対に記述しないこと。これらは後続のタスク5で行う。
*   **COMインターフェースの適切な管理**: メモリリークを防ぐため、WICの各オブジェクトは必ず `Microsoft::WRL::ComPtr` を用いて管理すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Hotfix サムネイルDBのファイルパス構築バグ修正
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルDBのファイルパス構築バグの修正（Hotfix）である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1150_RES_Phase22-2_ThumbCacherCook.md）の末尾に、HOTFIXとして原因と対応内容を追記すること。
3. チャットにて「ファイル名バグの修正が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
サムネイルエンジンのパックファイル（idx / img）のパス生成ロジックに不具合があり、実行ファイル名（`OZtone.exe`）の直後にファイル名が結合されてしまっているバグを修正する。

*   **要件1: パス構築ロジックの修正**
    *   `src/Application_Initialize.cpp` または `src/ThumbnailDatabase.cpp` など、パックファイルのパスを生成しているコード箇所を検索して特定する。
    *   `std::filesystem::path` を利用し、実行ファイルのフルパスから `.parent_path()` で親ディレクトリを確実に取得した上で、ファイル名を結合（`/` 演算子または `append`）するように修正する。
    *   修正例: `auto dbPath = exePath.parent_path() / L"oztone_track_thumb_idx.odb";`

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-2 Task 4 : ThumbCacher - WIC画像処理ヘルパーの実装
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンにおけるWICを用いた画像処理ヘルパーメソッドの実装である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1150_RES_Phase22-2_ThumbCacherCook.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク4が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
`ThumbCacher` クラスに、生バイナリを受け取り、WICを用いてサムネイルサイズに縮小し、指定品質のJPEGバイナリへと再エンコードする純粋な画像処理メソッドを追加する。

*   **要件1: メソッドの追加**
    *   `src/ThumbCacher.h` と `src/ThumbCacher.cpp` に、メソッド `std::vector<BYTE> CookThumbnailImage(const std::vector<BYTE>& rawBinary, UINT targetSize, float jpegQuality)` を追加する。
*   **要件2: WICによるデコードとスケーリング**
    *   `IWICImagingFactory` をローカル生成（またはメンバで保持）して用いること。
    *   `rawBinary` をもとにメモリ上に `IWICStream` を作成し、デコードする。
    *   アスペクト比を維持しつつ、長辺が引数の `targetSize` となるように縮小（スケーリング）する `IWICBitmapScaler` を作成・適用する。
*   **要件3: JPEGエンコードと品質指定**
    *   スケーリング後の画像を JPEG 形式 (`GUID_ContainerFormatJpeg`) としてエンコードするためのエンコーダとメモリストリームを作成する。
    *   エンコーダの初期化時、`IPropertyBag2` を取得し、`ImageQuality` プロパティに引数の `jpegQuality` (0.0f〜1.0f) を設定して反映させること。
    *   エンコード結果のメモリバイナリを抽出し、`std::vector<BYTE>` として返す。失敗した場合は空のベクターを返す。

#### 【絶対遵守ルール (Constraints)】
*   **責務分離の原則**: 本メソッドは「バイナリデータの変換アルゴリズム」のみに専念する。スレッドのキューからのパス取得や、データベースへの保存 (`StoreCookedData`)、`std::mutex` 等のロック制御は絶対に記述しないこと。これらは後続のタスク5で行う。
*   **COMインターフェースの適切な管理**: メモリリークを防ぐため、WICの各オブジェクトは必ず `Microsoft::WRL::ComPtr` を用いて管理すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-2 Task 5 : ThumbCacher - ワーカースレッドループの完成 (配線)
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンにおけるバックグラウンドワーカースレッドの配線と完成である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1150_RES_Phase22-2_ThumbCacherCook.md）の「タスク5」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク5が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`ThumbCacher` クラスのワーカースレッドループ (`WorkerLoop`) にて、これまで作成した各コンポーネント（Config、DB、TagManager、WICヘルパー）を連携させ、キューに追加された曲パスのサムネイルを非同期でクックしてデータベースに保存する一連のパイプラインを完成させる。

*   **要件1: COMのマルチスレッド初期化**
    *   `src/ThumbCacher.cpp` の `WorkerLoop` スレッド処理の冒頭（無限ループの直前）で `CoInitializeEx(nullptr, COINIT_MULTITHREADED)` を実行し、スレッド終了直前（関数の末尾）で `CoUninitialize()` を実行する処理を追加する。
*   **要件2: ワーカースレッドループの実装**
    *   `WorkerLoop` 内でキューから `filepath` を取り出した後のTODO部分に以下のパイプライン処理を実装する。
    *   1. `m_thumbnailDatabase->GetThumbnailId(filepath)` を呼び、サムネIDを取得する。
    *   2. そのIDが既にデータベースのセクタ情報（クック済みデータ）として登録済みか確認し、登録済みであれば処理をスキップ（`continue`）する。（※もし `ThumbnailDatabase` に確認用のメソッドがなければ、追加実装して利用すること）
    *   3. 未登録の場合、`TagManager::ExtractAlbumArtBinary(filepath)` を呼び出し、生バイナリを取得する。空であればスキップする。
    *   4. `ConfigManager` から `GetThumbnailSize()` と `GetThumbnailJpegQuality()` を取得する。
    *   5. `CookThumbnailImage(rawBinary, size, quality)` を呼び出し、リサイズ＆エンコード済みのJPEGバイナリを取得する。変換失敗（空）の場合はスキップする。
    *   6. 成功した場合、`m_thumbnailDatabase->StoreCookedData(thumbId, cookedBinary)` を呼び出し、データベースへ永続化する。

#### 【絶対遵守ルール (Constraints)】
*   **ロック期間の最小化**: `WorkerLoop` 内でキューから要素を取り出すためのミューテックスロック（`m_queueMutex`等）は、`filepath` を取り出したら直ちに解除すること。画像抽出やデコード処理、DB保存処理など時間のかかる処理をミューテックスを握ったまま絶対に実行しないこと。
*   **UIへの非干渉**: 本タスクの責務はバックグラウンドでのサムネイル生成と保存のみである。D2D描画ロジックやUI更新処理などは一切記述しないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Hotfix Phase 22-2 アーキテクチャ資料の更新
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1150_RES_Phase22-2_ThumbCacherCook.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは(タスク概要)である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、実装したコードと既存の作業レポート（YYYYMMDD_HHMM_RES_Phasex-x_(taskname).md）の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスクx」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「(tasknname)(Phase x-x)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`PROJECT_ARCHITECTURE.md` を更新し、Phase 22-2 で実装したサムネイル工場の設計仕様をクラスリファレンスへ反映する。極限までスリム化された資料の目的に従い、メソッドの詳細な引数などは書かず、責務と役割の概要のみを追記すること。

*   **要件1: クラスリファレンスの更新**
    *   `ThumbnailDatabase` クラスに、スレッドセーフなバイナリ追記機能と、セクタ情報（オフセットとサイズ）のメモリ同期機能が実装された旨を追記すること。
    *   `ThumbCacher` クラスに、ワーカースレッドによる「サムネイル工場」が稼働し、`TagManager`からの生バイナリ抽出、WICを用いたサムネイルサイズへの縮小・JPEG品質指定エンコード、およびデータベースへの非同期保存パイプラインが完成した旨を追記すること。
    *   `ConfigManager` クラスにサムネイルのJPEG品質を制御する `ThumbnailJpegQuality` が追加された旨を追記すること。
