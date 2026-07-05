### 作業指示書 REQ: Phase 16-4 Hotfix 2: プレイリスト削除バグおよび解析キャッシュ保存バグの修正 (実装実行)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase16-4_PlaylistInfo.md`）の末尾に「5. HOTFIX 2」の項目を追加し、原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

#### 【実装要件】
現在発生している2つのバグを修正する。

* **要件1: プレイリスト削除時の挙動とキャッシュ更新バグの修正 (Application)**
  * `Application::OnPlaylistToolbarClick` 内のプレイリスト一覧モード時のリスト削除処理（`buttonIndex == 2`）を以下のように修正する。
  * 削除対象を、現在フォーカスされているプレイリスト（`m_focusedPlaylistIndex` から算出したファイルパス）とする。
  * 対象ファイルを物理削除（`std::filesystem::remove`等）した後、直ちに `UpdatePlaylistSummaries()` を呼び出して一覧の表示キャッシュを更新する。
  * 削除したファイルが「現在再生中のプレイリスト（`m_config->GetDefaultPlaylistPath()` と一致）」であった場合のみ、別のプレイリスト（例: リストの先頭等）へフォールバックするため `SwitchPlaylist` を実行し、一覧モードを終了する。
  * 現在再生中のプレイリスト以外を削除した場合は、再生を継続したまま一覧モードの表示だけを更新する（`SwitchPlaylist`を行わない）。
* **要件2: バックグラウンド解析スレッドのファイル保存バグ修正 (Application)**
  * `Application::ParseThreadFunc` におけるバックグラウンドタグ解析処理において、メモリ上のメタデータ更新(`m_playlistManager.UpdateMetadata`)が行われているものの、ファイルへの永続化が行われていないバグを修正する。
  * I/O負荷を軽減するため、キュー (`m_parseQueue`) が空になったタイミング（未解析のトラックを一通り処理し終えてスリープ・待機状態に入る直前）に、`m_playlistManager.SaveToFile(m_config->GetDefaultPlaylistPath())` を一度だけ呼び出し、現在再生中のTSVファイルへ確実に保存するロジックを追加する。

