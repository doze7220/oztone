【目的】
Phase 8-5: システムトレイ連動型の「リサイズモード」の実装と、リアルタイムなサイズ変更（スワップチェインのリサイズ）を実装します。
※文字溢れ（トリミング）対応は別フェーズで行うため、本フェーズのスコープには含めません。

【実装要件】
1. リサイズモードのトグルと状態管理 (ConfigManager / Window)
対象: src/ConfigManager.h, src/ConfigManager.cpp, src/Window.h, src/Window.cpp
- ConfigManager の `[Window]` セクションに `EnableResize`（デフォルト false）を追加し、読み書きとゲッター/セッターを実装してください。
- Window.h/cpp にトレイメニューアイテム `ID_TRAY_ENABLE_RESIZE` を追加してください。
- メニュー構築時に `EnableResize` の状態に応じてチェックマークを付け、選択時 (WM_COMMAND) にフラグをトグル保存するように実装してください。

2. リサイズ判定とリサイズグリップ描画 (Window / Renderer)
対象: src/Window.cpp, src/Renderer.cpp
- WindowProc の `WM_NCHITTEST` にて、`ConfigManager` の `EnableResize` が true であり、かつマウス座標が「ウィンドウ右下の 15x15 (論理ピクセルをDPIスケーリングした値) の領域」にある場合のみ `HTBOTTOMRIGHT` を返すようにしてください。
- Renderer::Render にて、`EnableResize` が true の場合のみ、画面右下隅に半透明の白色で「リサイズグリップ（◢ または数本の斜め線など）」を描画してください（視覚的な手がかりの表示）。

3. リアルタイムリサイズ対応 (Window / Application / Renderer)
対象: src/Window.cpp, src/Application.h, src/Application.cpp, src/Renderer.h, src/Renderer.cpp
- WindowProc の `WM_SIZE` メッセージを捕捉し、Application 経由で Renderer::Resize(width, height) を呼び出してください。
- Renderer::Resize 内で、以下の手順でスワップチェインを安全にリサイズしてください。
  1. D2Dデバイスコンテキストのターゲット指定を解除 (`m_d2dContext->SetTarget(nullptr)`)
  2. 現在のバックバッファ (`m_d2dTargetBitmap`) を解放
  3. `m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)` を実行
  4. 再び DXGI サーフェスを取得し、`CreateBitmapFromDxgiSurface` で `m_d2dTargetBitmap` を再作成して `SetTarget` にセットする。

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点と懸念点をまとめた実装レポートを出力してください。

**【追加指示】**
【目的】
Phase 8-5 Hotfix: Win32のモーダルメッセージループによる描画停止を回避し、ウィンドウのリサイズ中（ドラッグ中）にもリアルタイムで描画を追従させるための強制再描画ロジックを追加します。

【実装要件】
1. Application クラスの拡張 (Application.h / Application.cpp)
- Application クラスに public メソッド `void ForceRender();` を追加してください。
- ForceRender() の内部には、現在 Run() メソッド内で行っている以下の描画直前準備と描画呼び出しのロジックを移植（またはコピー）してください。
  1. AudioPlayer からの現在時間、長さ、進行度 (progress) の計算
  2. 時間文字列 (timeString) の生成
  3. GetSpectrumData によるスペクトルデータの取得
  4. m_window からのホバー状態等の取得
  5. m_renderer.Render(...) の呼び出し

2. WindowProc からの強制描画フック (Window.cpp)
- WindowProc 内の `WM_SIZE` メッセージハンドラにて、スワップチェインのリサイズ（Application 経由での Renderer::Resize 呼び出し）を行った **直後** に、新設した `m_app->ForceRender();` を呼び出してください。
- これにより、OSのリサイズモーダルループ中であっても毎フレーム描画が更新されるようになります。

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点と懸念点をまとめた実装レポートを出力してください。
