### REQ: Phase 10-1 Hotfix: TrackInfoWidget のトラックナンバー消失バグの修正 (実装実行)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【現状のステータスと不具合】
プレイリストを開いた直後（最初の曲の再生時）に、左下のトラックナンバー（XXX/XXX）が表示されなかったり、一瞬だけ表示されてすぐ非表示になってしまうバグが発生している。
原因は、Phase 10-4 で実装した「タグ情報のバックグラウンド解析（遅延ロード）」が完了して曲名が切り替わったタイミングで、`TrackInfoWidget::UpdateLayout` が走り、その際に **トラックナンバーのテキストレイアウトキャッシュ (`m_trackCountTextLayout`) まで誤って一緒にクリア (Reset) してしまっている** ためである。

#### 【作業手順（厳守事項）】
事前の計画立案は省略し、即座に以下の手順でコードの修正を実行すること。
1. `src/Widgets.cpp` の `TrackInfoWidget::UpdateLayout` の内部ロジックを精査する。
2. 以下の【実装要件】に従い、キャッシュ更新の独立性を確保する。
3. 修正が完了したら、チャットにて「TrackInfoWidgetのキャッシュ初期化バグの修正が完了しました。動作確認をお願いします」と報告すること。

#### 【実装要件】
* **要件1: キャッシュクリアの分離**
  * `TrackInfoWidget::UpdateLayout` 内において、「曲名・アーティスト名が変化した時のキャッシュ再生成ブロック」と「トラックナンバーが変化した時のキャッシュ再生成ブロック」を完全に独立させること。
  * タイトル等が変化した時の `if` ブロック内で、誤って `m_trackCountTextLayout.Reset()` などを呼び出してトラックナンバーのキャッシュを巻き添えで破棄していないか確認し、もしあれば削除・分離すること。
* **要件2: トラックナンバーの更新条件の厳格化**
  * トラックナンバーのテキストレイアウト再生成の条件式が、`if (m_lastCurrentTrackIndex != ctx.currentTrackIndex || m_lastTotalTracks != ctx.totalTracks)` のように、**インデックスと総曲数の両方の変化を正確に捉えているか** 確認し、漏れがあれば修正すること。
  * `totalTracks == 0` の場合の安全なフォールバック（生成スキップ等）も確認すること。

#### 【作業終了後】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase10-1_hotfix_TrackNoMove.md`）として新規作成する。
2. 原因と、詳細作業内容を記載する。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う。

#### 【対象ファイル】
* `src/Widgets.cpp` (TrackInfoWidget)
