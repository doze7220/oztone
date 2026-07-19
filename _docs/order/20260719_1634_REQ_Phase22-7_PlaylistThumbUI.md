### 作業指示書 REQ: Phase 22-7: プレイリスト一覧のアルバムジャケット化 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-7_PlaylistJacket.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、タスクを安全な粒度（ファイル単位等）に細かく分割したタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
現在、プレイリスト一覧モード（`.ozl` ファイル一覧）において、リストの左側に余白が存在している。この空間を利用し、プレイリストの「1曲目」のサムネイル画像をアルバムジャケットとして表示し、さらにデザインの統一感を出すための「数字なしのCD帯」を描画する。

*   **要件1: PlaylistSummary の拡張 (1曲目のパス保持)**
    *   `src/Application.h` (または該当箇所) の `PlaylistSummary` 構造体に `std::wstring firstTrackPath;` を追加する。
    *   `Application::UpdatePlaylistSummaries` (または該当のパース処理) において、TSVファイルからプレイリストを読み込む際、最初のデータ行からファイルパスを抽出し、`firstTrackPath` に保持させる。
*   **要件2: LayoutCalculator の再調整**
    *   先ほどHotfixで「プレイリスト一覧モード時は回避シフトを無効にする」としたロジックを再調整する。
    *   プレイリスト一覧モードであっても、サムネイル画像やCD帯が存在することを前提に、曲一覧モードと同様のテキスト描画開始X座標（右シフト）の回避ロジックを適用するように修正する。
*   **要件3: Widget_Playlist_DrawItems の改修 (サムネイルと数字なし帯の描画)**
    *   `DrawPlaylistList` (または該当のプレイリスト一覧描画ループ) 内において、`summary.firstTrackPath` を用いて Application 層（司令塔）から渡されたサムネイルデータベース等経由で画像を取得し、アスペクト比を維持して `thumbRect` へ描画する。
    *   曲一覧と同じCD帯（ボックスとアンダーライン）を描画する処理を追加する。ただし、文字（トラックナンバー）の描画処理はスキップし、「数字なしの帯（カラーバー）」としてデザインのアクセントのみを描画する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-7_PlaylistJacket.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **非同期エンキューの徹底**: 描画ループ内で新規サムネイルが必要（`isNew == true`）と判定された場合でも、Widget層から絶対に `ThumbCacher::EnqueueTrack` を呼び出してはならない（描画層は受動態を貫くこと）。エンキューは必ず Application 層で行う設計とするよう計画を立てること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-7 Task 1-2 : プレイリスト一覧のアルバムジャケット化（第1撃：データ拡張とエンキュー）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1634_RES_Phase22-7_PlaylistJacket.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはバックエンド拡張の第1フェーズである。直ちに以下の【実装要件】に従ってコードの修正を実行すること。描画ファイル（Widget）やレイアウト計算の修正であるタスク3以降は後続の指示で行うため、今は絶対に手を触れないこと。
2. 作業完了後、実装したコードがRESファイルに記載された「🚨 監査項目2」および「🚨 監査項目3」に抵触していないか、厳密にセルフコードレビュー（自己監査）を行うこと。
3. 監査完了後、作業レポート（20260719_1634_RES_Phase22-7_PlaylistJacket.md）の「タスク1」および「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「タスク1と2の実装および自己監査が完了しました。次の指示をお願いします」と報告すること。

#### 【実装要件】
*   **タスク1: PlaylistSummary 拡張とサムネイル非同期エンキュー (Application層)**
    *   対象ファイル: `src/WidgetContext.h`, `src/Application_Render.cpp`
    *   `WidgetContext.h` の `PlaylistSummary` 構造体に `std::wstring firstTrackPath;` を追加する。
    *   `Application_Render.cpp` の `UpdatePlaylistSummaries` (または TSVファイルからのプレイリストパース処理部分) において、リストから最初のデータ行（1曲目）を読み込んだ際、ファイルパスを抽出して `firstTrackPath` に設定する。
    *   同時に、抽出した `firstTrackPath` を用いて `m_thumbnailDatabase.GetOrGenerateThumbId(filepath, isNew)` を呼び出し、`isNew == true` の場合のみ `m_thumbCacher.EnqueueTrack(thumbId, filepath)` を実行してサムネイル生成の非同期発注（エンキュー）を行う。
*   **タスク2: Renderer Context の拡張 (描画層へのサムネイル提供)**
    *   対象ファイル: `src/Renderer_Context.cpp`
    *   `BuildRenderContext` 等のコンテキスト構築ループ内で、プレイリスト一覧モード用のデータをパックする際、`summary.firstTrackPath` を用いて `m_thumbnailDatabase.GetThumbnailBitmap` を呼び出し、サムネイル画像を取得する（キャッシュのみ参照、発注は絶対にしないこと）。
    *   取得した画像ポインタをコンテキストへ登録し、Widget層へ渡せるようにする。

