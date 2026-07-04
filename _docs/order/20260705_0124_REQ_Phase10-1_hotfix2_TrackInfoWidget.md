### REQ: Phase 10-1 Hotfix 2: TrackInfoWidgetの曲名・アーティスト名キャッシュ化 (実装実行)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【現状のステータスと不具合】
`TrackInfoWidget` において、曲名およびアーティスト名が `DrawText` を用いて毎フレーム直接描画されており、テキストレイアウトのキャッシュ（`IDWriteTextLayout`）が使用されていない状態であることが判明した。
これは Phase 9-4 で定めた「描画ループ内での動的リソース生成の排除」というアーキテクチャ設計に反しており、内部的なレイアウト生成コストが毎フレーム無駄に発生してしまっている。

#### 【作業手順（厳守事項）】
事前の計画立案は省略し、即座に以下の手順でコードの修正を実行すること。
1. `src/Widgets.h`, `src/Widgets.cpp` の `TrackInfoWidget` を改修し、曲名・アーティスト名のテキストレイアウトをキャッシュする処理を実装すること。
2. 修正が完了したら、チャットにて「曲名・アーティスト名のキャッシュ化が完了しました。動作確認をお願いします」と報告すること。

#### 【実装要件】
* **要件1: キャッシュ用変数の追加**
  * `TrackInfoWidget` に `m_titleTextLayout`, `m_artistTextLayout` (`ComPtr<IDWriteTextLayout>`) を追加する。
  * 再生成の判定用に、前回の曲名・アーティスト名を保持する変数（`m_lastTitle`, `m_lastArtist`）を追加する。
* **要件2: UpdateLayout でのキャッシュ生成**
  * `TrackInfoWidget::UpdateLayout` 内で、曲名・アーティスト名が前回から変化した場合、またはキャッシュが `null` の場合（ウィンドウリサイズ等）に、`CreateTextLayout` を用いて曲名とアーティスト名のレイアウトを生成・キャッシュするロジックを実装する。
* **要件3: Draw での描画処理の変更**
  * `TrackInfoWidget::Draw` 内の `DrawText` 呼び出しを完全に削除し、`DrawTextLayout` に変更して、生成したキャッシュを使用して描画するように修正する。

#### 【作業終了後】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase10-1_hotfix2_TrackNoMove.md`）として新規作成する。
2. 原因と、詳細作業内容を記載する。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う。

#### 【対象ファイル】
* `src/Widgets.h`
* `src/Widgets.cpp`
