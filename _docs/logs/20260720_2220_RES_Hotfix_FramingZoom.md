# RES:HOTFIX作業レポート: 画像拡縮の強制オーバーライドと論理バグの完全修正

## 1. 実装目的
右クリック＋ホイールによる背景画像の拡大縮小操作において、「画像が微動だにしない」「ズームの中心が狂う」という問題を完全に解決する。
前回のHotfixおよび強制上書きコードで発生していた、オフセットクランプの座標系不一致、DPIスケーリングの無視、およびプレイリストのピン留めによるイベントブロックを解消し、正しい座標系（画面中央を原点とする相対オフセット）に基づいたズーム中心補正を実装する。

## 2. 調査内容
*   **イベントブロック**: `Window_Mouse.cpp` において、プレイリストがピン留めされているとマウス位置に関わらず `m_isPlaylistHovered` が `true` となり、背景ホイールの判定がスキップされる状態になっていた。
*   **コンパイルエラー**: 強制上書きコード内で `m_renderer.GetDpiScale()` が使用されていたが、`Renderer.h` に該当のメソッドが存在しないためビルドが通らない状態だった。
*   **座標系の矛盾**: `Renderer::ClampArtFraming` が絶対座標（左上原点）を前提としたクランプを行っていたのに対し、描画側の `LayoutCalculator` は相対オフセット（中央原点）を前提としていたため、クランプが描画を相殺し、画像が中央に張り付いて微動だにしない状態になっていた。

## 3. 対象ファイル
*   `src/Renderer.h`
*   `src/Window_Mouse.cpp`
*   `src/Renderer.cpp`
*   `src/Application_Initialize.cpp`

## 4. 実装タスクリスト
[x] タスク1: `GetDpiScale()` メソッドの追加
[x] タスク2: プレイリストピン留め時のイベントブロック解消
[x] タスク3: `ClampArtFraming` の相対オフセットベースへの修正
[x] タスク4: ズーム中心を維持するオフセット補正計算の完全改修

## 5. 詳細作業内容（新規チャット用・実装コード詳細）

*   **タスク1: `GetDpiScale()` メソッドの追加**
    - `src/Renderer.h` の `public` 領域に以下を追加:
      `float GetDpiScale() const { return m_dpiScale; }`

*   **タスク2: プレイリストピン留め時のイベントブロック解消**
    - `src/Window_Mouse.cpp` の `HandleMouseWheel` メソッド内の以下の条件式を修正:
      ```cpp
      // 修正前: if (!m_isPlaylistHovered && !m_isVolumeHovered)
      bool inPlaylist = m_isPlaylistHovered && IsInPlaylistRegion(pt.x, pt.y);
      if (!inPlaylist && !m_isVolumeHovered) {
          if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (wParam & MK_RBUTTON)) { ... }
      }
      ```

*   **タスク3: `ClampArtFraming` の相対オフセットベースへの修正**
    - `src/Renderer.cpp` の `ClampArtFraming` メソッドの中身を以下に完全に置き換える:
      ```cpp
      void Renderer::ClampArtFraming(float& scale, float& offsetX, float& offsetY) {
          scale = (std::max)(1.0f, scale);
          if (!m_d2dContext || !m_currentBgBitmap) return;

          D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
          float logicWidth = rtSize.width / m_dpiScale;
          float logicHeight = rtSize.height / m_dpiScale;

          D2D1_SIZE_F bmpSize = m_currentBgBitmap->GetSize();
          if (bmpSize.width <= 0 || bmpSize.height <= 0) return;

          float scaleX = logicWidth / bmpSize.width;
          float scaleY = logicHeight / bmpSize.height;
          float baseScale = (std::max)(scaleX, scaleY);
          float finalScale = baseScale * scale;

          float drawWidth = bmpSize.width * finalScale;
          float drawHeight = bmpSize.height * finalScale;

          float limitX = (std::max)(0.0f, (drawWidth - logicWidth) / 2.0f);
          float limitY = (std::max)(0.0f, (drawHeight - logicHeight) / 2.0f);

          offsetX = std::clamp(offsetX, -limitX, limitX);
          offsetY = std::clamp(offsetY, -limitY, limitY);
      }
      ```

*   **タスク4: ズーム中心を維持するオフセット補正計算の完全改修**
    - `src/Application_Initialize.cpp` の `SetArtFramingScrollCallback` 内のズーム計算ロジックを以下に置き換える:
      ```cpp
      float logicalX = x / m_renderer.GetDpiScale();
      float logicalY = y / m_renderer.GetDpiScale();
      
      RECT clientRect;
      GetClientRect(m_window.GetHandle(), &clientRect);
      float logicalWidth = clientRect.right / m_renderer.GetDpiScale();
      float logicalHeight = clientRect.bottom / m_renderer.GetDpiScale();
      float centerX = logicalWidth / 2.0f;
      float centerY = logicalHeight / 2.0f;
      
      float oldScale = artScale;
      artScale += delta * 0.001f;
      if (artScale < 1.0f) artScale = 1.0f;
      
      if (oldScale > 0.0f) {
          float relX = logicalX - centerX;
          float relY = logicalY - centerY;
          artX = artX - relX * (artScale / oldScale - 1.0f);
          artY = artY - relY * (artScale / oldScale - 1.0f);
      }
      ```
