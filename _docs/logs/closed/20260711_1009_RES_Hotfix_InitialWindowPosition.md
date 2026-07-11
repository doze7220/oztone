# RES:実装計画・作業レポート Phase Hotfix: 初回起動時のウィンドウ中央配置

## 1. 実装目的
INIファイルが存在しない初回起動時において、ウィンドウがOS任せの配置（CW_USEDEFAULT）となり画面外や不自然な位置へ表示される問題を防ぐため。

## 5. HOTFIX1
### 原因・理由: 初回起動時のウィンドウ配置がOS任せになる問題
    - 初回起動時など、INIファイルにウィンドウ座標が記録されておらず `CW_USEDEFAULT` が設定されている場合、そのまま `CreateWindowExW` に渡されると、OSのデフォルト挙動により意図しない位置に配置されることがある。
### 対応: 画面中央への配置処理追加
    - `src/Window.cpp` の `Window::Initialize` にて、`x == CW_USEDEFAULT` または `y == CW_USEDEFAULT` の場合、`GetSystemMetrics(SM_CXSCREEN)` と `GetSystemMetrics(SM_CYSCREEN)` を使用してプライマリモニタの解像度を取得し、ウィンドウが画面中央になるよう `x` と `y` を計算して上書きする処理を追加した。
