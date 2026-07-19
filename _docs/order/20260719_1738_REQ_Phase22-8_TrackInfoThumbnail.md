### 作業指示書 REQ: Phase 22-8: トラックインフォのサムネイル純化と自己修復の軽量化 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-8_TrackInfoThumbnail.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
現在、曲切り替え時の自己修復処理（UpdateTrackMetadataIfNeeded）やドラムスロットへのデータ供給において、フル解像度のアルバムアートが不要に抽出・セットされ、パフォーマンスとVRAMを圧迫している。トラックインフォは「TrackDatabase(ODB) と ThumbnailDatabase の確認窓」に特化させるため、画像供給ラインを完全に分離・純化する。

*   **要件1: 自己修復ロジックからの「画像抽出」のパージ**
    *   対象ファイル: `src/Application_Playback.cpp` (または該当箇所)
    *   `UpdateTrackMetadataIfNeeded` 内部で TagManager を用いてタグを読み込む際、テキスト情報（曲名・アーティスト名・曲時間等）のみを取得・比較し、フル解像度画像の抽出（`artBitmap`のデコードなど）が行われている場合はその処理を完全に削除して軽量化する。
*   **要件2: ドラムスロットへのサムネイル画像直結 (dataProviderの純化)**
    *   対象ファイル: `src/Application_Playback.cpp` (PlayCurrentTrack 等)
    *   `StartDrumAnimation` に渡す `dataProvider` ラムダ式内部において、フル解像度画像（`artBitmap`）を `DrumSlot` にセットしている処理を削除する。
    *   代わりに、対象トラックの `thumbId`（メタデータから取得、または発番）を用いて `m_thumbnailDatabase.GetCachedThumbnailBitmap(thumbId)` を呼び出し、取得したサムネイル画像を `DrumSlot.artBitmap` へ直接渡すように修正する。
    *   ※キャッシュにない場合（nullptr）は、`m_thumbnailDatabase.RequestThumbnailLoad` を呼び出してVRAMロードを要求し、サムネイルDBの仕組みに完全に乗っかること。
*   **要件3: 背景アートとの分離確認**
    *   フル解像度の画像デコードと背景へのセット（`SetBackgroundArt`）は、ドラムスロットのデータ供給からは完全に独立させ、背景描画専用の処理として維持・隔離すること。現時点ですでにその処理が入っているので、それを邪魔しないこと。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-8_TrackInfoThumbnail.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **フル解像度画像のパージ**: ドラムスロット（`DrumSlot`）には、絶対にフル解像度の画像ポインタを渡してはならない。常にサムネイル画像のみを供給すること。
*   **コード修正の禁止**: 本フェーズは計画立案であるため、ソースコードへの変更は一切行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-8 Task 1: 自己修復ロジック（UpdateTrackMetadataIfNeeded）からの画像抽出パージ
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは「自己修復ロジックからの不要な画像抽出処理の削除」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「自己修復ロジックからの画像抽出パージ(Phase 22-8 Task 1)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在、曲切り替え時の自己修復処理（`UpdateTrackMetadataIfNeeded`）において、テキスト情報だけでなくフル解像度のアルバムアート画像まで抽出・取得しようとしている。トラックインフォは今後サムネイルDBのみを参照する構造へ純化させるため、このメソッド内での重い画像抽出処理を完全にパージして軽量化する。

*   **要件1: UpdateTrackMetadataIfNeeded の軽量化（画像パージ）**
    *   対象ファイル: `src/Application_Playback.cpp`
    *   `UpdateTrackMetadataIfNeeded` 内部で `TagManager` を用いてタグを読み込む際、フル解像度画像の抽出・デコード（`artBitmap`等）が行われている場合は、その処理を完全に削除（スキップ）する。
    *   曲名・アーティスト名・曲時間等のテキスト情報、およびサムネイルIDの確認・更新を行うロジックはそのまま維持・保護すること。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の最小化 (スコープ厳守)**: 本タスクは「不要な抽出ロジックを消す（パージする）」ことのみを目的とする。サムネイルデータベースの取得処理追加（結線）などは次タスクで行うため、この段階では新たな画像ロード処理等を書き加えないこと。
*   **既存テキスト修復ロジックの保護**: 画像の抽出処理を削除する過程で、既存のテキストメタデータの比較・自己修復ロジックを誤って破壊しないよう細心の注意を払うこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-8 Task 2: ドラムスロットへのサムネイル画像直結 (dataProviderの純化)
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはドラムスロットへのサムネイル画像結線の切り替えである。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「dataProviderの純化とサムネイル直結(Phase 22-8 Task 2)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在、`Application::PlayCurrentTrack` から `StartDrumAnimation` へ渡される `dataProvider` コールバックの内部で、フル解像度のアルバムアートがそのまま `DrumSlot` にセットされてトラックインフォへと流し込まれている。このパイプラインを完全に切り離し、トラックインフォを「サムネイルDBの確認窓」へと純化させる。

