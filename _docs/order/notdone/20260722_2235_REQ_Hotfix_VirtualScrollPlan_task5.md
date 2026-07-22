### 作業指示書 REQ: Hotfix Task 5 : Application層 - 仮想スクロール状態とタイマーの基礎実装
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはTask 5・仮想スクロール状態とタイマーの基礎実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク5」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク5のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク6以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク5」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。（※既存のファイル内容を破壊せず、タスク5の完了報告を正しく追記すること）
5. チャットにて「仮想スクロール状態とタイマーの基礎実装(Hotfix Task 5)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、仮想スクロールの状態を管理するための変数を Application 層に追加し、毎フレームのタイマー減算処理の基礎を構築する。

*   **要件1: Application層への状態変数の追加**
    *   `src/Application.h` に仮想スクロール用のメンバ変数を追加する。
        *   `float m_virtualScrollTimer = 0.0f;` （再生確定までの待機タイマー）
        *   `int m_virtualScrollTargetIndex = -1;` （スクロール先の目標トラックインデックス）
*   **要件2: タイマーの減算処理基盤の実装**
    *   `src/Application_Render.cpp` の `ForceRender` または `Run` 内など、毎フレーム実行される適切な箇所（deltaTime を算出している付近など）に、タイマーの減算処理を追加する。
    *   `m_virtualScrollTimer` が 0.0f より大きい場合、`deltaTime` を減算し続ける基礎的なロジックを記述する。
    *   ※この段階では、タイマーが 0 以下になった際の具体的な処理（再生の確定やインデックスのジャンプなど）は書かなくてよい（後続タスクで実装するため、空の if 構文やコメントのみでよい）。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは Application 層への「変数追加」と「タイマーの減算処理」のみを責務とする。自動曲送りのブロック（`IsAtEnd`のスキップ）や、ドラムアニメーションの発注、Windowからのコールバックのバインドなどは、後続のタスクで行うため絶対に実装しないこと。
