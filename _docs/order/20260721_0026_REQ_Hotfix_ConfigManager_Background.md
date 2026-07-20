### 作業指示書 REQ: Phase 23-8 Hotfix : ConfigManager_Background.cpp の分離 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
本プロンプトは、INI設定の `[Background]` セクションパース処理を独立させるHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ConfigManager_Background.md）として新規作成し、原因と対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、ConfigManager の分割ファイル一覧などに影響があるため資料の修正を行うこと。
5. チャットにて「ConfigManagerの背景設定分離が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`ConfigManager_Window.cpp` 内に存在する `[Background]` セクションの処理を分離し、データ層の責務を完全に独立させる。

*   **要件1: `ConfigManager_Background.cpp` の新設と処理移行**
    *   `src/ConfigManager_Background.cpp` を新規作成する。
    *   `src/ConfigManager_Window.cpp` から、`BgOpacity`, `BgDarkenOpacity`, `BackgroundArtMode`, `CrossfadeDuration` の設定パース、オートフィル（デフォルト値の書き込み）、および保存ロジックを切り離し、新設したファイルへ移行・カプセル化する。
    *   `src/ConfigManager.h` に `void LoadBackgroundSettings();` 等の委譲用メソッドのプロトタイプを追加し、`src/ConfigManager.cpp` の司令塔（`LoadSettings` など）から呼び出すように配線する。

*   **要件2: ビルド設定の更新**
    *   `CMakeLists.txt` のソースリストに `src/ConfigManager_Background.cpp` を追加し、ビルドが通ることを確認する。

*   **要件3: ドキュメントの更新**
    *   `PROJECT_ARCHITECTURE.md` の `ConfigManager` クラスの物理分割ファイル一覧に `ConfigManager_Background.cpp` （背景アート・クロスフェード設定用）を追記し、責務分離が完了したことを明記する。

#### 【絶対遵守ルール (Constraints)】
*   **既存ゲッターの維持**: `GetBgOpacity()` などのゲッターメソッド名や振る舞いは一切変更しないこと。外部（BackgroundManager等）からの呼び出しに影響を与えず、あくまで内部のパースロジックの物理ファイルの移動のみを行う。
