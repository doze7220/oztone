##### 作業指示書 REQ: Hotfix / 仮想スクロールバグ修正 最終形態 (PlaylistManager疑似選択への純化) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは仮想スクロールのメタデータズレバグを根本解決し、アーキテクチャを純化する最終Hotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VirtualScrollFinal.md）として新規作成すること。原因と対応内容を追記すること。
4. チャットにて「仮想スクロールの純化（疑似選択化）が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
仮想スクロール用のターゲット変数（`m_virtualScrollTargetIndex`）を用いた泥臭い状態管理を完全にパージし、ホイール操作時に直接 `PlaylistManager` のインデックスを移動（疑似選択）させるアーキテクチャへ純化する。

*   **要件1: 仮想ターゲット変数の完全パージとUI偽装の解除**
    *   `src/Application.h` から `m_virtualScrollTargetIndex` の宣言を削除する。
    *   `src/Application_Render.cpp` にて、プレイリストUI（`DrawPlaylist` 等への引数渡し）に対して「仮想スクロール中はインデックスをすり替える」という偽装処理（前回のHotfix内容）を完全に削除し、常に純粋な `m_playlistManager.GetCurrentIndex()` を渡す形へと戻す。
*   **要件2: 仮想スクロールコールバックの疑似選択化（カレント移動）**
    *   `src/Application_Initialize.cpp` （またはコールバック登録箇所）の仮想スクロール入力処理にて、ホイール方向に応じて **直接 `m_playlistManager.Previous()` または `m_playlistManager.Advance()` を呼び出し**、カレントインデックスを本当に移動させる。
    *   その後 `m_trackDrum.StartDrumAnimation` を呼び出す。渡す距離は（UP=-1, DOWN=+1 等、標準化済みの方向で）+1 または -1 とする。
    *   `dataProvider` ラムダ式内でのメタデータ取得基準（ベースインデックス）は、移動済みの `m_playlistManager.GetCurrentIndex()` を直接参照するように純化する。
*   **要件3: 確定タイマー処理の純化**
    *   `src/Application_Render.cpp` にある、仮想スクロールタイマーが 0 になった瞬間の「再生確定処理」において、もし `m_playlistManager.JumpToIndex(...)` などのターゲット移動処理が残っていれば削除する（すでにカレントインデックスが目的の曲に合っているため）。
    *   純粋に `PlayCurrentTrack(true)` のみを呼び出し、現在の曲の再生開始（と正規画像の即時適用）のみを行わせる。

#### 【絶対遵守ルール (Constraints)】
*   **状態の純化** : 仮想スクロール中であっても、システムの真実の単一情報源（SSOT）は常に `PlaylistManager` のカレントインデックス一つのみとすること。一時的な仮想変数で状態を二重管理しないこと。
