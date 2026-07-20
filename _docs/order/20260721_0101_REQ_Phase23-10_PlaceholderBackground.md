##### 作業指示書 REQ: Phase 24-10: プレースホルダ背景の統合とフレーミング保存ロック (修正版)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反がないか確認して計画を純化すること。
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase24-10_PlaceholderBackground.md）として新規作成（出力）すること。
5. チャットにて「修正版の計画書の作成が完了しました。タスクを実行してください」と報告すること。

###### 【実装要件】
アートなし楽曲のプレースホルダ表示をクロスフェードに対応させつつ、フレーミングDBへの意図しない保存を防止する。プレースホルダ画像を「通常のWIC画像」として BackgroundManager 内部でフォールバックとして扱い、Renderer には一切の変更を加えない。司令塔（Application）は画像有無の判定と保存のロックのみを制御する。

*   **要件1: BackgroundManager へのプレースホルダWIC画像保持とフォールバック適用 (`src/BackgroundManager.h`, `src/BackgroundManager.cpp`)**
    *   `BackgroundManager` の初期化時等に、WICファクトリを用いてリソース (`IDI_PLACEHOLDER_ART`) から画像をデコードし、プレースホルダ用の `IWICBitmapSource` (例: `m_placeholderWic`) として保持する。
    *   非同期ロード等で「空パス」が渡された場合や、画像の抽出・デコードに失敗した場合は、フォールバックとして保持している `m_placeholderWic` を `NEW` 用のWIC画像としてセットし、通常と同じクロスフェード進行を開始させる。
    *   ※これにより、`Renderer` に特別なフラグやD2Dビットマップを渡す必要がなくなり、完全に既存の描画ロジックのままクロスフェードが可能になる。

*   **要件2: Applicationでの画像判定とフラグ管理 (`src/Application.h`, `src/Application_Playback.cpp`)**
    *   `Application` クラスに、現在の背景がプレースホルダであるかを判定するためのフラグ（例: `m_isCurrentBackgroundPlaceholder`）を追加する。
    *   楽曲再生時の背景設定処理において、タグから画像が取得できなかった場合は、`BackgroundManager` に対して画像なし（空パス等）を指示し、フラグを `true` に設定する。画像がある場合は `false` とする。

*   **要件3: フレーミング保存の完全ロック (`src/Application_Initialize.cpp` 等)**
    *   右ドラッグ移動、ホイール拡縮、中クリックリセットなどのフレーミング操作を受け取るコールバック処理内において、プレースホルダのフラグが `true` である場合は、操作自体を即座に弾くか、少なくとも `ArtFramingDatabase` に対する値の更新および `SaveToFile` の呼び出しを完全にスキップ（ロック）するロジックを追加する。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **Rendererの完全純化維持**: `Renderer` や `Renderer_Draw.cpp` には一切変更を加えないこと。
*   **VRAMリソースの逆流禁止**: `ID2D1Bitmap` をロジック層（ApplicationやBackgroundManager）で扱ってはならない。画像はシステムメモリ上のWICとして扱うこと。
