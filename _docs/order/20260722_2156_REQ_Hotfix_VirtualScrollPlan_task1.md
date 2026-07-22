### 作業指示書 REQ: Hotfix Task 1 : ツールチップ描画ロジックの共通化
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはタスク1・ツールチップ描画ロジックの共通化である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク1」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク1」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。
5. チャットにて「ツールチップ描画ロジックの共通化(Hotfix Task 1)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、仮想スクロール機能のアフォーダンスとして流用するために、現在 `Widget_VolumeControl.cpp` 内に存在するツールチップ描画ロジックを共通化する。

*   **要件1: WidgetCommon へのロジック抽出**
    *   `src/WidgetCommon.h` および `src/WidgetCommon.cpp` に、ツールチップを描画するための汎用関数（例: `DrawMouseScrollTooltip`）を追加する。
    *   引数には、描画コンテキスト (`ID2D1DeviceContext`)、ベース座標 (`X`, `Y`)、テキストレイアウト (`IDWriteTextLayout`)、吹き出しの背景ジオメトリ (`ID2D1PathGeometry`)、描画ブラシなどを適切に受け取れるように設計すること。
*   **要件2: VolumeControlWidget のリファクタリング**
    *   `src/Widget_VolumeControl.cpp` の `Draw` メソッド内にある、ツールチップ（背景ジオメトリやテキスト）を描画しているコードを、要件1で作成した `WidgetCommon::DrawMouseScrollTooltip` の呼び出しに置き換える。
    *   この変更により、既存の音量コントロールのツールチップ描画結果（見た目）が一切変わらないことを保証すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは描画ロジックの「抽出」と「置換」のみを行う。`ConfigManager` の拡張や、他のウィジェット（TrackInfo等）へのツールチップ組み込みなどは、後続のタスクで行うため絶対に実装しないこと。
