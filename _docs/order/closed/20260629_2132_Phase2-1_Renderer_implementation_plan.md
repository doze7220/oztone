# Phase 2 - Step 1: 3D/2D Hybrid Rendering Engine

Direct3D 11とDirect2Dを連携させた描画エンジン（Rendererクラス）を導入し、ウィンドウのクライアント領域を単色で塗りつぶす処理を実装します。

## Proposed Changes

### Build Configuration

#### [MODIFY] CMakeLists.txt
- `target_link_libraries` に `d3d11.lib`, `d2d1.lib`, `dxgi.lib` を追加します。
- ソースとヘッダーのリストに `src/Renderer.cpp`, `src/Renderer.h` を追加します。

---

### C++ Source Code

#### [NEW] src/Renderer.h
- `Renderer` クラスを宣言します。
- `Initialize(HWND hwnd)`: D3D11デバイス、スワップチェイン、D2D1ファクトリ、デバイス、デバイスコンテキスト、レンダーターゲットを初期化します。
- `Render()`: 画面を単色（例: `D2D1::ColorF::Black`）でクリアし、Present を呼び出します。
- `ComPtr` を用いてCOMリソースを管理します。

#### [NEW] src/Renderer.cpp
- `Renderer` クラスの実装。
- `D3D11CreateDeviceAndSwapChain` を使用した初期化。
- IDXGISurface を取得し、それを用いて D2D1 のビットマップ・レンダーターゲットを作成するハイブリッド初期化処理。

#### [MODIFY] src/Application.h
- `Renderer` クラスのインスタンス（`m_renderer`）をメンバとして追加します。

#### [MODIFY] src/Application.cpp
- `Initialize` メソッド内で、ウィンドウ作成後に `m_renderer.Initialize(m_window.GetHandle())` を呼び出します。
- `Run` メソッドのメインループ内で、毎フレーム `m_renderer.Render()` を呼び出します。

---

### Documentation

#### [MODIFY] PROJECT_ARCHITECTURE.md
- 「5. 実装済みクラス・関数リファレンス」に `Renderer` クラスの情報を追記します。

#### [NEW] _docs/logs/20260629_2253_Phase2_Step1.md
- 実装完了後に、今回の作業ログを出力します（ファイル名は実行時の日時に合わせます）。

## Verification Plan

### Automated Tests
- CMake でのビルドエラーやリンクエラーがないことを確認します。

### Manual Verification
- ユーザーにビルド・実行していただき、ウィンドウの背景が真っ黒に描画されること（初期化時のデフォルトの白背景ではなくなること）を確認していただきます。
