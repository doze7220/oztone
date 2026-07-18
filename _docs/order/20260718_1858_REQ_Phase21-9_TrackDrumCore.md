### 作業指示書 REQ: Phase 21-9: 完全独立型トラックドラムエンジンの構築
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\order\20260718_1858_REQ_Phase21-9_TrackDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-9_TrackDrumRebuild.md）として新規作成すること。
3. **【重要】後続の実装AIが過去の肥大化したレポート（Phase 21-8 RES）を一切読まずとも作業が完結できるよう、パージ（削除）すべき古いハック実装（具体的な変数名や処理内容）を本計画書内に詳細に明記し、この計画書単体で情報が完結する（Single Source of Truthとなる）ように作成すること。**
4. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
プレイリストのデータ構造からトラックドラムの描画データを完全に切り離し、ドラム自身が独立したスナップショットデータと操作意図を持つ「最強のバケツリレーアーキテクチャ」を構築する。
※本フェーズのベースラインはPhase 21-8のHotfix 4完了時点とする。

*   **要件1: ドラム専用データ構造と操作意図の定義**
    *   `Renderer.h` 等に、描画に必要な純粋なデータのみを持つ構造体 `DrumSlotData` (曲名、アーティスト、トラックNo文字列、画像ビットマップ等) を定義する。
    *   ドラムの回転意図を指示する列挙型 `DrumMoveType` (`Next`, `Prev`, `Jump`, `CrossPlaylist`, `Reset` 等) を定義し、`SetTrackInfo` の引数として受け取れるようインターフェースを改修する。
*   **要件2: Application層からの操作意図伝達**
    *   `Application` 層の各操作（メディアキー、ジャンプ、リスト切り替え、UIクリア等）において `SetTrackInfo` を呼び出す際、単に曲情報を渡すだけでなく、適切な `DrumMoveType` を引数として渡すように連携させる。
*   **要件3: 独立したバケツリレーと位置リセットの構築**
    *   `Renderer` 内に `m_oldDrumSlot` と `m_nowDrumSlot` を実体（コピー）として保持する。
    *   `SetTrackInfo` 呼び出し時、渡された `DrumMoveType` に応じて現在位置（`m_drumPosition`）を正確にワープさせる。
        *   `Next` または `CrossPlaylist`: `m_drumPosition = 目標 - 1.0f` (1つ手前から順方向へ進む)
        *   `Prev`: `m_drumPosition = 目標 + 1.0f` (1つ先から逆方向へ戻る)
        *   `Jump`: インデックス差分による遠距離ジャンプ（現状維持）
    *   同時に、`m_nowDrumSlot` を `m_oldDrumSlot` へ完全にコピーしてデータを退避させる、絶対的なバケツリレーを確立する。
*   **要件4: Widget_TrackInfo の描画純化**
    *   仮想スロット描画ループにおいて、OLDスロットとNOWスロットを描画する際は絶対にプレイリストデータ（`ctx.shuffleMetadataList` 等）のインデックスを参照せず、`ctx` を経由して渡された `oldDrumSlot` と `nowDrumSlot` の実体のみを使用してテキストと画像を描画するように純化する。

#### 【絶対遵守ルール (Constraints)】
*   **完全な独立**: ドラムのOLD/NOWデータは、プレイリストが破棄・再構築されても絶対に消えない完全に独立したメモリ（実体のコピー）として管理すること。
