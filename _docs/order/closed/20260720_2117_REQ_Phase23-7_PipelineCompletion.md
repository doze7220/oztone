### 作業指示書 REQ: Phase 23-7 Hotfix : マウスによるフレーミング操作の完全復活 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
本プロンプトは背景アートのフレーミング操作（マウスドラッグ等）に関するビルドエラーの修正および機能復活のHotfixである。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従ってソースコードの修正を実行すること。
3. 作業完了後、作業レポート（D:\ozlab\oztone\_docs\logs\20260720_RES_Hotfix_FramingMouseOp.md）を新規作成し、原因と対応内容を記録すること。
4. チャットにて「フレーミング操作のHotfix実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: Rendererクラスへの ClampArtFraming の復活**
    *   `src/Renderer.h` および `src/Renderer.cpp` に、Phase 23-1の大解体で削除されていた `void ClampArtFraming(float& scale, float& offsetX, float& offsetY)` メソッドを復活・再実装すること。
    *   クランプ計算に必要な「元画像のサイズ」は、`m_currentBgBitmap`（キャッシュされたD2Dビットマップ）または `BackgroundManager` のWIC画像から動的に取得すること。
    *   画像がまだロードされていない（`nullptr` の）場合はクラッシュを避け、安全にスキップ（または何もしない）ようにフォールバック処理を入れること。
*   **要件2: Application_Initialize.cpp の再結線と残骸パージ**
    *   `src/Application_Initialize.cpp` 内に存在する `SetArtFramingMoveCallback`、`SetArtFramingScrollCallback`、`SetArtFramingResetCallback` の3箇所に残る `// [Phase23-1] Renderer層の背景アートパージに伴いコメントアウト` とそのコメントアウト部分（`//`）を完全に解除し、再結線すること。
    *   解除したコードから `m_renderer.ClampArtFraming` および `m_renderer.SetBackgroundFraming` が正しく呼び出されるようにすること。
*   **要件3: 操作のリアルタイム描画**
    *   マウスドラッグやホイールでフレーミングを変更した際、リアルタイムに画面に反映されるよう、必要に応じてコールバック内または描画ループ内で正しく描画更新が行われることを担保すること。
