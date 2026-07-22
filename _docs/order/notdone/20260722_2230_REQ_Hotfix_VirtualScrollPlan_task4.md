### 作業指示書 REQ: Hotfix Task 4 : Window層 - アキュムレータの実装
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはTask 4・アキュムレータの実装（仮想スクロールの初動重み付け対応）である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク4」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク4のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク5以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク4」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。（※既存のファイル内容を破壊せず、タスク4の完了報告を正しく追記すること）
5. チャットにて「アキュムレータの実装(Hotfix Task 4)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、TrackInfo領域でのマウスホイール操作を捕捉し、「初動は重く、一度回り始めたら軽く回せる」アキュムレータをWindow層に実装する。

*   **要件1: Window層へのスクロール状態追加**
    *   `src/Window.h` に仮想スクロール用の状態とコールバックを追加する。
        *   `int m_virtualScrollAccumulator = 0;` (ホイール入力の蓄積値)
        *   `bool m_isVirtualScrolling = false;` (現在仮想スクロール状態にあるか)
        *   `std::function<void(int)> m_onVirtualScrollCallback;` (スクロール確定時に +1 または -1 を通知するコールバック)
    *   コールバック登録用の `void SetVirtualScrollCallback(std::function<void(int)> cb);` を追加する。
    *   外部（Application層）からスクロール完了時に状態をリセットするためのメソッド `void ResetVirtualScrollState();` を宣言・実装し、中で `m_virtualScrollAccumulator = 0;` と `m_isVirtualScrolling = false;` を行う。
*   **要件2: マウスホイール入力の捕捉とアキュムレータ実装**
    *   `src/Window_Mouse.cpp` の `HandleMouseWheel` メソッド内にて、`m_isTrackInfoHovered` が true の場合の処理を追加する（プレイリスト領域等がホバーされている場合はそちらの処理を優先し、既存の排他制御を壊さないこと）。
    *   受け取った `zDelta` を `m_virtualScrollAccumulator` に加算する。
    *   「初動の閾値」と「スクロール中の閾値」を切り替えるロジックを実装する。
        *   `m_isVirtualScrolling` が **false** の場合（初動）は、重めの閾値（例: `WHEEL_DELTA * 2` 等、適度な重み）を要求する。
        *   `m_isVirtualScrolling` が **true** の場合（回転中）は、軽めの閾値（例: `WHEEL_DELTA`）で反応するようにする。
    *   蓄積値の絶対値が現在の閾値を超えた場合、以下の処理を行う。
        *   `m_isVirtualScrolling = true;` に状態を変更する。
        *   ホイールの回転方向（`zDelta` が正なら前へ＝1、負なら次へ＝-1 等、既存の曲送り仕様に合わせた方向）を判定し、`m_onVirtualScrollCallback` を発火させる。
        *   コールバック発火後、`m_virtualScrollAccumulator` を超過分含めて適切にリセットまたは減算し、連続入力に備える。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは「Window層でのホイール入力捕捉」と「コールバックの呼び出し」のみを責務とする。Application層での `m_onVirtualScrollCallback` の登録やタイマー管理、ドラムアニメーションの発注などは、後続のタスクで行うため絶対に実装しないこと。
