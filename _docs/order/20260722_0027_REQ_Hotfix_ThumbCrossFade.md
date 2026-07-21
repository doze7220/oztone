##### 作業指示書 REQ: サムネイル遅延フェードの完全クロスフェード化 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、サムネイル遅延フェードイン時におけるプレースホルダ（ガラス板）のフェードアウト処理の実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbCrossFade.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Widget_TrackInfo.cpp`
*   **タスク1: ガラス板（プレースホルダ）のフェードアウト描画の実装**
    *  `Widget_TrackInfo::Draw` 内のドラムアイテム描画処理（`drawDrumItem` ラムダ式内等）において、ガラス板（プレースホルダ）を描画する処理を修正する。
    *  現在、ガラス板は固定の不透明度（ConfigManagerから取得した `FallbackArtOpacity` の値など）で常に描画されているが、これを画像がフェードインするのに合わせてフェードアウトするように変更する。
    *  ガラス板を描画するためのブラシ（`m_fallbackBlackBrush` 等）に対して `SetOpacity` を呼び出す際、本来の不透明度に **`(1.0f - m_thumbFadeAlpha[slotIndex])`** の係数を乗算した値を設定してから描画（`FillRectangle`）を行うこと。
    *  これにより、サムネイルが 1.0f（100%）に完全にフェードインした瞬間、背面のガラス板の不透明度が 0.0f となり、非正方形の画像であっても余白にガラス板が残らない完全なクロスフェードを実現する。

#### 【絶対遵守ルール (Constraints)】
*   **既存のフェードインロジックの維持**: サムネイル画像自体を `DrawBitmap` で描画する際の不透明度指定（`m_thumbFadeAlpha[slotIndex]` の適用）はそのまま維持すること。
