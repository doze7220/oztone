### 作業指示書 REQ: Hotfix Task 2 : ConfigManagerへの設定追加
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはタスク2・ConfigManagerへの設定追加である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク2」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク2のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク2」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。
5. チャットにて「ConfigManagerへの設定追加(Hotfix Task 2)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、仮想スクロール時のツールチップ表示位置を制御するため、ConfigManagerに新しいパラメータを追加する。

*   **要件1: [Layout_NowPlaying] セクションの拡張**
    *   `src/ConfigManager.h` に `TooltipOffsetX` および `TooltipOffsetY` のメンバ変数とゲッターを追加する。
    *   設定のパース処理を行っている適切なファイル（`src/ConfigManager_Window.cpp` など `[Layout_NowPlaying]` のロードを担当しているファイル）にて、INIファイルからの読み書き処理を実装する。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` にある `[Layout_NowPlaying]` セクションに、デフォルト値（例: `TooltipOffsetX=-10.0`, `TooltipOffsetY=-20.0` など、微調整可能な初期値）を追記し、真実の単一情報源 (SSOT) の原則を維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `ConfigManager` および関連する設定ファイルへの「プロパティの追加」のみを行う。`Window` 層でのホバー判定追加や `Widget` での描画処理の組み込みは後続のタスクで行うため、絶対に実装しないこと。
