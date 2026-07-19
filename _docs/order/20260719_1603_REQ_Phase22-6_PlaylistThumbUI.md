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
