### 作業指示書 REQ: Phase 22-1: ThumbnailDatabaseとThumbCacherの基盤構築（計画立案）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-1_ThumbnailEngine.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
ユーザーが任意にVRAM使用量やキャッシュサイズを制御できる設定値の追加と、画像バイナリをパックファイル（idx + img）として永続化・管理する `ThumbnailDatabase` クラス、および非同期でキャッシュを生成する専用ルーチン `ThumbCacher` クラスの骨組みを新規構築する。

*   **要件1: ConfigManager の拡張**
    *   `[System]` セクションに以下を追加し、ゲッター・セッター・INI読み書き処理を実装する。
        *   `MaxThumbnailCache` （デフォルト: 100）: VRAMに保持する最大サムネ数
        *   `ThumbnailSize` （デフォルト: 120）: キャッシュ生成サイズ（UIのArtSizeとは分離）

*   **要件2: ThumbnailDatabase と ThumbCacher の新設**
    *   `src/ThumbnailDatabase.h` / `.cpp` および `src/ThumbCacher.h` / `.cpp` を新規作成し、`CMakeLists.txt` に追加する。
    *   **ThumbnailDatabase**: 
        *   管理ファイルとして `oztone_track_thumb_idx.odb` と `oztone_track_thumb_img.odb` を使用する。
        *   初期化時に `idx` の1行目（ヘッダ）を読み、マジックナンバー(`OZTHUMB_V1`)と現在の `ThumbnailSize` を比較する。不一致・破損時は既存の `idx` と `img` をファイルサイズ0で上書き（trunc）し、新しいヘッダを書き込んで全破棄する自己修復ロジックを実装する。
        *   UIからのアクセス用APIとして、ファイルパスから整数ハンドルを返す `uint32_t GetThumbnailId(const std::wstring& filepath)` と、整数ハンドルベースで描画をディスパッチする `void DrawThumbnail(ID2D1DeviceContext* context, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity)` を定義する。
        *   内部に `MaxThumbnailCache` を上限とする LRUキャッシュの枠組みを用意する。
    *   **ThumbCacher**: 
        *   `TrackAnalyzer` とは完全に分離した、サムネイル専用のバックグラウンド生成（クック）スレッドを管理するクラスの骨組み（スレッド起動、終了、キュー管理など）を作成する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-1_ThumbnailEngine.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **WIC/D2Dの直接操作は次タスク以降**: 本タスクでは「Configの拡張」「ファイルのI/O管理（自己修復）」「スレッドとキャッシュの枠組み」の構築に専念すること。WICを用いた実際のリサイズ処理や描画連携は後続タスクで行うため、関数定義などに留めてよい。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-1 Task 1 : ConfigManagerの拡張
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1101_RES_Phase22-1_ThumbnailEngine.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジン用設定の追加である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1101_RES_Phase22-1_ThumbnailEngine.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「ConfigManagerの拡張(Phase 22-1 Task 1)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
ユーザーが任意にVRAM使用量やキャッシュサイズを制御できる設定値を追加する。

*   **要件1: ConfigManager の拡張**
    *   `src/ConfigManager.h`, `src/ConfigManager_System.cpp`, `src/ConfigManager_DefaultIni.h` を修正し、`[System]` セクションに以下を追加する。
        *   `MaxThumbnailCache` （デフォルト: 100）: VRAMに保持する最大サムネイル数（整数）
        *   `ThumbnailSize` （デフォルト: 120）: バックグラウンドで生成するサムネイルの基本サイズ（浮動小数点数）
    *   それぞれのゲッター（`GetMaxThumbnailCache`, `GetThumbnailSize`）・セッター、およびINI読み書き処理（`LoadSystemSettings`, `Set...` 内での保存処理等）を実装する。

#### 【絶対遵守ルール (Constraints)】
*   **責務分離の原則**: 今回追加する設定は `[System]` セクションに属するため、ファイルI/Oの実装は必ず `ConfigManager_System.cpp` 内に記述すること。他のUI設定セクションには干渉しないこと。


-------------------------------------------------------------------------------

### 作業指示書 REQ: Hotfix_Warning_C4267 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_Warning_C4267.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
ビルド時に発生している以下の Warning (C4267) を解消する。

