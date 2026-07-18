##### 作業指示書 REQ: プレイリスト切り替え時のOLDメタ情報入れ替わり修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2333_RES_Hotfix_TrackInfoMeta_Restore.md（該当不具合を実装した作業レポート)

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_OldDrumSlot_DataProtection.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: 視界データ抽出ロジックの堅牢化 (DrumSlotデータの絶対保護)**
    *   `src/Widget_TrackInfo.cpp` の描画ループ（`for (int i = startOffset; i <= endOffset; ++i)` 内）における視界データ（画像およびテキスト）の抽出ロジックを以下のように修正する。
    *   **カレントスロット (`i == 0`) の場合**:
        画像だけでなく、曲名(`title`)、アーティスト名(`artist`)、トラック番号(`trackNo`)のテキスト情報すべてを、`ctx.drumSlots[ctx.currentDrumSlotIndex]` から直接抽出する。
    *   **OLDスロット (`i == ctx.animatingOldIndexOffset`) の場合**:
        画像だけでなく、テキスト情報すべてを、過去の器である `ctx.drumSlots[1 - ctx.currentDrumSlotIndex]` から直接抽出する。
    *   **中間スロット (それ以外) の場合**:
        画像は `nullptr` とし、テキスト情報のみ `ctx.shuffleMetadataList` と相対位置インデックスを用いて動的に計算して抽出する。
*   **要件2: 描画の単一フロー維持**
    *   抽出された変数の値（画像やテキスト）を用いて描画を実行する後半のフローは、従来通り `i` による条件分岐を行わない単一フローを維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **OLDの保護**: OLDスロットの情報を `shuffleMetadataList` から再計算してはならない。プレイリストが切り替わっている可能性があるため、必ず保存された `DrumSlot` の内容を信用すること。
