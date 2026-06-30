【AIへの指示（Phase 5 - Step 7 : 先読みデータを活用した「次の曲」表示）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 5 - Step 7】**
先読み（プリフェッチ）されたデータを活用し、ウィンドウ右下を基準点としたレイアウトで「次の曲（極小アルバムアート＋曲名）」をUI上に描画します。ロード中（先読み中）の場合は代替の四角板を表示します。

**【実装要件】**
1. **ConfigManager の拡張 ([Layout_NextTrack] セクション)**:
   - `OZtone.ini` に以下の設定値を読み書きできるように追加してください。
   - `BaseRightOffset` (右端からの基準点オフセット, 例: 250.0f)
   - `BaseBottomOffset` (下端からの基準点オフセット, 例: 80.0f)
   - `ArtSize` (極小アルバムアートのサイズ, 例: 40.0f)
   - `ArtOffsetX`, `ArtOffsetY` (基準点からのアート相対オフセット)
   - `TextOffsetX`, `TextOffsetY` (基準点からのテキスト相対オフセット)

2. **Renderer の拡張 (状態受け取りと右下基準レイアウト)**:
   - `SetNextTrackInfo(bool isReady, ID2D1Bitmap* art, const std::wstring& title, const std::wstring& artist)` メソッドを追加してください。
   - `Render` メソッド内にて、`m_d2dContext->GetSize()` で取得した論理サイズ（width, height）を基に、右下座標（`width - BaseRightOffset`, `height - BaseBottomOffset`）を親基点として計算してください。
   - **ロード中の描画 (`isReady == false`)**:
     - 親基点 + アートオフセットの位置に、`ArtSize` の「半透明の黒、または灰色の四角板 (`FillRectangle`)」を描画してください。
     - テキストは "Loading..." 等を描画してください。
   - **ロード完了時の描画 (`isReady == true`)**:
     - `art` が有効な場合は縮小描画し、無効（画像なし曲）の場合は Phase 4-4 で実装した `FallbackArtOpacity` 等を用いて代替の四角板を描画してください。
     - テキスト領域に "Next: [title] - [artist]" の形式で描画してください。テキストは右揃え（または指定オフセット）で美しく配置してください。

3. **Application の連携**:
   - `Run()` のメインループ内、または適切な更新タイミングで、現在のプリフェッチ状態 `m_isPrefetchReady.load()` と、キャッシュされている「次の曲」の WIC デコード済み画像、タイトル、アーティスト名を `Renderer::SetNextTrackInfo` へ毎フレーム（または状態変化時に）渡すようにしてください。

**【事後処理】**
実装完了後、ビルドを行ってください。曲の再生中に右下領域へ「次の曲」のロード中を示す板が表示され、先読み完了と同時に画像とテキストがパッと反映されることを確認できたら、`PROJECT_ARCHITECTURE.md` のUI仕様を更新し、`_docs/logs/YYYYMMDD_HHMM_Phase5_Step7.md` へ実装レポートを出力してください。
