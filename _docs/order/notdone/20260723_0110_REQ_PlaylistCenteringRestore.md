##### 作業指示書 REQ: Hotfix / プレイリストの能動的センタリング機能復元 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは、失われたプレイリストの「カレント曲センタリング機能」を、ユーザーの能動的な操作時のみに限定して復活させるHotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_PlaylistCenteringRestore.md）として新規作成すること。
4. チャットにて「能動的センタリング機能の復元が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
手動スクロール量が永遠にリセットされずカレント曲が中央に戻らない状態を解決するため、Widget側にスクロールリセット機能を復活させる。ただしUXの観点から、自動再生時やクリックジャンプ時には発動させず、「ダイヤル（ホイール）操作時」および「Next / Prev 操作時」の能動的な曲変更時にのみセンタリング（リセット）を発動させる。

*   **要件1: IWidget および Widget_Playlist へのリセット機能追加**
    *   `src/Widget.h` の `IWidget` インターフェースに `virtual void ResetScroll() {}` (デフォルト空実装) を追加する。
    *   `src/Widget_Playlist.h` および `src/Widget_Playlist_Update.cpp` (または該当の物理分割ファイル) にて `ResetScroll()` をオーバーライドし、内部の手動スクロール量（`m_playlistManualScrollY` 等）を `0.0f` にリセットする処理を実装する。
*   **要件2: Renderer層への伝達**
    *   `src/Renderer.h` と `src/Renderer.cpp` に `void ResetPlaylistScroll();` を追加する。
    *   実装内で `m_widgets` をイテレートし、各 widget に対して `ResetScroll()` を呼び出す。
*   **要件3: センタリング発動トリガーの追加 (Application層)**
    *   **トリガーA (ダイヤル/ホイール操作時)**: `src/Application_Initialize.cpp` (またはスクロールコールバック登録箇所) にて、ホイール回転に伴い `m_playlistManager.Previous()` または `Advance()` を呼んで疑似選択を行った直後に、`m_renderer.ResetPlaylistScroll();` を呼び出す。
    *   **トリガーB (Next / Prev 操作時)**: `src/Application_Playback.cpp` の `HandleMediaCommand` メソッド内にて、`APPCOMMAND_MEDIA_NEXTTRACK` および `APPCOMMAND_MEDIA_PREVIOUSTRACK` に該当する処理（曲が切り替わった直後）において、`m_renderer.ResetPlaylistScroll();` を呼び出す。これによりホットキーやメディアキー操作時にもリストが中央へスナップする。

#### 【絶対遵守ルール (Constraints)】
*   **既存UXの保護 (除外対象)** : 以下の2つの処理においては、**絶対に `ResetPlaylistScroll()` を呼び出さないこと**。
    1. `Application_Playlist.cpp` の `OnPlaylistClick` (クリックによるジャンプ再生)。
    2. 自動での次曲移行処理（`Run` ループ内の `IsAtEnd()` 経由など）。これらはユーザーの視界を勝手に奪わないため、現在のスクロール位置を完全に維持する。