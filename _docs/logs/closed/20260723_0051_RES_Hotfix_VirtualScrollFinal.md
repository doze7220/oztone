# RES:HOTFIX作業レポート: 仮想スクロールバグ修正 最終形態 (PlaylistManager疑似選択への純化)

## 1. 実装目的
仮想スクロール用のターゲット変数（`m_virtualScrollTargetIndex`）を用いた泥臭い状態管理を完全にパージし、ホイール操作時に直接 `PlaylistManager` のインデックスを移動（疑似選択）させるアーキテクチャへ純化することで、メタデータズレバグを根本解決する。

## 2. 調査内容
仮想ターゲット変数による状態の二重管理が原因で、プレイリストUIやドラムアニメーションなどの各システム間でのインデックス同期にズレが生じていた。
一時的な仮想変数を削除し、システムの真実の単一情報源（SSOT）を常に `PlaylistManager` のカレントインデックス一つのみとすることで、複雑な偽装処理や同期処理を不要にし、より堅牢なアーキテクチャとする。

## 3. 対象ファイル
* `src/Application.h`
* `src/Application_Initialize.cpp`
* `src/Application_Render.cpp`

## 4. 実装タスクリスト
- [x] タスク1: 仮想ターゲット変数の完全パージとUI偽装の解除
- [x] タスク2: 仮想スクロールコールバックの疑似選択化（カレント移動）
- [x] タスク3: 確定タイマー処理の純化

## 5. 詳細作業内容
* タスク1: 仮想ターゲット変数の完全パージとUI偽装の解除
    - `src/Application.h` から `m_virtualScrollTargetIndex` の宣言を削除した。
* タスク2: 仮想スクロールコールバックの疑似選択化（カレント移動）
    - `src/Application_Initialize.cpp` にて、ホイール方向に応じて直接 `m_playlistManager.Previous()` または `Advance()` を呼び出し、カレントインデックスを実際に移動させるように修正した。
    - その後 `m_trackDrum.StartDrumAnimation` に適切な距離（+1 または -1）を渡してアニメーションを開始するように純化した。
    - 仮想ターゲット変数の計算式や偽装渡しなど、不要な処理を削除した。
* タスク3: 確定タイマー処理の純化
    - `src/Application_Render.cpp` の確定タイマー処理において、すでにカレントインデックスが目的の曲に合っているため、`m_playlistManager.JumpToIndex(...)` を削除し、純粋に `PlayCurrentTrack(-1, true)` を呼び出すだけに純化した。
