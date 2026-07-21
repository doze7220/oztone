##### 作業指示書 REQ: サムネイル遅延ロードの1秒ポーリング監視とフェードイン着地UX (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、非同期生成されたサムネイルの遅延ロードを監視し、フェードインで美しく着地させる機構の追加である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbDelayedFade.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Renderer.h`, `src/Renderer_Update.cpp`, `src/Widget_TrackInfo.h`, `src/Widget_TrackInfo.cpp`
*   **タスク1: 1秒間隔のクック監視ポーリング導入 (Renderer.h / Renderer_Update.cpp)**
    *  `Renderer.h` に監視タイマー `ULONGLONG m_lastThumbPollTime = 0;` を追加する。
    *  `Renderer_Update.cpp` の `UpdateAnimation` 内にて、`GetTickCount64()` を用いて1000ms間隔で実行されるポーリング処理を追加する。
    *  3つの `m_drumSlots` をループし、「`artBitmap == nullptr` かつ `thumbId != 0`」のスロットに対して `m_thumbnailManager->HasCookedData(thumbId)` をチェックする。
    *  クック済み（true）であれば、`m_thumbnailManager->RequestThumbnailLoad(thumbId)` を呼び出してVRAMロードを発注する。
*   **タスク2: トラックインフォへのフェード状態追加 (Widget_TrackInfo.h / .cpp)**
    *  `Widget_TrackInfo.h` に、3スロット分のフェードアルファ値を保持する `std::array<float, 3> m_thumbFadeAlpha = {0.0f, 0.0f, 0.0f};` を追加する。
    *  `Widget_TrackInfo.cpp` の `UpdateAnimation` にて、`ctx.drumSlots` を3スロットループし、`artBitmap` が `nullptr` の場合は `m_thumbFadeAlpha[i] = 0.0f;` にリセットし、有効なポインタがある場合は `deltaTime` に応じた適当な速度（例: `5.0f * deltaTime`）で `1.0f` に向かって加算（クランプ）するフェードインロジックを実装する。
*   **タスク3: ガラス板とのクロスフェード描画 (Widget_TrackInfo.cpp)**
    *  `Widget_TrackInfo.cpp` の `Draw`（`drawDrumItem` 内）にて、サムネイル画像の描画処理を改修する。
    *  常にベースとして「半透明の白のガラス板（プレースホルダ）」を描画する。
    *  画像（`slot.artBitmap`）が存在する場合、その上に `m_thumbFadeAlpha[slotIndex]` の不透明度（`SetOpacity`）でサムネイル画像をオーバーレイ（クロスフェード）描画する美しい着地を実現する。

#### 【絶対遵守ルール (Constraints)】
*   **既存のVRAMポーリング維持**: `Renderer_Update.cpp` に既に存在する「毎フレーム `GetCachedThumbnailBitmap` を叩いて VRAMキャッシュから拾う」処理はそのまま維持し、あくまで「VRAMへのロードを発注するきっかけ」として1秒ポーリングを追加すること。
