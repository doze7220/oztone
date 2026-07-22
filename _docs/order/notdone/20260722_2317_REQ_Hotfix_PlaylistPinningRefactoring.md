##### 作業指示書 REQ: Hotfix / 仮想スクロールバグ修正1 (ホバー領域・ピン留め判定の適正化) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは仮想スクロール実装に伴うホバー領域バグのHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VirtualScrollBug1.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「バグ1とバグ4の修正が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: TrackInfoホバー領域の適正化 (バグ1対応)**
    *   `src/Window_Mouse.cpp` にある `IsInTrackInfoRegion` メソッドのY座標の判定条件を修正する。
    *   現状、Y座標の範囲が広すぎるため、アルバムアートとテキストが描画される実際の高さ（例えば `logicalHeight - m_config->GetBaseBottomOffset() - artSize` 付近から下端まで、等）に厳格に制限し、画面中央付近までホバーが反応してしまうバグを修正すること。
*   **要件2: プレイリストピン留め時の領域専有の解消 (バグ4対応)**
    *   `src/Window_Mouse.cpp` の `IsInPlaylistRegion` メソッドを修正する。
    *   現在、「プレイリスト展開中（`m_isPlaylistHovered == true`）はホバー判定領域を拡張する」というロジックが入っているが、この拡張がプレイリストのピン留め（固定）時にも発動してしまい、他のUIへのホバーを阻害している。
    *   領域を拡張する条件式に「ピン留めされていないこと（`!m_config->GetIsPlaylistPinned()`）」を追加し、ピン留め中は過剰な領域拡張を行わず、本来のプレイリストの描画幅でのみホバー判定されるように修正すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `Window_Mouse.cpp` における判定領域（ヒットテスト）の修正のみを行う。ホイールスクロールの方向修正や Widget の色変更ロジックなどは絶対に含めないこと。