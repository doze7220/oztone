### 作業指示書 REQ: Hotfix: NextTrack表示機能の完全パージ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートに従い、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_NextTrackPurge.md`）として新規作成し、原因（機能の陳腐化による削除）と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、`NextTrackWidget` などの記述を完全に削除すること。
4. チャットにて「NextTrackの完全パージが完了しました。ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: Widget_NextTrackの物理削除**
    *   `src/Widget_NextTrack.h` および `src/Widget_NextTrack.cpp` を物理的に削除する。
    *   `CMakeLists.txt` のソースリストから上記2ファイルを削除する。
*   **タスク2: ConfigManagerのクリーンアップ**
    *   `src/ConfigManager.h` および実装ファイルから、`[Layout_NextTrack]` に関連するすべての変数、ゲッター、INIロード処理を完全に削除する。
    *   `[Visibility]` 設定の中にある `ShowNextTrack` のフラグ変数を削除する。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` から `[Layout_NextTrack]` セクション全体、および `ShowNextTrack` を完全に削除する。
*   **タスク3: LayoutCalculatorのクリーンアップ**
    *   `src/LayoutCalculator.h` および `src/LayoutCalculator.cpp` から `NextTrackLayout` 構造体と `CalculateNextTrackLayout` メソッドを完全に削除する。
*   **タスク4: RendererとApplicationの連携解除**
    *   `src/Renderer.h` および `src/Renderer_Setter.cpp` 等から `SetNextTrackInfo` メソッドを完全に削除する。
    *   `src/Renderer_Initialize.cpp` における `Widget_NextTrack` のインクルードと、`m_widgets` への登録処理を削除する。
    *   `src/Application_Render.cpp` （または該当ファイル）にある `m_renderer.SetNextTrackInfo(...)` の呼び出しを完全に削除する。
    *   ※注：次曲の先読み（PrefetchNextTrack）のバックグラウンド解析ロジック自体は、再生のシームレス化のために維持し、あくまで「UIへの表示連携」のみをパージすること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 本作業はNextTrackの「UI表示」に関するコードと設定の完全削除のみに留め、次曲の音声先読み処理（Prefetch）など内部の再生・解析ロジックには絶対に影響を与えないこと。
