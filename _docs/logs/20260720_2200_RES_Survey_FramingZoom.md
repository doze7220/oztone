# 調査レポート: 拡縮機能の現状コード抽出

## 1. `src/Window_Mouse.cpp`
`HandleMouseWheel` メソッド内、`m_onArtFramingScroll` を呼び出している `if` 分岐とその周辺のコード。

```cpp
    if (!m_isPlaylistHovered && !m_isVolumeHovered) {
      if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (wParam & MK_RBUTTON)) {
        if (m_onArtFramingScroll) {
          int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
          m_onArtFramingScroll(static_cast<float>(zDelta), pt.x, pt.y);
        }
        return true;
      }
    }
```

## 2. `src/Application_Initialize.cpp`
`m_window.SetArtFramingScrollCallback` に渡しているラムダ式の中身全体。

```cpp
  m_window.SetArtFramingScrollCallback([this](float delta, int x, int y) {
    if (m_playlistManager.IsEmpty()) return;
    std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
    float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
    m_framingDb.GetFraming(currentTrack, artX, artY, artScale);

    float oldScale = artScale;
    artScale += delta * 0.001f;

    float targetScale = (std::max)(1.0f, artScale);

    float dpiScale = static_cast<float>(GetDpiForWindow(m_window.GetHandle())) / 96.0f;
    float mx = x / dpiScale;
    float my = y / dpiScale;

    if (oldScale > 0.0f) {
        artX = mx - (mx - artX) * (targetScale / oldScale);
        artY = my - (my - artY) * (targetScale / oldScale);
    }

    artScale = targetScale;
    m_renderer.ClampArtFraming(artScale, artX, artY);
    m_framingDb.SetFraming(currentTrack, artX, artY, artScale);
    m_renderer.SetBackgroundFraming(artX, artY, artScale);
    this->ForceRender();
  });
```

## 3. `src/Renderer.cpp`
`void Renderer::ClampArtFraming(float& scale, float& offsetX, float& offsetY)` のメソッドの中身全体。

```cpp
void Renderer::ClampArtFraming(float& scale, float& offsetX, float& offsetY) {
    if (!m_d2dContext || !m_currentBgBitmap) return;

    scale = (std::max)(1.0f, scale);

    D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
    float logicWidth = rtSize.width / m_dpiScale;
    float logicHeight = rtSize.height / m_dpiScale;

    D2D1_SIZE_F imgSize = m_currentBgBitmap->GetSize();
    if (imgSize.width <= 0.0f || imgSize.height <= 0.0f) return;

    float baseScaleX = logicWidth / imgSize.width;
    float baseScaleY = logicHeight / imgSize.height;
    float baseScale = (std::max)(baseScaleX, baseScaleY);

    float actualScale = baseScale * scale;
    float scaledWidth = imgSize.width * actualScale;
    float scaledHeight = imgSize.height * actualScale;

    float maxOffsetX = (scaledWidth - logicWidth) / 2.0f;
    float maxOffsetY = (scaledHeight - logicHeight) / 2.0f;

    if (maxOffsetX < 0.0f) maxOffsetX = 0.0f;
    if (maxOffsetY < 0.0f) maxOffsetY = 0.0f;

    if (offsetX > maxOffsetX) offsetX = maxOffsetX;
    if (offsetX < -maxOffsetX) offsetX = -maxOffsetX;

    if (offsetY > maxOffsetY) offsetY = maxOffsetY;
    if (offsetY < -maxOffsetY) offsetY = -maxOffsetY;
}
```
