##### 作業指示書 REQ: Phase 19-3 Hotfix: TrackDatabaseの安全なマージ更新ロジック実装 (実装実行)
以下のプロジェクトルールと開発資料、実装計画兼実装レポートを熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260711_2235_RES_Phase19-3_TrackDatabase_Analyzer.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（20260711_2235_RES_Phase19-3_TrackDatabase_Analyzer.md）の末尾に「HOTFIX: TrackDatabaseの安全なマージ更新ロジック実装」を追加し、対応内容を追記すること。
3. チャットにて「Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
現在、`TrackDatabase::SetMetadata` が単純な上書き保存となっているため、タグ解析スレッドとFFTスキャンスレッドが非同期にデータを更新した際、互いのデータ（isMetaLoaded と isFFTLoaded の管轄データ）を上書きして破壊してしまう懸念がある。これを防ぐため、賢いマージ更新を行う `UpdateMetadata` メソッドを新設する。

*   **1. `src/TrackDatabase.h` の修正**
    *   `void UpdateMetadata(const std::wstring& filepath, const TrackMetadata& newData);` のメソッド宣言を追加する。
*   **2. `src/TrackDatabase.cpp` の修正**
    *   `UpdateMetadata` を実装する。内部で `std::lock_guard<std::mutex> lock(m_mutex);` を用いてスレッドセーフにする。
    *   指定された `filepath` が既にDBに存在する場合、`newData.isMetaLoaded` が `true` なら、既存データのタグ関連情報（`title`, `artist`, `timeString`, `isMetaLoaded`）のみを上書きする。
    *   同様に、`newData.isFFTLoaded` が `true` なら、既存データのFFT関連情報（`peakAmplitude`, `maxFrequency`, `isFFTLoaded`）のみを上書きする。
    *   DBに存在しない場合は、そのまま `m_database[filepath] = newData;` として新規登録する。
*   **3. 呼び出し元の修正 (`Application.cpp` / `TrackAnalyzer.cpp`)**
    *   `Application.cpp` のメタデータ自己修復ロジック（`UpdateTrackMetadataIfNeeded`等）や、`TrackAnalyzer.cpp` の解析スレッドから `TrackDatabase` を更新している箇所において、単純な `SetMetadata` ではなく、新設した `UpdateMetadata` を呼び出すように変更し、データの上書き破壊を完全に防止する。
