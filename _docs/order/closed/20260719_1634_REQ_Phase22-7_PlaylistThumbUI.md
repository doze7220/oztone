### 作業指示書 REQ: Phase 22-8 Hotfix: トラックインフォのサムネイル純化と自己修復の軽量化
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase22-8_TrackInfoThumbnail.md）として新規作成し、詳細作業内容を記載すること。
3. チャットにて「トラックインフォの純化と自己修復の軽量化が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在、曲切り替え時の自己修復処理（UpdateTrackMetadataIfNeeded）やドラムスロットへのデータ供給において、フル解像度のアルバムアートが不要に抽出・セットされ、パフォーマンスとVRAMを圧迫している。トラックインフォは「TrackDatabase(ODB) と ThumbnailDatabase の確認窓」に特化させるため、画像供給ラインを完全に分離・純化する。

*   **要件1: 自己修復ロジックからの「画像抽出」のパージ**
    *   対象ファイル: `src/Application_Playback.cpp` (または該当箇所)
    *   `UpdateTrackMetadataIfNeeded` 内部で TagManager を用いてタグを読み込む際、テキスト情報（曲名・アーティスト名等）のみを取得・比較し、フル解像度画像の抽出（`artBitmap`のデコードなど）が行われている場合はその処理を完全に削除（スキップ）して軽量化する。
*   **要件2: ドラムスロットへのサムネイル画像直結 (dataProviderの純化)**
    *   対象ファイル: `src/Application_Playback.cpp` (PlayCurrentTrack 等)
    *   `StartDrumAnimation` に渡す `dataProvider` ラムダ式内部において、フル解像度画像（`artBitmap`）を `DrumSlot` にセットしている処理を削除する。
    *   代わりに、対象トラックの `thumbId`（メタデータから取得、または発番）を用いて `m_thumbnailDatabase.GetCachedThumbnailBitmap(thumbId)` を呼び出し、取得したサムネイル画像を `DrumSlot.artBitmap` へ直接渡すように修正する。
    *   ※キャッシュにない場合（nullptr）は、`m_thumbnailDatabase.RequestThumbnailLoad` を呼び出してVRAMロードを要求し、サムネイルDBの仕組みに完全に乗っかること。
*   **要件3: 背景アートとの分離確認**
    *   フル解像度の画像デコードと背景へのセット（`SetBackgroundArt`）は、ドラムスロットのデータ供給からは完全に独立させ、背景描画専用の処理として維持・隔離すること。

#### 【絶対遵守ルール (Constraints)】
*   **フル解像度画像のパージ**: ドラムスロット（`DrumSlot`）には、絶対にフル解像度の画像ポインタを渡してはならない。常にサムネイル画像のみを供給すること。
