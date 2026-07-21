##### 作業指示書 REQ: サムネイル遅延ロードのVRAM受け取り漏れとフェード描画の修正 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  参考資料: D:\ozlab\oztone\_docs\logs\20260721_2252_RES_Hotfix_ThumbDelayedFade.md
###### 【作業手順（厳守事項）】
本プロンプトは、サムネイル遅延ロード時に画像が永遠に表示されないバグ（受け取り漏れ）およびフェード描画のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbDelayedFadeFix.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Renderer_Update.cpp`, `src/Widget_TrackInfo.cpp`
*   **タスク1: VRAMからの受け取り（代入）ロジックの追加 (Renderer_Update.cpp)**
    *  `Renderer_Update.cpp` の `UpdateAnimation` 内にある1秒ポーリング処理を改修する。
    *  `m_drumSlots` をループし、`slot.artBitmap == nullptr && slot.thumbId != 0` の場合、**毎フレーム** `m_thumbnailManager->GetCachedThumbnailBitmap(slot.thumbId)` を呼び出してVRAMキャッシュを確認する。
    *  戻り値が有効（非null）であれば、その場で `slot.artBitmap = bmp.Get();` のように代入し、画像を受け取る。
    *  無効（まだVRAMにない）場合に限り、1000ms間隔のタイマー条件（`m_lastThumbPollTime`）の中で `HasCookedData` をチェックし、`RequestThumbnailLoad` で発注するロジックとする。
*   **タスク2: DrawBitmapへの正しい不透明度適用と速度調整 (Widget_TrackInfo.cpp)**
    *  `Widget_TrackInfo.cpp` の `UpdateAnimation` にて、フェード加算処理を `m_thumbFadeAlpha[i] += 2.0f * deltaTime;` に変更し、約0.5秒でフワッとフェードインするよう調整する。
    *  同ファイルの `Draw` にて、`DrawBitmap` の**第3引数 (opacity)** に対して、正しく `m_thumbFadeAlpha[slotIndex]` を渡すように修正する（誤ってブラシ等に `SetOpacity` を使っていた場合は削除する）。

#### 【絶対遵守ルール (Constraints)】
*   **既存描画構造の維持**: `Widget_TrackInfo.cpp` では、常にガラス板（プレースホルダ）を描画した上で、`slot.artBitmap` が存在するときだけその上に `DrawBitmap` でフェードオーバーレイする設計を厳守すること。