#### 【絶対遵守ルール (Constraints)】
*   **憲法・ルール・責務の厳守**: 作業前に必ず冒頭で上げた資料を把握すること。
*   **発注連打の防止（監査項目3）**: タスク1の `EnqueueTrack` 処理は、プレイリストの一覧をパース・更新するタイミングでのみ発動し、毎フレーム実行されることがないように確認すること。
*   **描画構築層での発注禁止（監査項目2）**: タスク2の `Renderer_Context.cpp` 内部で `GetOrGenerateThumbId` や `EnqueueTrack` を絶対に呼び出さないこと。必要な画像はキャッシュからの取得（`GetThumbnailBitmap`）のみに留めること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-7 Task 3-4 : プレイリスト一覧のアルバムジャケット化（第2撃：UI描画とドキュメント）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1634_RES_Phase22-7_PlaylistJacket.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはフロントエンドUI改修の最終フェーズである。直ちに以下の【実装要件】に従ってコードおよびドキュメントの修正を実行すること。
2. 作業完了後、実装したコードがRESファイルに記載された「🚨 監査項目1（Widget 層の越権行為）」および「🚨 監査項目4（LayoutCalculator への状態混入）」に抵触していないか、厳密に自己監査を行うこと。
3. 監査完了後、作業レポート（20260719_1634_RES_Phase22-7_PlaylistJacket.md）の「タスク3」および「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「Phase 22-7 の全タスクが完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **タスク3: LayoutCalculator の再調整とプレイリスト一覧UIの描画改修 (Widget層)**
    *   対象ファイル: `src/LayoutCalculator.cpp`, `src/Widget_Playlist_DrawItems.cpp`
    *   `LayoutCalculator.cpp` の改修: 以前のHotfixで追加した「プレイリスト一覧モード時はテキストの右シフト（サムネイルとCD帯の回避）を無効化する」ロジックを再調整（撤廃）し、一覧モードであっても曲一覧モードと同様にテキストの開始X座標を右へシフトさせる計算を適用する。
    *   `Widget_Playlist_DrawItems.cpp` の改修: `DrawPlaylistList`（プレイリスト一覧描画ループ）内において、Application層から `ctx` を経由して渡された1曲目のサムネイル画像を、アスペクト比を維持して `layout.thumbRect` へ描画する（※Phase 22-6 で実装したアスペクト比維持ロジックを踏襲すること）。
    *   同ループ内にて、曲一覧と同じ「CD帯（ボックスとアンダーライン）」の描画処理を追加する。ただし、文字（トラックナンバー）の描画処理はスキップし、「数字なしの帯（カラーバー）」としてデザインのアクセントのみを描画すること。
*   **タスク4: ドキュメントの更新**
    *   対象ファイル: `PROJECT_ARCHITECTURE.md`
    *   プレイリスト一覧モードのUIに関する説明へ、「1曲目のサムネイルがアルバムジャケットとして表示され、デザインの統一感を出すための数字なしCD帯が追加された」旨を追記する。

#### 【絶対遵守ルール (Constraints)】
*   **憲法・ルール・責務の厳守**: 作業前に必ず冒頭で上げた資料を把握すること。
*   **Widget層の受動態徹底（監査項目1）**: `Widget_Playlist_DrawItems.cpp` の描画処理において、絶対に `#include "ThumbnailDatabase.h"` 等を行わないこと。画像データは必ず `ctx` の中にあるものだけを使用すること。
*   **計算の純粋性維持（監査項目4）**: `LayoutCalculator.cpp` の修正において、`static` 変数などの状態を一切持たせないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-7 Hotfix 2: 1曲目サムネイルのオンデマンド読み出し復旧
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1634_RES_Phase22-7_PlaylistJacket.md）の末尾に「HOTFIX 2」として作業内容を追記すること。
3. チャットにて「プレイリスト一覧のサムネイル読み出し処理を修正しました。再度ビルドと動作確認をお願いします」と報告すること。

#### 【実装要件】
プレイリスト一覧のサムネイルが「永遠にガラス板のまま」になる不具合を修正する。
事前の指示で「描画層での新規生成発注(ThumbCacher::EnqueueTrack)禁止」を厳格にルール化した結果、必要な画像バイナリをパックファイルからVRAMへ読み出すための「非同期ロード要求(ThumbnailDatabase::RequestThumbnailLoad)」まで一緒にスキップされてしまっていることが原因である。

*   **要件1: Renderer_Context.cpp のオンデマンド読み出し要求の復旧**
    *   対象ファイル: `src/Renderer_Context.cpp`
    *   `BuildRenderContext` 等における `isPlaylistListViewMode == true` 用のループ処理において、`m_thumbnailDatabase.GetCachedThumbnailBitmap(summary.firstTrackThumbId)` で取得した結果が `nullptr` だった場合、確実に `m_thumbnailDatabase.RequestThumbnailLoad(summary.firstTrackThumbId, m_d2dRenderTarget.Get(), m_wicFactory.Get());` （引数は既存の実装・シグネチャに合わせること）を呼び出して、VRAMへの読み込みを要求する処理を追加する。
    *   ※ 注意: `ThumbCacher::EnqueueTrack` は絶対に追加しないこと（アーキテクチャ維持）。純粋なデータベースからのロード要求のみを行うこと。

