# 実装ログ: 20260701_2108_Hotfix_WindowResize_debug.md

## 概要
ウィンドウリサイズ後に、右下のリサイズグリップ（`WM_NCHITTEST`）および画面下部の各種UIホバー領域（再生コントロール・音量コントロール）が反応しなくなるバグを修正しました。

## 調査結果
指示書で疑われていた「`ConfigManager`の初期サイズを参照している」という問題は、現在のコードには存在せず、すべての座標計算処理（`WM_NCHITTEST`, `IsInPlaybackControlRegion`, `IsInVolumeControlRegion`, `GetPlaybackButtonAt`）はすでに `GetClientRect` を用いて動的なクライアント領域を取得していることを確認しました。

バグの真の原因は、Windows OS における `WS_EX_LAYERED` と DirectComposition の仕様でした。GDI関数（`UpdateLayeredWindow`）による透明化を行わずにDCompを使用した場合、OSは入力判定領域（Hit-test region）を初期サイズのまま自動更新しなくなります。これにより、ウィンドウサイズが拡大されても枠外のマウスイベントがOSからアプリケーションに通知されないという現象が発生していました。

## 修正内容
- **対象ファイル:** `src/Window.cpp`
- **修正内容:** `Window::Initialize` メソッドにおいて、`CreateWindowExW` を呼び出してウィンドウを生成した直後に、`SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA)` を呼び出す処理を追加しました。
- **効果:** これにより、OSが入力判定領域をウィンドウ矩形に従って自動更新するようになり、リサイズ後も画面下部や右下のUIが正常に機能するようになります。描画は引き続きDirectCompositionによって行われるため、視覚的な影響はありません。

## 今後の課題・備考
- 本件は DComp を用いたレイヤードウィンドウにおけるOS固有の既知の挙動に対するワークアラウンドとなります。
