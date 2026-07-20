##### 作業指示書 REQ: Hotfix: トラックドラムの完全初期化とmaxId復元漏れの修正 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトはトラックドラムの初期化不足およびサムネイルID復元漏れに関する単独のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DrumInit_And_MaxIdFix.md）として新規作成すること。作業レポートに原因と対応内容を記載すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/Renderer_TrackDrum.cpp`, `src/ThumbnailDatabase.cpp`
*   **要件1: ドラムの完全初期化 (`src/Renderer_TrackDrum.cpp`)**
    *   `StartDrumAnimation` 内の初回起動時処理 (`if (!m_isDrumInitialized)`) において、カレントスロットのみならず、3つのスロットすべてに対して `dataProvider` を呼び出してデータを事前注入（Pre-fill）するように修正すること。
    *   例: 
        `dataProvider(0, &m_drumSlots);`
        `dataProvider(1, &m_drumSlots[1]);`
        `dataProvider(-1, &m_drumSlots[2]);`
    *   これにより「次・前のサムネが消える」問題を解決する。
*   **要件2: maxId更新漏れの修正 (`src/ThumbnailDatabase.cpp`)**
    *   `Initialize` メソッド内において、`.idx` ファイルのパースループ内で `maxId` を確実に更新する処理（例: `if (thumbId > maxId) maxId = thumbId;`）が記述されているか確認し、欠落していれば追記すること。
    *   これにより、再起動時に `m_nextId` が 1 にリセットされ、新曲に `id 1` が強制発番されてしまうバグを完全に解決する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本タスクでは初期化時のスロット全注入と `maxId` の更新処理の補完のみを行うこと。他のロジックには一切触れないこと。
