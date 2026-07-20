##### 作業指示書 REQ: Hotfix: トラックドラムの初回起動時即時ロード対応 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトはTrackDrumエンジンの初回起動時アニメーションスキップに関する単独のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_TrackDrum_InitLoad.md）として新規作成すること。作業レポートに原因と対応内容を記載すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/Renderer_TrackDrum.h`, `src/Renderer_TrackDrum.cpp`
*   `Renderer_TrackDrum.h` のメンバ変数に `bool m_isDrumInitialized = false;` を追加すること。
*   `Renderer_TrackDrum.cpp` の `StartDrumAnimation` メソッドの冒頭に、`if (!m_isDrumInitialized)` の分岐を追加すること。
*   初回（未初期化）時の処理として、以下を実装すること。
    1.  `m_drumTargetPosition` および `m_drumRelativePosition` を `0.0f` に設定する（スクロールなし）。
    2.  `dataProvider(0)` を呼び出し、現在の曲のメタデータを取得して現在のスロット（`m_drumSlots[m_currentDrumSlotIndex]`）に直接セットする（画像は一旦 `nullptr` 等で構わない）。
    3.  `m_isDrumInitialized = true;` とし、引数で渡された `onComplete()` コールバックを即座に呼び出して、そのまま `return;` で早期リターンする。
*   この修正により、アプリ起動時（初回呼び出し時）はアニメーションをスキップし、即座に1曲目の情報がカレントスロットに定義される状態を保証すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本タスクでは `TrackDrum` の初回起動時フラグの追加と即時リターン処理のみを行うこと。他のアニメーション計算式等には一切触れないこと。