*   **要件1: フル解像度画像のパージとサムネイルDBへの結線**
    *   対象ファイル: `src/Application_Playback.cpp` (`PlayCurrentTrack` 等)
    *   `StartDrumAnimation` に渡す `dataProvider` ラムダ式内部において、フル解像度の画像（`artBitmap`など）を `DrumSlot` にセットしている処理を **完全に削除** する。
    *   代わりに、対象トラックのファイルパスを用いて `m_thumbnailDatabase.GetOrGenerateThumbId(track.filepath, isNew)` で `thumbId` を取得する（※新規発番であっても絶対に `EnqueueTrack` は呼ばないこと）。
    *   取得した `thumbId` を用いて `m_thumbnailDatabase.GetCachedThumbnailBitmap(thumbId)` を呼び出し、取得できたサムネイル画像を `DrumSlot` の `artBitmap` へセットするように修正する。
    *   キャッシュに画像が存在しない場合（`nullptr` の場合）は、`m_thumbnailDatabase.RequestThumbnailLoad(thumbId, track.filepath, m_renderer.GetRenderTarget(), m_renderer.GetWicFactory())` を呼び出してバックグラウンドでのVRAMロードを要求すること。

#### 【絶対遵守ルール (Constraints)】
*   **背景描画の保護**: 背景アート用のフル解像度画像読み込みとセット（`SetBackgroundArt` 等）は、すでに独立したライフサイクルで実装されているため、今回のドラムスロット側の改修で誤って削除・破壊しないこと。
*   **発注権限（エンキュー）の禁止**: `dataProvider` 内は単なるデータ供給（描画の準備）フェーズであるため、未登録の曲であっても `m_thumbCacher.EnqueueTrack` を呼び出してサムネイル工場へ新規生成を発注してはならない。工場への発注権限はリスト更新時等の司令塔にのみ許される。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-8 Task 3: 裏スレッド（TrackAnalyzer）からのフル解像度画像抽出パージ
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料: D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書: D:\ozlab\oztone\_docs\logs\20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md

#### 【作業手順（厳守事項）】
本プロンプトは裏スレッドによる画像上書きの防止タスクである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md）を読み、今回の自分のスコープが「タスク3」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
4. 作業完了後、既存の作業レポートの「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
5. チャットにて「TrackAnalyzerからの画像抽出パージ(Phase 22-8 Task 3)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在、バックグラウンドの解析スレッド（`TrackAnalyzer` 等）においてタグ情報を読み込む際、フル解像度のアルバムアートまで抽出してしまい、それがメインスレッドのUIへ強制上書きされてしまう問題が発生している。これを完全に防ぐため、裏スレッドでの画像抽出をパージする。

*   **要件1: TrackAnalyzer からの画像抽出パージ**
    *   対象ファイル: `src/TrackAnalyzer.cpp` （または裏スレッドでの解析処理を行っているファイル）
    *   バックグラウンドでのタグ解析処理（`TagManager::Load` の呼び出し等）において、画像抽出をスキップするフラグ（タスク1で追加した `skipImage = true` など）を確実に渡し、フル解像度画像の抽出を完全に無効化する。
    *   波形スキャンやテキスト情報の解析ロジックはそのまま維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の制限 (スコープ厳守)**: 今回の目的は「裏スレッドによる画像の上書きを防ぐ（犯人の確保）」ことのみである。UI側へのサムネイルのポーリング機構の導入やその他の描画ロジックの修正は絶対に行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-8 Task 4: サムネイルポーリング機構の導入と上書きパージ
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料: D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書: D:\ozlab\oztone\_docs\logs\20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md

#### 【作業手順（厳守事項）】
本プロンプトはサムネイル画像の表示保証および不要な上書きパージタスクである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md）を読み、現在の状況と今回の自分のスコープが「タスク4」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
4. 作業完了後、既存の作業レポートの「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
5. チャットにて「サムネイルポーリング機構の導入(Phase 22-8 Task 4)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在、波形解析完了時などにUIを最新化する際、メインスレッドが古いフル解像度画像をUIに強制上書きしてしまう問題、および非同期ロード完了後に画像がUIへ反映されない（ポーリング不足の）問題が発生している。これらを完全に解決する。

