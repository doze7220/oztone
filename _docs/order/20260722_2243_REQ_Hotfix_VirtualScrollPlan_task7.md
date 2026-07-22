### 作業指示書 REQ: Hotfix Task 7 : Application層 - ドラム連動と再生確定ロジックの実装
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはTask 7・ドラム連動と再生確定ロジックの実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク7」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク7のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク8以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク7」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。（※既存のファイル内容を破壊せず、タスク7の完了報告を正しく追記すること）
5. チャットにて「ドラム連動と再生確定ロジックの実装(Hotfix Task 7)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、Windowからの仮想スクロールコールバックを受け取り、ドラムアニメーションを起動し、タイマー終了時に再生を確定させる一連の心臓部を実装する。

*   **要件1: スクロールコールバックのバインドとドラム回転発注**
    *   `src/Application_Initialize.cpp` の `SetupCallbacks` 等にて、`m_window.SetVirtualScrollCallback` を用いてコールバックを登録する。
    *   コールバック内では以下の処理を行う。
        1. 初回入力時（`m_virtualScrollTimer <= 0.0f`）は、`m_virtualScrollTargetIndex` を `m_playlistManager.GetCurrentIndex()` で初期化する。
        2. 引数で渡された方向（1 または -1）に従い `m_virtualScrollTargetIndex` を増減させ、プレイリストの曲数でループ（またはクランプ）させる。
        3. `m_virtualScrollTimer` を再設定する（例: `0.5f`）。
        4. `m_trackDrum.StartDrumAnimation` を呼び出してドラムを回転させる。
*   **要件2: ファイルロックを完全に回避する DataProvider の供給**
    *   上記の `StartDrumAnimation` に渡す `dataProvider` ラムダ式は、**絶対に画像ファイル（サムネイルやフル解像度アート）の非同期/同期ロード（`GetOrGenerateThumbId` や `RequestThumbnailLoad` 等）を発注・実行しない**ものとする。
    *   `m_playlistManager` のメタデータからタイトル・アーティスト名のみを純粋に取得し、`artBitmap` は必ず `nullptr` を設定して `DrumSlot` に詰め込むこと（これにより、スクロール中はファイルロックが一切発生せず、高速なパラパラめくりが可能となる）。
*   **要件3: 再生確定ロジックの実装**
    *   `src/Application_Render.cpp` の `Run` ループ内にある `m_virtualScrollTimer` の減算処理を拡張する。
    *   「前フレームまでタイマーが 0 より大きく、今回の減算で 0 以下になった瞬間（指を止めて一拍置いた瞬間）」を検知する。
    *   検知した瞬間、`m_playlistManager.JumpToIndex(m_virtualScrollTargetIndex)` を行い、`PlayCurrentTrack()` を呼び出して再生を正式に確定させる。
    *   さらに `m_window.ResetVirtualScrollState()` を呼び出してWindow層のスクロール状態（アキュムレータ等）をリセットする。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクはドラム連動とタイマー制御による再生確定のみを責務とする。Widget層でのツールチップ（アフォーダンス）の描画処理は、後続のタスクで行うため絶対に実装しないこと。
*   **ファイルロックの完全回避** : スクロール中の `dataProvider` において、ディスクアクセスを伴うタグ・画像ロード処理を絶対に書いてはならない。