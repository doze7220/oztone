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

