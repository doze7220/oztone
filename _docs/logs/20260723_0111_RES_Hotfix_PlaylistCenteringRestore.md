# RES:HOTFIX作業レポート: プレイリストの能動的センタリング機能復元

## 1. 実装目的
手動スクロール量が永遠にリセットされず、カレント曲が中央に戻らない状態を解決するため、Widget側にスクロールリセット機能を復活させる。ただしUXの観点から、自動再生時やクリックジャンプ時には発動させず、「ダイヤル（ホイール）操作時」および「Next / Prev 操作時」の能動的な曲変更時にのみセンタリング（リセット）を発動させる。

## 2. 調査内容
*   `IWidget` および `PlaylistWidget` に対するスクロール量のリセットインターフェース (`ResetScroll()`) の追加が必要。
*   `Renderer` 層から全Widgetに対してリセット命令をブロードキャストするメソッド (`ResetPlaylistScroll()`) が必要。
*   `Application` 層における能動的曲送り（仮想スクロール操作、およびホットキー/メディアキーからのNext/Prev操作）直後にのみ上記リセット処理を呼び出す。
*   自動再生（RunループのIsAtEnd経由）やリストクリック時 (`OnPlaylistClick`) は除外する。

## 3. 対象ファイル
*   `src/Widget.h`
*   `src/Widget_Playlist.h`
*   `src/Widget_Playlist.cpp`
*   `src/Renderer.h`
*   `src/Renderer.cpp`
*   `src/Application_Initialize.cpp`
*   `src/Application_Playback.cpp`

## 4. 実装タスクリスト
[x] タスク1: IWidget および Widget_Playlist へのリセット機能追加
[x] タスク2: Renderer層への伝達メソッド追加
[x] タスク3: センタリング発動トリガーの追加 (Application層)

## 5. 詳細作業内容
* タスク1: IWidget および Widget_Playlist へのリセット機能追加
    - `src/Widget.h` の `IWidget` インターフェースに `virtual void ResetScroll() {}` を追加。
    - `src/Widget_Playlist.h` に `void ResetScroll() override;` を追加。
    - `src/Widget_Playlist.cpp` に `ResetScroll()` を実装し、`m_playlistManualScrollY = 0.0f;` となるよう修正。
* タスク2: Renderer層への伝達メソッド追加
    - `src/Renderer.h` に `void ResetPlaylistScroll();` を追加。
    - `src/Renderer.cpp` に実装を追加し、`m_widgets` をイテレートして各widgetの `ResetScroll()` を呼び出すようにした。
* タスク3: センタリング発動トリガーの追加 (Application層)
    - `src/Application_Initialize.cpp` の仮想スクロールコールバック内で、`m_playlistManager.Previous()` または `Advance()` の呼び出し直後に `m_renderer.ResetPlaylistScroll();` を追加。
    - `src/Application_Playback.cpp` の `HandleMediaCommand` 内（`APPCOMMAND_MEDIA_NEXTTRACK`, `APPCOMMAND_MEDIA_PREVIOUSTRACK` 処理ブロック）で、同様に曲変更直後に `m_renderer.ResetPlaylistScroll();` を追加。
    - （`OnPlaylistClick` や自動曲送り処理にはリセット処理を入れないよう配慮した）
