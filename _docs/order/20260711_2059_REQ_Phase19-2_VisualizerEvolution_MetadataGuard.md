##### 作業指示書 REQ: Hotfix Phase 19-2: メタデータとFFT解析のフラグ分離 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポートの末尾に「HOTFIX」を追加し、対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
現在、`TrackMetadata` 構造体の `isLoaded` フラグが、「タグ情報の解析完了」と「FFT波形スキャンの完了」の両方の意味を持ってしまっているため、即時再生時のタグ自己修復が行われた瞬間に裏のFFTスキャンがスキップされてしまうバグが発生している。これを2つのフラグに分離して責務を明確化する。

*   **1. 構造体の変更 (`src/PlaylistManager.h`)**
    *   `TrackMetadata` 構造体内の `bool isLoaded;` を削除し、代わりに `bool isMetaLoaded = false;` と `bool isFFTLoaded = false;` の2つのフラグを定義する。

*   **2. フラグ更新ロジックの分離 (`src/PlaylistManager.cpp` 等)**
    *   TSVのロード (`LoadFromFile`) 時：データが揃っている行を読み込んだ際は、`isMetaLoaded = true;` および `isFFTLoaded = true;` の両方をセットする。
    *   タグ情報のみの更新時 (`UpdateTrackMetadataIfNeeded` 等から呼ばれる処理)：`isMetaLoaded = true;` のみをセットし、`isFFTLoaded` は変更しない。
    *   FFT波形解析を含む完全な更新時 (バックグラウンドの `ParseThreadFunc` 等から呼ばれる処理)：算出が完了したピーク値等をセットした上で、`isMetaLoaded = true;` と `isFFTLoaded = true;` の両方をセットする。

*   **3. 解析スレッドの条件変更 (`src/Application.cpp` 等)**
    *   バックグラウンドの解析対象を取得する処理 (`GetUnparsedTracks` や `ParseThreadFunc` 内のスキップ判定) において、評価するフラグを `!isLoaded` から **`!isFFTLoaded`** に変更する。
    *   これにより、タグ情報 (`isMetaLoaded`) が先行して `true` になっていたとしても、FFTスキャンが終わっていなければ確実に裏スレッドでの解析対象となるようにする。
