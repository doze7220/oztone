##### 作業指示書 REQ: Hotfix / アーティストメタ情報なしの表示変更 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトはアーティスト名未取得時のフォールバック文字列の変更のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ArtistFallback.md）として新規作成すること。作業レポートに対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
本タスクでは、OZtoneのミニマルなUI美学を向上させるため、曲のメタデータからアーティスト名が取得できなかった場合のフォールバック表示を、従来の「---」から「」（空文字）へと変更する。

*   `src/FileManager.cpp`（`ExtractTextMetadata`等）、`src/TrackAnalyzer.cpp`、および `Application` 層のソースコードを調査し、アーティスト名が取得できなかった際に `L"---"` を代入している箇所を特定し、すべて `L""`（空文字）を代入するように修正すること。
*   空文字になった場合でも、既存のテキストレイアウト（IDWriteTextLayout）や描画処理が破綻しないように、必要であれば空文字チェック等を行うこと（通常は空文字でもDirectWriteは正常に処理するが、念のため考慮すること）。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の最小化** : 本Hotfixは文字列の置換のみを目的とする。その他の機能追加やアーキテクチャの変更は絶対に行わないこと。
