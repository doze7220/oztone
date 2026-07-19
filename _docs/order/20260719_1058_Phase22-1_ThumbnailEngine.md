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

