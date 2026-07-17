### 作業指示書 REQ: Hotfix ピン留め時の背景クリック判定抜け修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_BackgroundClick_Pinned.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「ピン留め時の背景クリック判定抜けの修正が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
プレイリストのピン留め時（排他制御の緩和時）に、純粋な背景クリックによる能動的フェードアウト（ディレイキャンセル）が発動しなくなっている不具合を修正する。

*   **要件1: HandleLButtonDown へのフォールバック発火追加**
    *   `src/Window_Mouse.cpp` の `HandleLButtonDown` メソッドにおいて、上部の排他制御ブロックをスルーした後に実行される「通常のヒットテスト（`IsInPlaylistRegion`, `IsInLogoMenuRegion`, `IsInPlaybackControlRegion` などの `if - else if` チェーン）」を特定する。
    *   そのヒットテストチェーンの最後尾に `else` ブロックを追加し、いずれのUI領域にも該当しなかった場合に `if (m_onBackgroundClickCallback) m_onBackgroundClickCallback();` を呼び出す処理を復元する。（下部にあるドラッグ移動である `HTCAPTION` の送信処理より確実に前で行うこと）

*   **要件2: HandleRButtonDown へのフォールバック発火追加**
    *   同様に `src/Window_Mouse.cpp` の `HandleRButtonDown` メソッドにおいても、通常のヒットテスト（UIの右クリック判定など）を抜けて背景アートフレーミング操作（`m_onArtFramingScroll` 等の準備）へ移行する直前、またはいずれのUIにも該当しなかった場合のフォールバックとして `if (m_onBackgroundClickCallback) m_onBackgroundClickCallback();` を呼び出す処理が欠落していれば追加・復元する。
