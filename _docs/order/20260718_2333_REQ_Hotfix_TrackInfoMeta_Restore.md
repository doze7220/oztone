##### 作業指示書 REQ: スクロール中のメタ情報消失とOLDハードコード修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-9_DoubleBuffering.md（該当不具合を実装した作業レポート)

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_TrackInfoMeta_Restore.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: 描画フローからの i=x 条件分岐の完全撤廃 (カメラの純化)**
    *   `src/Widget_TrackInfo.cpp` の描画ループ内から、「`i == 0` ならカレント用の描画」「`i == -1` ならOLD用の描画」といった、インデックスに基づく描画フローの `if` 分岐を完全に削除する。
    *   描画処理自体は「カメラ（i）が捉えたデータを無条件で描画する」という1つの共通フローに統合する。
*   **要件2: 視界データの抽出 (データ準備と描画の分離)**
    *   描画の直前（ループの先頭）で、カメラ位置 `i` が捉えるべき「視界データ（テキストと画像）」を抽出するロジックを設ける。
    *   **テキスト**：中間スロットも含め、常に `ctx.shuffleMetadataList` とカメラの相対位置（`ctx.currentTrackIndex + i` を全曲数で丸めたもの）から取得する。
    *   **画像**：`i == 0` の場合は `ctx.drumSlots[ctx.currentDrumSlotIndex].artBitmap`、`i == ctx.animatingOldIndexOffset` の場合は `ctx.drumSlots[1 - ctx.currentDrumSlotIndex].artBitmap` を抽出し、それ以外は `nullptr` とする。
*   **要件3: 単一フローによる描画の実行**
    *   抽出した画像データが有効であればそれを描画し、`nullptr` であればフォールバック（ガラス板等）を描画する。
    *   続いて、抽出したテキストデータを描画する。
    *   これにより、OLDのハードコードも、中間スロットのテキスト消失もすべて単一の美しいロジックに吸収されて解決する。

#### 【絶対遵守ルール (Constraints)】
*   **描画の単一化**: 描画処理のブロック自体を `if (i == 0)` のように分けてはならない。違いは「抽出されたデータの中身」だけであること。
