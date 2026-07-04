# 実装レポート: Phase 10-1 TrackNoMove

## 1. アーキテクチャの絶対制約
*   **Rendererの状態非保持**: ウィジェット間で情報をやり取りする際は、必ず `WidgetContext` を経由する。Renderer自体に状態を持たせないこと。
*   **責任の分離**: UI領域の計算は `LayoutCalculator`、設定のロード・保存は `ConfigManager`、描画処理と描画リソースの管理は `Widgets`（`TrackInfoWidget`, `PlaylistWidget`）の各クラスで完全に分担すること。

## 2. 実装方針・手順
今回の目的は、プレイリスト表示部にある「トラック番号（TRACK XXX/XXX）」を、左下の現在再生中情報（TrackInfoWidget）の領域に移動し、表示形式を「XXX/XXX」に変更することである。

### 対象ファイルと変更概要
*   **`src/ConfigManager.h`, `src/ConfigManager.cpp`**
    *   現在 `[Layout_Playlist]` セクションにある `TrackCountFontFamily`, `TrackCountFontSize`, `TrackCountLetterSpacing`, `TrackCountShadow...` などを `[Layout_NowPlaying]` セクションの読み込みに変更する。
    *   `TrackCountRightOffset`, `TrackCountBottomOffset` を削除し、代わりに `TrackCountOffsetX`, `TrackCountOffsetY` を追加する（デフォルト値は `[Layout_NowPlaying]` 基準の適当な値、例えば X=20, Y=100 などとする）。
    *   `SaveDefaultSettings` 等の出力先も変更する。

*   **`src/LayoutCalculator.h`, `src/LayoutCalculator.cpp`**
    *   `PlaylistLayout` 構造体および `CalculatePlaylistLayout` からトラック番号用の計算処理を削除する。
    *   `TrackInfoLayout` 構造体および `CalculateTrackInfoLayout` にトラック番号の矩形計算を追加する。
    *   新しい計算は `TrackCountOffsetX`, `TrackCountOffsetY` を用い、ウィンドウ（あるいは基準）の左上からの絶対座標・相対座標として算出する。

*   **`src/Widgets.h`, `src/Widgets.cpp`**
    *   **`PlaylistWidget`**: トラック番号テキストの生成・描画、および状態管理用変数（`m_lastCurrentTrackIndex`, `m_trackCountTextLayout` など）を削除する。
    *   **`TrackInfoWidget`**: 削除した状態管理変数とリソース生成処理をこちらに追加。
    *   テキスト文字列の生成部分で `swprintf_s(buf, L"TRACK %zu/%zu", ...)` となっていた部分を `swprintf_s(buf, L"%zu/%zu", ...)` に変更する。
    *   テキストアライメントを右寄せ（`DWRITE_TEXT_ALIGNMENT_TRAILING` 等）から左寄せ（`DWRITE_TEXT_ALIGNMENT_LEADING`）に変更する。

## 3. タスクリスト

- [x] タスク1: `ConfigManager.h` および `ConfigManager.cpp` の改修（変数の置換とセクション変更）
- [x] タスク2: `LayoutCalculator.h` および `LayoutCalculator.cpp` の改修（レイアウト計算処理の移動）
- [x] タスク3: `Widgets.h` および `Widgets.cpp` の改修（PlaylistWidgetから削除）
- [x] タスク4: `Widgets.h` および `Widgets.cpp` の改修（TrackInfoWidgetへの追加・フォーマット変更・アライメント修正）
- [ ] タスク5: ビルド確認と動作確認

## 4. 詳細作業内容
* **ConfigManager**:
  * `TrackCountRightOffset` / `TrackCountBottomOffset` を `TrackCountOffsetX` / `TrackCountOffsetY` に変更し、INIファイルのセクションを `[Layout_NowPlaying]` に移動しました。
* **LayoutCalculator**:
  * トラック番号のレイアウト計算を `CalculatePlaylistLayout` から `CalculateTrackInfoLayout` に移動し、絶対座標・相対座標計算を適用しました。
* **Widgets**:
  * `PlaylistWidget` から `m_trackCountTextLayout` などの変数と描画ロジックを削除しました。
  * `TrackInfoWidget` に関連変数・リソース作成処理・描画ロジックを追加し、フォーマットを `%zu/%zu`（「TRACK」を除外）に変更、テキストアライメントを左寄せ（LEADING）に設定しました。

### 追加実装: トラック番号のテキストアライメント動的設定
* **要件1 (ConfigManager)**: `[Layout_NowPlaying]` セクションに `TrackCountTextAlignment` を追加し、`0` (左寄せ), `1` (右寄せ: デフォルト), `2` (中央揃え) の設定値を取得できるようにしました。
* **要件2 (LayoutCalculator)**: トラック番号用の計算処理において、固定幅（`200.0f`）のテキストボックスを算出するロジックが正常に適用されていることを確認しました。
* **要件3 (TrackInfoWidget)**: `UpdateLayout` メソッドにて、テキストレイアウトの生成前に `ConfigManager` から取得した `TrackCountTextAlignment` に応じて動的に `SetTextAlignment()` を適用するロジックを追加しました。

### デバッグ・ビルドエラー対応内容
* **LayoutCalculator.cpp**:
  * コード置換処理の不具合により欠落・混入してしまった複数のレイアウト計算関数（`CalculateNextTrackLayout` や `CalculatePlaylistLayout` 等）を元の正しい構造に完全に復元し、構文エラーを解消しました。
* **Widgets.cpp (`PlaylistWidget::Draw`)**:
  * トラック番号の変数を移動させた後、`PlaylistWidget` 側に残存していた不要なNULLチェック（`!m_trackCountTextFormat`）を削除し、未定義識別子のビルドエラーを解消しました。
