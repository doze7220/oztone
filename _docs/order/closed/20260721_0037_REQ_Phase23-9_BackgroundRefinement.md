##### 作業指示書 REQ: Phase 23-9: 背景データ層の純化とレイヤーグループ化
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-9_BackgroundRefinement.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
背景マネージャー移行に伴う設定の独立化と、透過機能・背景全体への不透明度（BgOpacity）適用機構を実装する。

*   **要件1: ConfigManager_Background.cpp の分離 (データ層の純化)**
    *   `src/ConfigManager_Window.cpp` に残存している `[Background]` セクションの設定パース処理・保存処理（`BgOpacity`, `BgDarkenOpacity`, `BackgroundArtMode`, `CrossfadeDuration`等）を、新設する `src/ConfigManager_Background.cpp` へと移行し、ファイル分割とカプセル化を行う。

*   **要件2: ウィンドウ透過の維持（下敷きレイヤーのパージ）**
    *   `BackgroundManager::GetLayers()` において、一番下に追加している「下敷き用の黒色カラーフィルレイヤー」の追加処理を削除する。これにより、OZtoneのアイデンティティである「デスクトップの透過（他ウィンドウへの重なり）」を阻害しないようにする。

*   **要件3: レイヤーグループ機構の定義と BgOpacity の全体適用**
    *   `BackgroundLayerType` に `LayerGroupBegin` と `LayerGroupEnd` を追加する。
    *   `BackgroundManager` のレイヤー構築ロジックにおいて、背景を構成する要素（OLD画像、NEW画像、ダークオーバーレイ）を追加する「直前」に `LayerGroupBegin` (不透明度は `BgOpacity` を適用) を追加し、すべての要素の追加が終わった「直後」に `LayerGroupEnd` を追加する。これにより、背景全体を1つのグループとしてカプセル化する。

*   **要件4: Renderer のレイヤーグループ描画対応**
    *   Renderer の初期化時に `ID2D1Layer` リソース (`m_backgroundLayer`) を作成する。
    *   `Renderer_Draw.cpp` のレイヤー描画ループにおいて、`LayerGroupBegin` が来た際は `PushLayer` を呼び出し（`layer.opacity` を適用）、`LayerGroupEnd` が来た際は `PopLayer` を呼び出す処理を実装し、不要なVRAM（オフスクリーンバッファ）を消費せずに背景全体へのアルファ適用を実現する。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-9_BackgroundRefinement.md）に、詳細作業内容を記載する（タスクリストに含める）こと。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、ConfigManagerの分割ファイル一覧などに影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **Rendererの純化維持** : Renderer 側で `m_config->GetBgOpacity()` などを直接読み取ってはならない。不透明度は司令塔（BackgroundManager）から渡された `LayerGroupBegin` の `layer.opacity` から受け取ること。
