【目的】
Phase 8-5 Hotfix: ウィンドウリサイズ後に、右下のリサイズグリップ（WM_NCHITTEST）および画面下部の各種UIホバー領域（再生コントロール・音量コントロール）が反応しなくなるバグを修正します。

【原因】
Window クラス内のヒットテスト（座標判定）において、ConfigManager の初期ウィンドウサイズ（GetWindowWidth/Height）を参照しているため、リサイズ後の動的なウィンドウサイズ（クライアント領域）と判定領域にズレが生じています。

【実装要件】
対象: src/Window.cpp のマウス座標判定処理群
以下のすべての座標判定処理において、`ConfigManager` のウィンドウサイズを使用するのではなく、`GetClientRect(hwnd, &rect)` を用いて「現在の動的なクライアント領域の物理サイズ」を取得し、それをDPIスケールで論理サイズに変換してから判定を行うように修正してください。

1. `WM_NCHITTEST` 内の右下リサイズアンカー（HTBOTTOMRIGHT）の判定処理
   - `GetClientRect` で取得した `rect.right` と `rect.bottom` を基準にして、右下15x15ピクセル（DPIスケーリング適用後）の領域を計算してください。
2. `IsInPlaybackControlRegion` および `IsInVolumeControlRegion` メソッド
   - メソッド内で（または引数として）現在のクライアント領域の高さを取得し、そこから `ControlHoverHeight` などのオフセットを引いて下端領域を正しく算出するように修正してください。
3. `GetPlaybackButtonAt` メソッド
   - こちらも Y座標のベース（BaseBottomOffsetなど）を計算する際、動的なウィンドウ高さを基準とするように修正してください。

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点と懸念点をまとめた実装レポートを出力してください。
