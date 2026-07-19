### 作業指示書 REQ: Phase 22-6: プレイリストサムネイル描画の修復とINI制御化 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-6_PlaylistThumbUI.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、タスクを安全な粒度（1タスクにつき1〜2ファイル程度）に細かく分割したタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
現在、プレイリストのサムネイル表示において、「アスペクト比が崩れて正方形に潰れる」「レイアウトがハードコードされている」という不具合が存在しているため、以下の改修を行う。

*   **要件1: サムネイルレイアウトのINI制御化 (ConfigManager の拡張)**
    *   `ConfigManager_Playlist.cpp` および関連ファイルを修正し、`[Layout_Playlist]` セクションにサムネイル用のパラメータとして `PlaylistThumbSize`, `PlaylistThumbOffsetX`, `PlaylistThumbOffsetY` の3つを追加し、読み書きとゲッターを実装する。※不透明度の設定は不要。
*   **要件2: LayoutCalculator の改修 (動的レイアウト)**
    *   `CalculatePlaylistItemLayout` にて、ConfigManager の設定値を参照して `thumbRect` の位置とサイズを計算する。
    *   サムネイル領域の幅（`PlaylistThumbSize`）とオフセット（`PlaylistThumbOffsetX`）を考慮し、既存のテキスト（タイトル、アーティスト、再生時間）がサムネイル画像と被らないように描画開始X座標を動的に右へシフト（回避）させる計算ロジックを実装する。
*   **要件3: アスペクト比を維持した描画 (Widget_Playlist_DrawItems.cpp)**
    *   アイテム描画ループ内において、単純に `thumbRect` へ `DrawBitmap` を行うのではなく、取得したビットマップの元のサイズ（`bitmap->GetSize()`）を取得する。
    *   画像のアスペクト比（縦横比）を計算し、`thumbRect` の領域内にアスペクト比を維持したままフィット（長辺基準での内接、または中央配置でトリミング）させて描画する計算ロジックへと改修する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-6_PlaylistThumbUI.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **タスクの細分化**: UIレイアウト計算と描画ロジックの変更が伴うため、AIのコンテキスト保護の観点から、タスクをファイル単位（ConfigManager, LayoutCalculator, Widget）で十分に細分化して計画を立てること。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-6 Task 1-2 : プレイリストサムネイル描画の修復とINI制御化（第1撃：設定とレイアウト計算）
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1604_RES_Phase22-6_PlaylistThumbUI.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはフロントエンドUI修復の第1フェーズである。直ちに以下の【実装要件】に従ってコードの修正を実行すること。描画ファイル（Widget）の修正であるタスク3以降は後続の指示で行うため、今は絶対に手を触れないこと。
2. 作業完了後、実装したコードと既存の作業レポート（20260719_1604_RES_Phase22-6_PlaylistThumbUI.md）のタスク1およびタスク2の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスク1」および「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「タスク1とタスク2の実装が完了しました。次の指示をお願いします」と報告すること。

#### 【実装要件】
*   **タスク1: ConfigManager の拡張 (サムネイルレイアウトのINI制御化)**
    *   対象ファイル: `src/ConfigManager.h`, `src/ConfigManager_Playlist.cpp`, `src/ConfigManager_DefaultIni.h`
    *   `[Layout_Playlist]` セクションに以下のパラメータを追加し、読み書きとゲッターを実装する。
        *   `PlaylistThumbSize` (デフォルト: 32.0f)
        *   `PlaylistThumbOffsetX` (デフォルト: 5.0f)
        *   `PlaylistThumbOffsetY` (デフォルト: 5.0f)
    *   ※ 不透明度の設定は不要。`ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` への追記も忘れずに行うこと。
*   **タスク2: LayoutCalculator の改修 (動的レイアウトと回避ロジック)**
    *   対象ファイル: `src/LayoutCalculator.h`, `src/LayoutCalculator.cpp`
    *   `PlaylistItemLayout` 構造体に `D2D1_RECT_F thumbRect;` を追加する。
    *   `CalculatePlaylistItemLayout` にて、ConfigManager から取得した値を用いて `thumbRect` を算出する（例: `left = startX + PlaylistThumbOffsetX`, `right = left + PlaylistThumbSize` など）。
    *   サムネイル画像とテキストが被らないよう、既存のタイトル・アーティスト名・再生時間の描画開始X座標（`startX`）を、`PlaylistThumbSize` と `PlaylistThumbOffsetX` （およびマージン）を考慮して動的に右へシフトさせる回避ロジックを組み込む。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本REQの目的は「設定の読み込みとレイアウトの計算ロジック」のみである。`Widget_Playlist_DrawItems.cpp` の描画処理にはまだ絶対に触れないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 22-6 Task 3-4 : プレイリストサムネイル描画の修復とINI制御化（第2撃：アスペクト比維持描画）
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_1604_RES_Phase22-6_PlaylistThumbUI.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはフロントエンドUI修復の最終フェーズである。直ちに以下の【実装要件】に従ってコードおよびドキュメントの修正を実行すること。
2. 作業完了後、実装したコードと既存の作業レポート（20260719_1604_RES_Phase22-6_PlaylistThumbUI.md）の実装要件を照らし合わせ、反映漏れがないか厳密に自己監査を行うこと。
3. 監査完了後、作業レポートの「タスク3」および「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
4. チャットにて「Phase 22-6 の全タスクが完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **タスク3: Widget_Playlist_DrawItems の改修 (アスペクト比を維持したフィット描画の実装)**
    *   対象ファイル: `src/Widget_Playlist_DrawItems.cpp`
    *   アイテム描画ループ内において、サムネイル描画時に `layout.thumbRect` へ直接 `DrawBitmap` を行っている箇所を改修する。
    *   `ctx.playlistThumbnails` から取得した `ID2D1Bitmap` の元のサイズ（`bitmap->GetSize()`）を取得し、元画像のアスペクト比（縦横比）を算出する。
    *   `layout.thumbRect` の領域内に、元画像のアスペクト比を維持したまま長辺を基準にして内接フィット（中央配置）させるための描画用矩形（`destRect`）を計算するロジックを実装する。
    *   算出した `destRect` を用いて `DrawBitmap` を呼び出し、サムネイルが正方形に潰れることなく正しい比率で描画されるようにする。
*   **タスク4: PROJECT_ARCHITECTURE.md の更新**
    *   対象ファイル: `PROJECT_ARCHITECTURE.md`
    *   プレイリストの仕様や `ConfigManager` の説明等へ、追加されたサムネイル設定パラメータ（`PlaylistThumbSize`, `PlaylistThumbOffsetX`, `PlaylistThumbOffsetY`）について追記し、サムネイルがアスペクト比を維持して描画されるようになった旨を反映させること。

#### 【絶対遵守ルール (Constraints)】
*   **アスペクト比計算の厳格化**: いかなる縦横比の画像（横長、縦長、正方形）が渡されても、`layout.thumbRect` の枠からはみ出さず、かつ比率を崩さずに中央へ配置されるよう、`width` と `height` のスケールを正しく比較計算すること。
