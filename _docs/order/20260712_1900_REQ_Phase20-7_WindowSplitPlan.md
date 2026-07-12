### 作業指示書 REQ: Phase 20-7: Window.cpp 分割に向けた事前調査と整理・解体計画
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. まず `src/Window.cpp` 全体をスキャンし、物理分割（別ファイル化）を行う前に「関数の抽出」や「重複の排除」といった事前の整理（リファクタリング）が必要かどうかを調査すること。
3. 調査結果を踏まえ、以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-7_WindowSplitPlan.md）として新規作成すること。
4. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
現在1200行を超える `Window.cpp` を、AI-IDEのコンテキスト節約のため、実装のみを責務別の .cpp ファイルへ物理分割する。
分割後は **「ある機能を修正する際に、対象となる.cppだけを読めば実装を理解できる」** ことを目標とする。

*   **要件1: WindowProcの薄いディスパッチャ化と抽出**
    *   現在の `WindowProc` はほぼ全ての機能に触れており、そのまま分割すると `Window_Proc.cpp` に巨大なロジックが残ってしまう可能性が高い。
    *   これを防ぐため、`WindowProc` 自体は薄いディスパッチャ（司令塔）として維持し、各メッセージ処理は個別のメンバ関数（`HandleXXXX()`）へ抽出する整理計画を立てること。
    *   抽出候補を可能な限り洗い出し、責務ごとに整理すること（例: `HandleMouseMove()`, `HandleMouseLeave()`, `HandleMouseWheel()`, `HandleTrayIcon()`, `HandleCommand()`, `HandleDestroy()`, `HandleLogoMenuClick()`, `HandlePlaybackClick()`, `HandlePlaylistClick()` など）。
*   **要件2: コンテキスト単位での解体計画**
    *   抽出されたメンバ関数群を、以下の責務別ファイル（例）へ物理分割するロードマップを策定する。
        *   `Window_Proc.cpp` (薄いメッセージディスパッチの基盤)
        *   `Window_Mouse.cpp` (各UIの座標判定とマウスイベント処理)
        *   `Window_TrayMenu.cpp` (トレイメニュー生成とコマンド処理)
        *   `Window_DropTarget.cpp` (ファイルドロップ処理)
        *   `Window_Initialize.cpp` (ウィンドウ生成・初期化)
        *   `Window_System.cpp` (フック・グローバルホットキー等のシステム処理)
*   **要件3: ユーティリティファイルの作成禁止**
    *   分割単位は必ず「責務単位」とし、`WindowUtility.cpp` や `WindowCommon.cpp` のような機能横断的なファイルは **絶対に新設しない** こと。

#### 【絶対遵守ルール (Constraints)】
*   **Window.h の変更制約**: `Window.h` への「新しい `private` メンバ関数の宣言追加」は許可する。ただし、データメンバ（変数）の追加や、公開インターフェース（`public`）の変更は絶対に禁止する。
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なリファクタリング（整理と分割）の計画のみを立案すること。