*   **要件1: メタデータ更新時の画像上書きの完全パージ**
    *   対象ファイル: `src/Application_Playback.cpp`、`src/Application_Render.cpp`、またはメタデータ更新を行っているファイル
    *   波形解析完了やタグ修復時などに `m_renderer.UpdateCurrentDrumSlot` （またはそれに類するUI更新メソッド）を呼び出している箇所を特定する。
    *   その際、フル解像度画像（`TagManager`のキャッシュなど）を取得して引数に渡している処理を **完全に削除** する。画像部分は更新しない（既存のまま維持する）か、`nullptr` を渡す等して、フル解像度画像がトラックインフォへ流入するパイプラインを完全に断ち切ること。

*   **要件2: 毎フレームのサムネイルポーリング機構の導入**
    *   対象ファイル: `src/Application_Render.cpp` (`ForceRender` 内など)
    *   毎フレームの描画前処理において、現在再生中の曲のパスから `m_thumbnailDatabase.GetOrGenerateThumbId` で `thumbId` を取得する（※新規発番時は絶対にエンキューしないこと）。
    *   取得した `thumbId` を用いて `m_thumbnailDatabase.GetCachedThumbnailBitmap(thumbId)` を呼び、キャッシュにサムネイル画像が存在すれば、`m_renderer.UpdateCurrentDrumSlot` 等を用いて確実にトラックインフォの画像としてUIへ流し込み続けるポーリング（監視・反映）機構を実装する。
    *   これにより、非同期ロードが完了した瞬間に画像がUIに反映される真の受動態を構築する。

#### 【絶対遵守ルール (Constraints)】
*   **エンキューの禁止**: ポーリング処理内で `GetOrGenerateThumbId` を呼ぶ際、新規発番であっても絶対に `m_thumbCacher.EnqueueTrack` を呼び出してはならない。発注権限はリスト更新時等の司令塔にのみ許される。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-8 HOTFIX 2: ドラムスクロール中サムネイルの完全表示 (DrumSlot連動ポーリング)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料: D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  参考資料: D:\ozlab\oztone\_docs\logs\20260719_1751_RES_Phase22-8_TrackInfoThumbnail.md (※不要な場合はこの行を削除)

#### 【作業手順（厳守事項）】
本プロンプトはドラムスクロール中の中間曲サムネイル表示を完全に機能させるHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、既存の作業レポートの末尾に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「スクロール中サムネイルの表示対応(HOTFIX 2)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
タスク4で導入したポーリング機構が「最終目的地の曲」を直接見てしまっているため、ドラムスクロール中にすべて同じサムネイルが表示されてしまう問題を解決し、中間曲のサムネイルもスクロール中に正しく表示させる（当初からの目的である完全なサムネイル機能を実現する）。

*   **要件1: DrumSlot への thumbId 保持**
    *   対象ファイル: `src/WidgetContext.h`
    *   `DrumSlot` 構造体に `uint32_t thumbId = 0;` を追加する。
*   **要件2: dataProvider およびスロット更新時の thumbId セット**
    *   対象ファイル: `src/Application_Playback.cpp` 等
    *   `PlayCurrentTrack` 等から `StartDrumAnimation` に渡される `dataProvider` ラムダ式内において、対象曲のパスから `m_thumbnailDatabase.GetOrGenerateThumbId(track.filepath, isNew)` でIDを取得し（※新規発番時でも絶対にエンキューはしない）、引数で渡される `DrumSlot` の `thumbId` にセットする。
*   **要件3: ポーリング機構の TrackDrum (DrumSlot) 連動化**
    *   対象ファイル: `src/Application_Render.cpp`, `src/Renderer.cpp`
    *   タスク4で `ForceRender` に追加された「最終目的地の曲パス」を用いたポーリング処理を削除する。
    *   代わりに、Renderer 内部（毎フレームの更新や描画前処理等）において、自身が保持している2つの `DrumSlot`（カレントおよびOLD）の `thumbId` を個別に確認するポーリング機構を実装する。
    *   各スロットにおいて、`thumbId` が有効（0以外）かつ `artBitmap` が空（`nullptr`）の場合、`GetCachedThumbnailBitmap(thumbId)` でキャッシュから取得を試み、取得できたら `artBitmap` にセットして UI へ即座に反映されるようにする。

#### 【絶対遵守ルール (Constraints)】
*   **エンキューの禁止**: ポーリング処理内で `GetOrGenerateThumbId` を呼ぶ際、新規発番であっても絶対に `m_thumbCacher.EnqueueTrack` を呼び出してはならない。