*   **対象のエラーログ:**
    `[build] D:\ozlab\oztone\src\Renderer_Context.cpp(96,67): warning C4267: '=': 'size_t' から 'int' に変換しました。データが失われているかもしれません。`
*   **対象コード:**
    `ctx.animatingTargetIndex = m_trackDrum.GetAnimatingTargetIndex();`
*   **修正方針:**
    `GetAnimatingTargetIndex()` の戻り値の型と、`WidgetContext` (src/WidgetContext.h) 内の `animatingTargetIndex` の型を調査し、不整合を解消すること。
    どちらかの型に統一するか、安全であることが自明な場合は `static_cast<int>` 等を用いて明示的なキャストを行い、警告を抑制すること。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の最小化**: 型の変更を行う場合、他のロジック（特に相対インデックス計算の符号など）に悪影響が出ないよう注意深く判断すること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-1 Task 2 : ThumbnailDatabase - パックファイルI/Oと自己修復
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1101_RES_Phase22-1_ThumbnailEngine.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンのファイルI/O基盤構築である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1101_RES_Phase22-1_ThumbnailEngine.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク2が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
画像バイナリを単一のパックファイル群として永続化・管理する `ThumbnailDatabase` クラスの基礎を構築し、ヘッダ検証によるキャッシュの自己修復（全破棄）ロジックを実装する。

*   **要件1: ThumbnailDatabase クラスの新設**
    *   `src/ThumbnailDatabase.h` と `src/ThumbnailDatabase.cpp` を新規作成する。
    *   コンストラクタで `ConfigManager` のポインタを受け取り、メンバに保持する。
*   **要件2: 管理ファイルの定義と初期化**
    *   実行ファイルと同階層に生成する管理ファイルとして `oztone_track_thumb_idx.odb` と `oztone_track_thumb_img.odb` のパス文字列を定義する。
    *   `Initialize()` メソッドを作成する。
*   **要件3: ヘッダ検証と無慈悲な自己修復 (物理フォーマット)**
    *   `Initialize()` 内で、`idx` ファイルを開き、1行目（ヘッダ）を読み込む。
    *   ヘッダフォーマット: `マジックナンバー (OZTHUMB_V1) \t ThumbnailSize`
    *   `ConfigManager` から取得した現在の `ThumbnailSize` と、ファイルに記録されているサイズを比較する。
    *   **不一致、ファイルが存在しない、または破損している場合**: 
        既存の `idx` と `img` をファイルサイズ0で上書き（`std::ios::trunc`）して既存キャッシュを全破棄する。その後、`idx` ファイルの先頭に新しい正しいヘッダを書き込む。
    *   **一致した場合**: そのまま安全にファイルをクローズする（シーク位置の構築等は後続タスクで行うため、現時点では検証のみでよい）。

#### 【絶対遵守ルール (Constraints)】
*   **ファイルI/Oの安全性**: `std::fstream` を用いたファイルオープン・クローズは確実に行い、例外やクラッシュが発生しない堅牢な実装とすること。
*   **メモリ管理・UI連携の禁止**: 本タスクは「物理ファイルのI/Oとバージョニング」のみに専念する。LRUキャッシュ（メモリ管理）や描画用のAPIは次のタスク3で実装するため、今回は記述しないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-1 Task 3 : ThumbnailDatabase - LRUキャッシュとUIインターフェース構築
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1101_RES_Phase22-1_ThumbnailEngine.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジンのメモリ管理およびインターフェース構築である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1101_RES_Phase22-1_ThumbnailEngine.md）の「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク3が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
`ThumbnailDatabase` に、設定値 `MaxThumbnailCache` を上限とする LRU (Least Recently Used) キャッシュ機構と、UI層から文字列を使わずにアクセスするための「サムネイルID（整数ハンドル）」ベースの描画ディスパッチャを実装する。

*   **要件1: LRUキャッシュとID管理のデータ構造**
    *   `src/ThumbnailDatabase.h` を拡張する。
    *   ファイルパスと整数ID（`uint32_t`）を相互変換するための `std::unordered_map` を追加する。
    *   D2Dビットマップ（サムネイル画像）を保持する LRUキャッシュ機構として、`std::list<uint32_t>`（アクセス順序の管理）と `std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID2D1Bitmap>>`（実際のデータ保持）を追加する。
