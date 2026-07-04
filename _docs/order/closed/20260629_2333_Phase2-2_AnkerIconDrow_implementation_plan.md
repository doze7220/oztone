# Phase 2 Step 2: Anker Icon Draw Implementation Plan

## 目的
Phase 2-2の要件に基づき、WICを用いたアプリアイコンの描画（ハイブリッド画像読み込み）、マウスホバー判定、およびレイアウトのINI駆動化を実装します。

## User Review Required
特に大きな懸念点はありませんが、以下の設計方針で進める点をご確認ください。
*   **WICの利用とCOMの初期化**: WICを使用するため、`main.cpp` で `CoInitializeEx` / `CoUninitialize` を呼び出します。また、CMakeのリンクライブラリに `windowscodecs.lib` を追加します。
*   **リソース定義**: `assets/app_logo.png` などを `RCDATA` 型としてexeに埋め込む `OZtone.rc` と `resource.h` を作成します。
*   **WindowとConfigの結合**: ホバー判定でアイコンの座標を必要とするため、`Window` クラス内に `ConfigManager` のポインタ（または参照）を保持するように修正します。

## Open Questions
*   現状の解釈で問題ないかご確認をお願いします。「問題がなければ実装を行って良い」とのことですので、承認いただけ次第、実装作業に入ります。

## Proposed Changes

### CMake & Resources

#### [NEW] resource.h
*   リソースID `IDI_APP_LOGO` および `IDI_APP_LOGO_HOVER` を定義。

#### [NEW] OZtone.rc
*   `resource.h` をインクルードし、画像パスを `RCDATA` としてリソース定義するスクリプト。

#### [MODIFY] CMakeLists.txt
*   `OZtone.rc` を `SOURCES` に追加。
*   `target_link_libraries` に `windowscodecs.lib` を追加。
*   テスト用に `add_custom_command(TARGET OZtone POST_BUILD ...)` を用いて、`assets` フォルダの画像をビルド出力ディレクトリ（exeと同階層）にコピーする設定を追加。

### ConfigManager

#### [MODIFY] src/ConfigManager.h / cpp
*   `m_logoX`, `m_logoY`, `m_logoWidth`, `m_logoHeight` のメンバ変数を追加し、ゲッターを実装。
*   `LoadSettings()` にて `[Layout_AppLogo]` セクションから各値を読み込む処理を追加。
*   `SaveDefaultSettings()` にデフォルト値（X=16, Y=16, Width=64, Height=64）を書き込む処理を追加。

### Window (Hover Detection)

#### [MODIFY] src/Window.h / cpp
*   メンバ変数 `const ConfigManager* m_config` を追加し、`Initialize` で受け取って保持。
*   ホバー状態を示す `bool m_isHovered`、マウス追跡状態の `bool m_isTrackingMouse` を追加し、ゲッター `IsHovered()` を実装。
*   `WindowProc` 内で `WM_MOUSEMOVE` イベント時にマウス座標が `ConfigManager` のロゴ矩形内か判定。必要に応じて `TrackMouseEvent` を呼び出し、`WM_MOUSELEAVE` をトラップしてホバー状態を解除する処理を追加。

### Renderer (WIC Integration)

#### [MODIFY] src/Renderer.h / cpp
*   `Renderer::Initialize` において WICファクトリ (`IWICImagingFactory`) の初期化を追加。
*   メンバ変数 `m_appLogoBitmap`, `m_appLogoHoverBitmap` (いずれも `ID2D1Bitmap`) を追加し、初期化時にハイブリッド方式（ファイル優先、無ければメモリリソース）でデコード・作成するヘルパー関数を実装。
*   `Render(bool isHovered)` にシグネチャを変更。背景クリア後、状態に応じたビットマップを `m_d2dContext->DrawBitmap` で `ConfigManager` で指定された座標に描画する処理を追加。

### Application & Main

#### [MODIFY] src/Application.cpp
*   `Run()` ループ内の `m_renderer.Render()` 呼び出し時に、`m_window.IsHovered()` を引数として渡すよう修正。

#### [MODIFY] src/main.cpp
*   `wWinMain` の先頭に `CoInitializeEx`、終了時に `CoUninitialize` を追加（WIC対応のため）。

## Verification Plan
### Automated Tests
*   CMakeコマンドによるビルドの正常終了確認。

### Manual Verification
*   ビルドされた `OZtone.exe` を実行し、(16, 16) の位置にアプリアイコンが表示されることを確認。
*   アイコン領域にマウスカーソルを合わせ、ホバー画像に切り替わることを確認。
*   exeと同階層の画像ファイルをリネームし、再度起動して内蔵リソースからフォールバック描画されることを確認。
