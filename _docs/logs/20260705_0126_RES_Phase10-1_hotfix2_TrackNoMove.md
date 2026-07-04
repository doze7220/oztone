# Phase 10-1 Hotfix 2: TrackInfoWidgetの曲名・アーティスト名キャッシュ化

## 実装目的
`TrackInfoWidget` において、曲名およびアーティスト名が `DrawText` で毎フレーム直接描画されており、テキストレイアウトのキャッシュが使用されていない状態を解消する。描画ループ内での動的リソース生成を排除するため、`IDWriteTextLayout` によるキャッシュ化を実装する。

## 変更ファイル
* `src/Widgets.h`
* `src/Widgets.cpp`
* `src/Renderer.cpp`
* `PROJECT_ARCHITECTURE.md`

## 詳細作業内容
* **キャッシュ用変数の追加:** `src/Widgets.h` の `TrackInfoWidget` に `m_titleTextLayout`, `m_artistTextLayout`, `m_lastTitle`, `m_lastArtist` を追加。
* **UpdateLayout でのキャッシュ生成:** `TrackInfoWidget::UpdateLayout` 内に、曲名・アーティスト名が変化した場合、またはキャッシュが `null` の場合に `CreateTextLayout` を実行するロジックを追加。生成時の最大幅・高さは大きめに確保（4000x1000）し、描画時に実際のサイズでクリップさせる方式を採用。
* **Draw での描画処理の変更:** `TrackInfoWidget::Draw` 内の `DrawText` を完全に削除し、`DrawTextLayout` に変更。直前に `SetMaxWidth` と `SetMaxHeight` を設定してトリミングを有効化。
* **ReleaseResources:** メモリリークを防ぐため、`m_titleTextLayout` と `m_artistTextLayout` を適切に解放（`Reset()`）する処理を追加。
* **追加作業 (バグ修正):** `Renderer::UpdateTextLayouts` 呼び出し時に `WidgetContext` へ `trackTitle` や `trackArtist` が渡されておらず、空文字でキャッシュ生成が行われ表示が消えるバグが発生したため、`src/Renderer.cpp` にて該当プロパティをセットするよう修正。
* **ドキュメントの更新:** `PROJECT_ARCHITECTURE.md` に `TrackInfoWidget` のキャッシュ化に関する仕様を追記。

## 懸念点
特になし。動的なサイズ変更にも対応可能な形でキャッシュ生成を行っており、リソース再生成のコスト削減効果が見込める。
