##### 作業指示書 REQ: Phase 19-3 Hotfix: TrackDatabaseからのフレーミング情報パージ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-3_TrackDatabase_Analyzer.md）の末尾に「HOTFIX: TrackDatabaseの冗長データパージ」を追加し、対応内容を追記すること。
3. チャットにて「Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
現在、`TrackDatabase` が管理する `TrackMetadata` 構造体、および `oztone_track.odb` への保存フォーマットに、本来プレイリスト側 (`PlaylistItem`) で管理すべきフレーミング情報が含まれてしまっている（二重管理状態）。これを削除し、データ層を純化する。

*   **1. `src/TrackDatabase.h` の修正**
    *   `TrackMetadata` 構造体から `artOffsetX`, `artOffsetY`, `artScale` の3つのメンバ変数を完全に削除する。
*   **2. `src/TrackDatabase.cpp` の修正**
    *   `SaveToFile` における出力フォーマットを `filepath \t title \t artist \t timeString \t peakAmplitude \t maxFrequency` の6要素のみに変更し、フレーミング情報の出力を削除する。
    *   `LoadFromFile` におけるパース処理を修正し、上記の6要素のみを読み込むようにする。（※すでに古いフォーマットで保存された `.odb` を読み込む際のエラーを防ぐため、タブ分割された要素数が6以上あれば正常に読み込み、7番目以降の要素は無視する後方互換の安全策を入れること）。
*   **3. エラー箇所の修正**
    *   `TrackMetadata` からフレーミング情報を削除したことによりコンパイルエラーが発生する箇所（メタデータの更新処理や UI連携部分など）があれば、`PlaylistManager` が保持する `PlaylistItem` から取得・更新する本来の正しいロジックへ修正するか、一時的に無効化してビルドを通すこと。
