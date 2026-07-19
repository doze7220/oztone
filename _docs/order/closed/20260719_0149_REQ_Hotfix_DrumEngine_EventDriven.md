##### 作業指示書 REQ: 背景アートとドラムアニメーションの連動パージおよび独立描画への復元 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_0116_RES_Hotfix_DrumEngine_EventDriven.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_BackgroundDecoupling.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: Application層での背景画像即時ロードの復元**
    *   `src/Application_Playback.cpp` 等の `PlayCurrentTrack` 内において、ドラムアニメーションの完了時 (`onComplete`) を待たずに、ジャンプ開始（曲決定）と **同時** に背景用のアルバムアート画像やフレーミング情報を読み込む旧来の即時ロード処理を復元する。
*   **要件2: Rendererへの背景アート専用インターフェースの復元・分離**
    *   `Renderer` 側に、ドラムスロット (`DrumSlot`) とは完全に独立して背景画像を保持するための変数（`m_backgroundArtBitmap` 等）と、それを受け取るためのメソッド（`SetBackgroundArt` 等、旧来 `SetAlbumArt` として存在した機能に相当）を用意し、要件1で取得した画像を即座にセットさせる。
*   **要件3: 背景とドラムの同期ロジックの完全パージ**
    *   `src/Renderer_Draw.cpp` の `DrawBackground` 等において、Phase 21-8で実装された「ドラムの相対距離 (`m_drumRelativePosition`) やフリップサイクルに連動して背景アートをスライド・フェードさせる同期ロジック」を **完全に削除（パージ）** する。
    *   背景描画は、ドラムの状態を一切参照せず、現在保持している背景画像を単独で描画する純粋な形へと復元する。

#### 【絶対遵守ルール (Constraints)】
*   **新クラス作成の禁止**: 本タスクでは `BackgroundManager` などの新しいクラスやファイルは作成しないこと。既存の `Renderer` の枠組みの中で、ドラムロジックと背景ロジックの依存（連動）を絶ち切ることにのみ専念する。

