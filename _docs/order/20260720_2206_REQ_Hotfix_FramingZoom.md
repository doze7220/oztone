### 作業指示書 REQ: Phase 23-7 Hotfix 4 : 画像拡縮の強制オーバーライド (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md

#### 【作業手順（厳守事項）】
本プロンプトは、不完全な状態にあるフレーミング拡縮機能を、指定したコードで強制的に上書き修正するための Hotfix である。
1. 以下の【強制上書きコード】に従い、対象となるファイルの該当箇所を **1文字も改変せず、省略せずにそのまま上書き** すること。
2. 作業完了後、作業レポート（D:\ozlab\oztone\_docs\logs\20260720_RES_Hotfix_ForceFramingZoom.md）を新規作成し、対応内容を記録すること。
3. チャットにて「拡縮機能の強制オーバーライドが完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【強制上書きコード】

**1. `src/Window.h` の修正**
`SetArtFramingScrollCallback` のシグネチャを必ず以下のようにし、`m_onArtFramingScroll` もこれに合わせること。
```cpp
void SetArtFramingScrollCallback(std::function<void(float, int, int)> cb) { m_onArtFramingScroll = cb; }
// メンバ変数: std::function<void(float, int, int)> m_onArtFramingScroll;
2. src/Window_Mouse.cpp の修正 HandleMouseWheel 内のフレーミング拡縮の呼び出し部分を以下で完全に上書きすること。
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (wParam & MK_RBUTTON)) {
        if (m_onArtFramingScroll) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(m_hwnd, &pt);
            m_onArtFramingScroll(static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)), pt.x, pt.y);
        }
        return true;
    }
3. src/Application_Initialize.cpp の修正 SetArtFramingScrollCallback のラムダ式を以下で完全に上書きすること。
  m_window.SetArtFramingScrollCallback([this](float delta, int x, int y) {
    if (m_playlistManager.IsEmpty()) return;
    std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
    float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
    m_framingDb.GetFraming(currentTrack, artX, artY, artScale);

    // 論理座標への変換
    float logicalX = x / m_renderer.GetDpiScale();
    float logicalY = y / m_renderer.GetDpiScale();

    // ズーム前のマウスポインタ位置の画像上の相対座標
    float relX = (logicalX - artX) / artScale;
    float relY = (logicalY - artY) / artScale;

    // スケール変更と下限ガード
    artScale += delta * 0.001f;
    if (artScale < 1.0f) artScale = 1.0f;

    // ズーム後の新しいオフセット計算（マウス位置がズレないように補正）
    artX = logicalX - (relX * artScale);
    artY = logicalY - (relY * artScale);

    m_renderer.ClampArtFraming(artScale, artX, artY);
    m_framingDb.SetFraming(currentTrack, artX, artY, artScale);
    m_renderer.SetBackgroundFraming(artX, artY, artScale);
    this->ForceRender();
  });
4. src/Renderer.cpp の修正 ClampArtFraming メソッドの中身を以下で完全に上書きすること。
void Renderer::ClampArtFraming(float& scale, float& offsetX, float& offsetY) {
    if (!m_d2dContext || !m_currentBgBitmap) return;

    D2D1_SIZE_F size = m_d2dContext->GetSize();
    D2D1_SIZE_F bmpSize = m_currentBgBitmap->GetSize();

    // 画像が画面を覆うためのベーススケール計算
    float scaleX = size.width / bmpSize.width;
    float scaleY = size.height / bmpSize.height;
    float baseScale = std::max(scaleX, scaleY);

    if (scale < 1.0f) scale = 1.0f; // スケールの再ガード

    float scaledWidth = bmpSize.width * baseScale * scale;
    float scaledHeight = bmpSize.height * baseScale * scale;

    // はみ出しを許容しないための限界座標計算
    float minX = size.width - scaledWidth;
    float minY = size.height - scaledHeight;

    // オフセットのクランプ処理
    if (offsetX > 0.0f) offsetX = 0.0f;
    if (offsetX < minX) offsetX = minX;

    if (offsetY > 0.0f) offsetY = 0.0f;
    if (offsetY < minY) offsetY = minY;
}