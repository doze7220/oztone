##### 作業指示書 REQ: Hotfix / 仮想スクロールバグ修正3 (メタタグ抽出の二重加算解消) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは仮想スクロール中のメタ情報ズレバグのHotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VirtualScrollBug3.md）として新規作成すること。原因と対応内容を追記すること。
4. チャットにて「メタタグのズレバグ修正が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
仮想スクロール（マウスホイール操作）時のドラム回転において、テキストメタデータ（曲名、アーティスト名等）が実際の曲とズレてしまうバグを修正する。原因は、ドラムへメタデータを供給する `dataProvider` ラムダ式内で、相対距離（`relDist`）を加算する基準点として誤った変数（ターゲットインデックス等）が使われていることにある。

*   **要件1: dataProvider の基準インデックスの適正化**
    *   `src/Application_Initialize.cpp` (または仮想スクロールのコールバック処理が実装されている箇所) の `WM_MOUSEWHEEL` 入力等に関連するコールバック内で、`m_trackDrum.StartDrumAnimation(...)` に渡している `dataProvider` ラムダ式を特定する。
    *   現在、データ取得対象のインデックスを計算する基準（`baseIndex`）として `m_virtualScrollTargetIndex` などが使われ、二重加算のバグを引き起こしている箇所を、**必ず `m_playlistManager.GetCurrentIndex()` を基準（ベース）とするように修正する。**
    *   取得先インデックスの計算式は、純粋に `m_playlistManager.GetCurrentIndex() + relDist` （プレイリストの総曲数を用いた適切なループ・クランプ処理を含む）の形へ純化すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `StartDrumAnimation` に渡す `dataProvider` ラムダ式内部のインデックス計算式の修正のみを責務とする。他のファイルや再生確定ロジックには一切触れないこと。