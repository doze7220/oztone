##### 作業指示書 REQ: Hotfix / 仮想スクロールバグ修正1-B (TrackInfoホバー領域の厳格化) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは仮想スクロール実装に伴うホバー領域異常バグの追加Hotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VirtualScrollBug1_B.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. チャットにて「TrackInfoのホバー領域厳格化が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
現在、`src/Window_Mouse.cpp` の `IsInTrackInfoRegion` におけるY座標の判定領域が曖昧であり、画面の半分近くをホバー領域として誤認識してしまうバグが発生している。これを解消するため、ConfigManagerの設定値を基に、各要素の正確な配置領域（バウンディングボックス）を算出して厳密なヒットテストを行うように改修する。

*   **要件1: Y座標の正確な領域計算の実装**
    *   `IsInTrackInfoRegion` 内のY座標判定を、以下のロジックに従って完全に書き直すこと。
    *   基準となるY座標を算出する: `float baseY = logicalHeight - m_config->GetBaseBottomOffset();`
    *   アルバムアート、タイトル、アーティスト名それぞれの「上端」のY座標を算出する（`baseY` + 各 `OffsetY`）。
    *   アルバムアート、タイトル、アーティスト名それぞれの「下端」のY座標を算出する（上端 + `ArtSize` または各 `FontSize`）。
    *   3つの上端のうち最も小さい値（画面上寄り）を `topLimit` とし、3つの下端のうち最も大きい値（画面下寄り）を `bottomLimit` とする（`<algorithm>` の `std::min`, `std::max` などを活用すること）。
    *   マウスポインタの `logicalY` が `topLimit` と `bottomLimit` の間に収まっている場合のみ、Y座標の条件を満たすとする。
*   **要件2: X座標の制限（維持）**
    *   X座標については、「画面左端(0)から、プレイリストの展開判定領域の手前まで」という既存のロジック（右端UIとの干渉回避）を維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `Window_Mouse.cpp` における `IsInTrackInfoRegion` の座標計算ロジックの修正のみを行う。スクロール方向の修正や、他のUIのヒットテストには一切触れないこと。