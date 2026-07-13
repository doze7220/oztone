# RES:HOTFIX作業レポート: 背景クリックによるUIフェードアウト機能の復元

## 1. 実装目的
Phase 20のファイル分割の過程で欠落した、「純粋な背景領域をクリックした際にUIのホバー維持ディレイを即座にリセットし、能動的フェードアウトを開始する」機能を復元する。

## 2. 調査内容
- 各層（Window層、Application層、Renderer層、Widget層）における該当機能の実装状態を調査した。
- `Renderer`、`Widget_Playlist`、`Widget_LogoMenu`、`Application_Initialize`などの層には、ディレイのリセット機能（`ForceClearHoverDelay`やコールバック登録）が既に正しく維持・実装されていることを確認した。
- 原因: `Window_Mouse.cpp`へのファイル分割時に、`WM_LBUTTONDOWN` (`HandleLButtonDown`) 内における「純粋な背景クリックの判定」と「コールバック発火」のロジックが欠落し、さらに下部の「ウィンドウのドラッグ判定」の箇所に誤ってコールバックの発火が残留・結合してしまっていた。

## 3. 詳細作業内容
- `src/Window_Mouse.cpp` の `HandleLButtonDown` を修正した。
- ウィンドウのドラッグ判定部（`GetLockWindowPosition()` を評価する箇所）から、誤って残留していた `m_onBackgroundClickCallback()` の発火処理を削除した。
- ドラッグ処理とは完全に分離し、他のUI領域（プレイリスト、ロゴメニュー、再生コントロール、音量コントロール、アプリアイコン）のいずれにも該当しない「純粋な背景クリック」の条件分岐内（上部）で、正確に `m_onBackgroundClickCallback()` を発火させるようロジックを復元した。
- 右クリック（`HandleRButtonDown`）側の判定・発火ロジックは既に正しく維持されていることを確認した。
