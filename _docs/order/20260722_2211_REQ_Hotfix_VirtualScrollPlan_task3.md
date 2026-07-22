### 作業指示書 REQ: Hotfix Task 3 : Window層 - TrackInfo領域のホバー判定追加
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはTask 3・TrackInfo領域のホバー判定追加である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク3」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク3のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク4以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク3」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。（※既存のファイル内容を破壊せず、タスク3の完了報告を正しく追記すること）
5. チャットにて「TrackInfo領域のホバー判定追加(Hotfix Task 3)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、左下のアルバムアート・曲名・アーティスト名（TrackInfo）領域へのマウスホバーを検知する基盤をWindow層に追加する。

*   **要件1: Window層への状態追加**
    *   `src/Window.h` にホバー状態を保持する `bool m_isTrackInfoHovered = false;` およびゲッター `bool IsTrackInfoHovered() const { return m_isTrackInfoHovered; }` を追加する。
    *   ホバー領域を判定するメソッド `bool IsInTrackInfoRegion(int x, int y) const;` を宣言し、`src/Window_Mouse.cpp` に実装する。
*   **要件2: ホバー判定ロジックの実装**
    *   `IsInTrackInfoRegion` の内部では、マウスの物理座標(x, y)を論理座標(logicalX, logicalY)に変換して判定を行う。
    *   判定領域は、Y座標が「TrackInfoが描画される高さの範囲（概ね画面下部だが、再生コントロール領域よりは上、または重複を避ける範囲）」であり、X座標は「画面左端(0)から、プレイリストのホバー判定領域（右端）の手前まで」とする。
*   **要件3: マウスイベントでの状態更新と排他制御**
    *   `src/Window_Mouse.cpp` の `HandleMouseMove` メソッド内にて、`IsInTrackInfoRegion` を呼び出し、`m_isTrackInfoHovered` を更新する。
    *   この時、プレイリスト領域 (`m_isPlaylistHovered`) や再生コントロール領域 (`m_isControlHovered`) 等、既存のUI判定よりも **優先順位を低く** 設定すること。プレイリスト等がホバー・展開されている場合は `m_isTrackInfoHovered = false` となるように排他制御を適用する。
    *   `HandleMouseLeave` 発生時には `m_isTrackInfoHovered = false;` にリセットする。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `WM_MOUSEMOVE` に関連するホバー判定の追加のみを行う。`WM_MOUSEWHEEL` の入力捕捉（アキュムレータ）や、Widget層でのアフォーダンス（ツールチップ・ハイライト）の描画処理は、後続のタスクで行うため絶対に実装しないこと。

