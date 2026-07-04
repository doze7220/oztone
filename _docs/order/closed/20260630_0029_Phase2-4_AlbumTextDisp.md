**【今回の目的：Phase 2 - Step 4】**
DirectWriteを用いたテキスト描画基盤を導入し、曲情報（ダミーの曲名・アーティスト名）をコンポーネント指向のデータ駆動設計で画面に描画します。

**【実装要件】**
1. **CMake修正**:
   - `CMakeLists.txt` のリンクライブラリに `dwrite.lib` を追加してください。
2. **ConfigManagerの拡張（曲情報テキストのレイアウト定義）**:
   - `OZtone.ini` の `[Layout_NowPlaying]` セクションに、以下の項目を追加し読み込めるようにしてください。
     - `TitleOffsetX`, `TitleOffsetY`, `TitleFontSize` (基準点からの曲名の相対座標とフォントサイズ。デフォルト例: 140, 10, 32)
     - `ArtistOffsetX`, `ArtistOffsetY`, `ArtistFontSize` (基準点からのアーティスト名の相対座標とフォントサイズ。デフォルト例: 140, 55, 18)
3. **Rendererクラスの拡張（DirectWriteの導入）**:
   - 起動時に `DWriteCreateFactory` を用いて DirectWrite ファクトリを初期化してください。
   - 白の単色ブラシ（SolidColorBrush）と、曲名用・アーティスト名用の `IDWriteTextFormat` をそれぞれ作成してください（フォントは "Meiryo" または "Yu Gothic UI" などを指定）。
   - `Render` メソッド内の「4.左下アルバムアート」の描画の後に、ダミーテキスト（例: "Dummy Track Title", "Dummy Artist Name"）を `DrawText` または `DrawTextLayout` で描画する処理を追加してください。
   - 描画位置の起点は、ConfigManagerから取得した `BaseX + TitleOffsetX` と `BaseY + TitleOffsetY` などの計算結果を用いてください。

**【事後処理】**
実装完了後、`.\build.bat` を実行して自動ビルドテストを行い、エラーがあれば自己修正してください。成功したら `PROJECT_ARCHITECTURE.md` の更新と、`_docs/logs/YYYYMMDD_HHMM_Phase2_Step4.md` へ実装レポートを出力してください。
