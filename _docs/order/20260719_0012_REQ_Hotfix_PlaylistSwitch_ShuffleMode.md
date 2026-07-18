##### 作業指示書 REQ: トラックインフォのCD帯（トラックナンバー）機能の削除 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\closed\20260715_2217_RES_Phase21-6_CDBandUI.md（該当機能を実装した作業レポート)
*  D:\ozlab\oztone\_docs\logs\closed\20260717_2231_RES_Hotfix_CDBandUI_BrushUp.md（該当機能を実装した作業レポート)

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_CDBandUI_Purge.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: ConfigManagerからのパラメータ完全パージ**
    *   `src/ConfigManager.h`, `src/ConfigManager_Window.cpp` (または該当の実装ファイル), `src/ConfigManager_DefaultIni.h` から `TrackCountBoxWidth`, `TrackCountBoxFontColor`, `TrackCountBoxBaseColor`, `TrackCountBoxBaseOpacity`, `TrackCountBoxUnderLineX`, `TrackCountBoxUnderLineWidth`, `TrackCountFontFamily`, `TrackCountFontSize` など、TrackInfoWidget用のトラックナンバー（CD帯）に関するすべての設定パラメータ、ゲッター、初期値、パース処理を完全に削除する。
*   **要件2: LayoutCalculatorからの計算ロジック削除**
    *   `src/LayoutCalculator.h`, `src/LayoutCalculator.cpp` の `TrackInfoLayout` 構造体および `CalculateTrackInfoLayout` メソッド内から、CD帯（トラックナンバーボックス、アンダーライン等）の矩形および座標計算を完全に削除する。
*   **要件3: Widget_TrackInfoからの描画・リソース・計算ロジックの削除**
    *   `src/Widget_TrackInfo.h`, `src/Widget_TrackInfo.cpp` から、CD帯の描画に使用していたブラシ（`m_trackCountBoxBrush`, `m_trackCountTextBrush`, `m_trackCountUnderLineBrush` 等）やテキストフォーマット（`m_trackCountTextFormat`）の生成・解放処理を削除する。
    *   `UpdateLayout` 内でのトラックナンバー文字列の生成ロジック（`ctx.shuffleIndices`から取得していた処理含む）を完全に削除する。
    *   描画ループ（`drawDrumItem` 等）における、-90度のキャンバス回転（`SetTransform`）および、CD帯の背景矩形・アンダーライン・テキストの描画処理を完全に削除する。

#### 【絶対遵守ルール (Constraints)】
*   **不要コードの完全削除**: コメントアウトで残すのではなく、不要になった変数、メソッド、計算ロジックは跡形もなく物理削除すること。
