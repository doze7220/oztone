現在のプロジェクト（Phase 2 Step 1完了状態）をベースに、次の実装を開始します。

**【今回の目的：Phase 2 - Step 2】**
WICを用いて画面左上にアプリアイコンを描画します。画像は「基本はexe内の埋め込みリソースを使用し、exeと同階層に同名の画像ファイルが存在する場合はそれを優先して読み込む（スキン機能）」というハイブリッド方式とします。また、マウスホバーによる画像切り替えと、レイアウトのINIデータ駆動設計も導入します。

**【実装要件】**
1. **リソース埋め込みとCMake修正**:
   - `assets/app_logo.png` と `assets/app_logo_hover.png` をexeに埋め込むためのリソーススクリプト（`.rc`）と `resource.h` を新規作成してください。
   - `CMakeLists.txt` を修正し、RCファイルのコンパイル有効化と、テスト用に `assets` 内の画像をexeと同ディレクトリにコピーする設定を追加してください。
2. **ConfigManagerの拡張**:
   - `OZtone.ini` に `[Layout_AppLogo]` セクションを追加し、`X, Y, Width, Height` を読み込めるように実装してください（デフォルト値例: X=16, Y=16, Width=64, Height=64）。
3. **Rendererクラスの拡張（ハイブリッド画像読み込み）**:
   - WICファクトリを初期化し、起動時に2枚の画像をロードして D2D1ビットマップとして保持してください。
   - ロード処理は「①exeと同階層に該当ファイルが存在するかチェックし、あればファイルからデコード」「②無ければ `FindResource`/`LoadResource` を用いてメモリ上のリソースからストリームを作成してデコード」というフォールバック処理を実装してください。
   - `Render(bool isHovered)` を実装し、背景クリア後、状態に応じてどちらかの画像を `ConfigManager` の設定座標へ描画してください。
4. **Windowクラスの拡張（ホバー判定）**:
   - `WM_MOUSEMOVE` と `TrackMouseEvent` (`WM_MOUSELEAVE`) を用いて、マウスが `ConfigManager` で指定されたアイコンの矩形領域内にあるかを判定し、その状態を `Renderer::Render` に渡してください。
5. **プロジェクト憲法の厳守**:
   - `Window` クラス内に描画処理やリソース読み込み処理を絶対に書かず、すべて `Renderer` または専用クラスにカプセル化すること。

**【事後処理】**
実装が完了したらビルドテストを行い、`PROJECT_ARCHITECTURE.md` に今回のスキン仕様（リソースフォールバック）とWICの情報を追記し、`_docs/logs/YYYYMMDD_HHMM_Phase2_Step2.md` に実装レポートを出力してください。
