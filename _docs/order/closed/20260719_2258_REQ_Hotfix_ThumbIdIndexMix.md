##### 作業指示書 REQ: サムネイルIDへのインデックス混入バグの修正 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトはサムネイルIDへのインデックス混入バグ修正のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbIdIndexMix.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル:** `src/Application_Playback.cpp` (および、`dataProvider` ラムダ式を定義している箇所)
*   ドラムスロットへのデータ注入（`StartDrumAnimation` に渡す `dataProvider` ラムダ式等）において、サムネイルID (`thumbId`) に、プレイリストの曲インデックス（`targetIndex` 等）が誤って代入されている致命的なバグを修正する。
*   `slot.thumbId = targetIndex;` などの誤った代入を直ちに削除すること。
*   必ず該当する楽曲のファイルパス (`filepath`) を用いて、`m_thumbnailDatabase.GetOrGenerateThumbId(filepath, isNew)` を呼び出し、正規のサムネイルIDを取得して `slot.thumbId` にセットするように修正すること。

#### 【絶対遵守ルール (Constraints)】
*   **エンキューの絶対禁止**: `dataProvider` 内で `GetOrGenerateThumbId` を呼び出した結果、新規発番 (`isNew == true`) であったとしても、その場で絶対に `m_thumbCacher.EnqueueTrack` を呼び出してサムネイル生成を発注してはならない（司令塔の別ロジックでのみ発注する原則を維持するため）。
