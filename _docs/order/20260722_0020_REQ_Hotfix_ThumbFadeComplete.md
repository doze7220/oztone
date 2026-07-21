##### 作業指示書 REQ: サムネイル遅延フェードインUXの完全実装 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、サムネイル遅延ロード時の美しいフェードイン着地UXの完全実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbFadeComplete.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Renderer.h`, `src/Renderer_Update.cpp`, `src/Widget_TrackInfo.h`, `src/Widget_TrackInfo.cpp`

*   **タスク1: 1秒間隔のサムネ監視とVRAMの完全な受け取り (Renderer.h / Renderer_Update.cpp)**
    *  `Renderer.h` に監視タイマー `ULONGLONG m_lastThumbPollTime = 0;` を追加。
    *  `Renderer_Update.cpp` の `UpdateAnimation` 内で、`for (auto& slot : m_drumSlots)` と **必ず「参照渡し」でループを回すこと**（値コピーによる代入喪失を絶対防ぐこと）。
    *  `slot.artBitmap == nullptr && slot.thumbId != 0` の場合、毎フレーム `m_thumbnailManager->GetCachedThumbnailBitmap` でVRAMを確認し、あれば `slot.artBitmap` に代入。
    *  無い場合のみ、`GetTickCount64()` を用いた1000ms間隔のタイマー条件の中で `HasCookedData` をチェックし、`RequestThumbnailLoad` を発注する。

*   **タスク2: ガラス板とのクロスフェード描画 (Widget_TrackInfo.h / .cpp)**
    *  `Widget_TrackInfo.h` に `std::array<float, 3> m_thumbFadeAlpha = {0.0f, 0.0f, 0.0f};` を追加。
    *  `Widget_TrackInfo.cpp` の `UpdateAnimation` にて、画像がない場合（または `artBitmap == nullptr`）は `m_thumbFadeAlpha[i] = 0.0f;` にリセットし、ある場合は `m_thumbFadeAlpha[i] += 2.0f * deltaTime;` で 1.0f に向かって加算（約0.5秒のフワッとしたフェードイン）。
    *  同ファイルの `Draw` にて、常にガラス板（プレースホルダ）を描画した上で、画像がある場合のみその上に `context->DrawBitmap` で描画する。その際、**`DrawBitmap` の第3引数 (opacity)** に `m_thumbFadeAlpha[slotIndex]` を渡し、正しく不透明度を適用すること。

#### 【絶対遵守ルール (Constraints)】
*   **値コピーの絶対禁止**: タスク1のポーリングループでは絶対に `auto slot` とせず、必ず参照渡し (`auto& slot` またはインデックスアクセス) を用いて、本物の `m_drumSlots` のポインタが更新されるようにすること。
