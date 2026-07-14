### 作業指示書 REQ: Phase 8-2 Hotfix: ControlHoverHeightの所属セクション適正化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260701_0918_Phase8-2_VolumeControl.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_Phase8-2_ControlHoverHeight.md）として新規作成すること。作業レポートに原因（歴史的経緯による配置不整合の修正）と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「ControlHoverHeightのセクション移動Hotfixが完了しました。ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: DEFAULT_INI_CONTENT の整理**
    *   `src/ConfigManager_DefaultIni.h` を開き、`[Layout_Window]` セクション内にある `ControlHoverHeight` の行を削除する。
    *   削除した行を、本来の所属である `[Layout_PlaybackControls]` セクション内（適切な位置）へ移動・追記し、SSOTの整合性を保つ。
*   **タスク2: ConfigManager_Window.cpp からのパージ**
    *   `src/ConfigManager_Window.cpp` の設定読み込み処理および保存処理から、`ControlHoverHeight` に関するパース・書き込み処理を完全に削除する。
*   **タスク3: ConfigManager_Playback.cpp への統合**
    *   `src/ConfigManager_Playback.cpp` の読み書き処理（`[Layout_PlaybackControls]` セクションを処理している箇所）に、`ControlHoverHeight` のパースおよび設定保存処理を移動・実装する。
*   ※補足：`src/ConfigManager.h` におけるメンバ変数の宣言やゲッター（`GetControlHoverHeight`）、および `Window.cpp` 等での呼び出し元については変更不要。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本作業は INIファイルのセクション移動およびパース処理の移行のみに留めること。
