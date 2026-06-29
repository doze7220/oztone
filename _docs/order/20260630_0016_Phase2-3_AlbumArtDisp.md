
**【今回の目的：Phase 2 - Step 3】**
WICを用いてプレースホルダー用のアルバムアート画像を読み込み、「背景全面（センタリング・Cover表示）」と「左下」の2ヶ所に描画します。
UIレイアウトにおいて、基準点から各要素をオフセット配置するコンポーネント指向のデータ駆動設計を導入します。

**【実装要件】**
1. **リソースとCMake修正**:
   - `assets/placeholder_art.png` をexeに埋め込むための設定を `.rc`, `resource.h`, `CMakeLists.txt` に追加してください（Step2と同様のスキン機能/ハイブリッド読み込み方式）。
2. **ConfigManagerの拡張（レイアウト定義）**:
   - `OZtone.ini` に `[Layout_NowPlaying]` セクションを追加し、以下の項目を読み込めるようにしてください。
     - `BaseX`, `BaseY` (曲情報グループ全体の基準座標。デフォルト例: BaseX=30, BaseY=350)
     - `ArtOffsetX`, `ArtOffsetY` (基準点からの左下アルバムアートの相対座標。デフォルト例: 0, 0)
     - `ArtSize` (左下アルバムアートの描画サイズ。デフォルト例: 120)
     - `BgOpacity` (背景アルバムアートの不透明度/明度。0.0〜1.0。デフォルト例: 0.3)
3. **Rendererクラスの拡張**:
   - 起動時にプレースホルダー画像をハイブリッド方式（ファイル優先、無ければリソース）で D2D1ビットマップとしてロードしてください。
   - `Render` 内での描画順序は「1.背景黒クリア → 2.背景アルバムアート → 3.アプリアイコン(Step2) → 4.左下アルバムアート」とします。
   - **背景アルバムアートの描画**: ウィンドウ背景に隙間ができないよう拡大し、アスペクト比を維持したまま「中央揃え（センタリング）」ではみ出た部分をクリッピングする処理（いわゆる Cover 表示）を計算して描画してください。引数の `opacity` に ConfigManager の `BgOpacity` を適用し暗く表示してください。
   - **左下アルバムアートの描画**: ConfigManager の `BaseX + ArtOffsetX` および `BaseY + ArtOffsetY` を左上座標とし、幅・高さともに `ArtSize` となる矩形へ描画してください。
   - 描画品質向上のため、`DrawBitmap` の補間モードは `D2D1_BITMAP_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC` または `LINEAR` などの高品質なものを指定してください。

**【事後処理】**
実装完了後、`.\build.bat` を実行して自動ビルドテストを行い、エラーがあれば自己修正してください。成功したら `PROJECT_ARCHITECTURE.md` の更新と、`_docs/logs/YYYYMMDD_HHMM_Phase2_Step3.md` へ実装レポートを出力してください。
