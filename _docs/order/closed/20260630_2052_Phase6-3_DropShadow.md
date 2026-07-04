【AIへの指示（Phase 6 - Step 3 : UI全体のドロップシャドウ対応）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 3 (全体ドロップシャドウ対応)】**
背景画像が明るい場合でもUI要素の視認性を確保するため、テキスト（曲名・アーティスト名等）、アルバムアート、NEXT表示の画像、および左上のアプリアイコンのすべてに対してドロップシャドウを追加します。影の設定はINIファイルから調整可能にします。

**【実装要件】**
1. **ConfigManager の拡張**:
   - `DEFAULT_INI_CONTENT` の `[Layout_AppLogo]`, `[Layout_NowPlaying]`, `[Layout_NextTrack]` の各セクションに以下の項目を追加し、読み込み機能とゲッターを実装してください。
     - `ShadowOffsetX=2.0`
     - `ShadowOffsetY=2.0`
     - `ShadowOpacity=0.7`

2. **Renderer クラス - テキストの影**:
   - 曲情報およびNEXT表示のテキストを描画する直前に、オフセットを加算した座標へ、指定された透明度の黒色ブラシで `DrawTextLayout` を実行し、その直後に本来の色・座標で上書き描画してください。

3. **Renderer クラス - 四角形画像（アルバムアート・NEXTアート）の影**:
   - アルバムアートやNEXT表示の画像を描画（`DrawBitmap`）する直前に、オフセットを加算した座標へ、画像と同じサイズの黒色半透明の矩形を `FillRectangle` で描画し、影としてください。

4. **Renderer クラス - アプリアイコンの影**:
   - アプリアイコンは透過（アルファチャンネル）を持つため、単純な矩形ではなく Direct2D の `ID2D1Effect` (`CLSID_D2D1Shadow` など) を用いて影を生成してください。
   - `m_d2dContext->CreateEffect` でエフェクトを作成し、オフセット位置へ `DrawImage` で影を描画した後、元のアプリアイコンを `DrawBitmap` で上書きするロジックを実装してください（※必要な GUID や `#include <d2d1effects.h>` の追加忘れに注意すること）。

**【追加指示１】**
すでに実装されているドロップシャドウ機能に対してINIファイルからのON/OFF切り替えスイッチを導入します。また、背景アルバムアート全体を少し暗くする「ダークオーバーレイ（背景暗化）」機能を追加します。
これらの視覚効果はUI全体に影響するため、INIファイルに新設する `[Layout_Window]` セクションで一元管理するアーキテクチャに修正します。

**【実装要件１】**
1. **ConfigManager の拡張と `[Layout_Window]` の新設**:
   - `DEFAULT_INI_CONTENT` に、全体の視覚効果を管理する新しいセクション `[Layout_Window]` を追加してください。
   - `[Layout_Window]` セクション内に以下の項目を追加し、読み込み処理とゲッターを追加してください。
     - `EnableShadow=1` （0 または 1 の bool 値）
     - `ShadowOffsetX=2.0` （float 値）
     - `ShadowOffsetY=2.0` （float 値）
     - `ShadowOpacity=0.7` （float 値）
     - `BgDarkenOpacity=0.3` （float 値）
   - ※もし以前の実装で `[Layout_AppLogo]` や `[Layout_NowPlaying]` 等に個別の影設定を持たせていた場合は、それらを廃止し、すべてこの `[Layout_Window]` の設定値を参照するように一本化してください。

2. **Renderer クラス - 影のON/OFF分岐と一元化**:
   - `Renderer::Render` メソッド内において、「テキスト」「アルバムアート」「NEXT画像」「アプリアイコン」の各ドロップシャドウ描画処理が、ConfigManager の `[Layout_Window]` から取得した `EnableShadow` が `true` の場合のみ実行されるように `if` 条件で囲んでください。また、影のオフセットや透明度も同セクションの共通値を使用するように修正してください。

3. **Renderer クラス - 背景暗化（ダークオーバーレイ）処理の追加**:
   - `Renderer::Render` メソッド内にて、一番下になる「背景アルバムアート（Cover表示）」または「代替プレースホルダー」を描画した直後（各種UI等を描画する前）に、ダークオーバーレイを描画してください。
   - ConfigManager から取得した `BgDarkenOpacity` が `0.0f` より大きい場合、画面全体に対して、指定された不透明度の黒色ブラシ（`D2D1::ColorF(0.0f, 0.0f, 0.0f, opacity)`）を用いて `FillRectangle` を実行し、背景全体の輝度を落としてください。

**【追加指示１】**
すでに実装されているドロップシャドウ機能に対してINIファイルからのON/OFF切り替えスイッチを導入します。また、背景アルバムアート全体を少し暗くする「ダークオーバーレイ（背景暗化）」機能を追加します。
これらの視覚効果はUI全体に影響するため、INIファイルに新設する `[Layout_Window]` セクションで一元管理するアーキテクチャに修正します。

**【実装要件１】**
1. **ConfigManager の拡張と `[Layout_Window]` の新設**:
   - `DEFAULT_INI_CONTENT` に、全体の視覚効果を管理する新しいセクション `[Layout_Window]` を追加してください。
   - `[Layout_Window]` セクション内に以下の項目を追加し、読み込み処理とゲッターを追加してください。
     - `EnableShadow=1` （0 または 1 の bool 値）
     - `ShadowOffsetX=2.0` （float 値）
     - `ShadowOffsetY=2.0` （float 値）
     - `ShadowOpacity=0.7` （float 値）
     - `BgDarkenOpacity=0.3` （float 値）
   - ※もし以前の実装で `[Layout_AppLogo]` や `[Layout_NowPlaying]` 等に個別の影設定を持たせていた場合は、それらを廃止し、すべてこの `[Layout_Window]` の設定値を参照するように一本化してください。

2. **Renderer クラス - 影のON/OFF分岐と一元化**:
   - `Renderer::Render` メソッド内において、「テキスト」「アルバムアート」「NEXT画像」「アプリアイコン」の各ドロップシャドウ描画処理が、ConfigManager の `[Layout_Window]` から取得した `EnableShadow` が `true` の場合のみ実行されるように `if` 条件で囲んでください。また、影のオフセットや透明度も同セクションの共通値を使用するように修正してください。

3. **Renderer クラス - 背景暗化（ダークオーバーレイ）処理の追加**:
   - `Renderer::Render` メソッド内にて、一番下になる「背景アルバムアート（Cover表示）」または「代替プレースホルダー」を描画した直後（各種UI等を描画する前）に、ダークオーバーレイを描画してください。
   - ConfigManager から取得した `BgDarkenOpacity` が `0.0f` より大きい場合、画面全体に対して、指定された不透明度の黒色ブラシ（`D2D1::ColorF(0.0f, 0.0f, 0.0f, opacity)`）を用いて `FillRectangle` を実行し、背景全体の輝度を落としてください。

**【事後処理】**
実装完了後、ビルドを行ってください。白い画像などを背景にした際でも、すべてのテキスト、アプリアイコン、各アルバムアート画像に影が落ちて視認性が確保されていること、INIファイルから各要素の影が調整できることを確認できたら、実装レポートを出力してください。