*   **要件2: UIアクセス用APIの追加**
    *   `uint32_t GetThumbnailId(const std::wstring& filepath);` を実装する。まだIDが発行されていないファイルパスには新規にIDを割り当てて返す。
    *   `void DrawThumbnail(ID2D1DeviceContext* context, uint32_t thumbId, const D2D1_RECT_F& destRect, float opacity);` を実装する。
*   **要件3: DrawThumbnail 内部の描画ディスパッチ（枠組み）**
    *   `DrawThumbnail` の内部で、以下の3分岐の枠組みを実装する。
        1. **LRUキャッシュに存在する場合**: リストの先頭（最近使われた）に移動させ、その `ID2D1Bitmap` を指定された矩形へ描画する。
        2. **キャッシュには無いが、idx（セクタ情報）には存在する場合**: パックファイルからのデコードが必要。※実際のWICデコード処理は後続タスクで行うため、ここでは `// TODO: パックファイルからデコードしてLRUへ登録` というコメントのみ残し、何も描画せずに抜けるかダミー処理に留める。
        3. **まだ解析・クック中（どちらにも無い）場合**: `context->FillRectangle` と半透明ブラシ（仮に白の不透明度0.1等）を用いて「ガラス板（プレースホルダー）」を描画する。

#### 【絶対遵守ルール (Constraints)】
*   **責務分離の原則**: `DrawThumbnail` 以外の関数（UI層の `TrackDrum` など）が画像のロード状態やキャッシュの有無を判定してはならない。状態分岐はすべて `DrawThumbnail` の内部に隠蔽すること。
*   **WICデコードの実装禁止**: 本タスクではバイナリデータからの画像復元ロジックは記述しないこと。あくまで「LRUキャッシュの押し出し機構」と「3分岐の枠組み」の構築に専念する。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-1 Task 4 : ThumbCacher - サムネイル生成スレッド基盤の構築
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1101_RES_Phase22-1_ThumbnailEngine.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはサムネイルエンジン専用のバックグラウンドスレッド基盤構築である。直ちに以下の【実装要件】に従ってコードの追加を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1101_RES_Phase22-1_ThumbnailEngine.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク4が完了しました。ビルド確認をお願いします」と報告すること。

#### 【実装要件】
テキストタグ解析（`TrackAnalyzer`）とは完全に分離した、サムネイル専用のバックグラウンド生成（クック）スレッドを管理する `ThumbCacher` クラスの骨組みを構築する。

*   **要件1: ThumbCacher クラスの新設**
    *   `src/ThumbCacher.h` と `src/ThumbCacher.cpp` を新規作成する。
    *   コンストラクタで `ThumbnailDatabase` のポインタを受け取り、メンバ変数に保持する（クックしたバイナリをDBへ渡すため）。
*   **要件2: スレッドとタスクキューの管理基盤構築**
    *   `std::thread`、`std::mutex`、`std::condition_variable`、タスクキュー（`std::queue<std::wstring>` 等のファイルパスを保持するキュー）、およびスレッド終了フラグ（`std::atomic<bool>`）をメンバとして追加する。
    *   スレッドを安全に起動する `Initialize()` と、安全に停止・ジョインする `Uninitialize()`（またはデストラクタ処理）を実装する。
*   **要件3: ワーカースレッドループとタスク追加API**
    *   外部からクック対象のファイルパスをキューに追加する `void EnqueueTrack(const std::wstring& filepath)` メソッドを実装し、キュー追加後に `condition_variable` でスレッドを起床させる。
    *   ワーカースレッドの本体ループ（キューからファイルパスを取り出し、空になるまで処理を続けるループ）の枠組みを実装する。
    *   ※実際のWICデコードおよびリサイズ処理、DBへの書き込み処理は後続タスクで行うため、スレッドループ内の実処理部分は `// TODO: WICでサムネイル生成しThumbnailDatabaseへ追記` などのコメントに留めるか、ダミーの待機処理にしておくこと。

#### 【絶対遵守ルール (Constraints)】
*   **責務分離の原則**: 本タスクでは「スレッドとキューの安全な運用ロジック」のみを実装すること。WIC関連のCOM初期化や画像処理は記述しない。
*   **スレッドセーフな設計**: キューの読み書きや終了フラグの制御において、データ競合やデッドロックが絶対に発生しないように排他制御を徹底すること。

