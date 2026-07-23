# 作業指示書 REQ: Phase 24-1 Task 11 : TrackInfo リネームと整理

*  ルール: `D:\ozlab\oztone\PROJECT_CONSTITUTION.md`
*  開発資料:`D:\ozlab\oztone\PROJECT_ARCHITECTURE.md`
*  実装計画書:`D:\ozlab\oztone\_docs\logs\20260723_2128_RES_Phase24-1_ConfigManagerPlan.md`

## 【作業手順（厳守事項）】
本プロンプトはPhase 24-1の追加タスク（TrackInfo構造体およびファイルのリネーム）である。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 実装計画書を読み、現在の移行状況を確認すること。本タスクは計画書への追加タスクとなる。
3. 以下の【追加仕様】に従って実装を開始し、ソースコードの修正およびファイルのリネームを実行すること。
4. 作業完了後、既存の作業レポート（`20260723_2128_RES_Phase24-1_ConfigManagerPlan.md`）の末尾に新しく「Task 11: TrackInfo リネームと整理」の項目を作成してチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
5. チャットにて「TrackInfoリネーム(Phase 24-1)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

## 【追加仕様】
UIウィジェット（`Widget_TrackInfo`）の名称と設定管理の名称を一致させるため、現在 `Config_LayoutNowPlaying` となっている設定構造体・ファイル名・INIセクション名を `TrackInfo` に統一する。

* **要件1: ファイルの物理リネーム**
  * `src/Config/Config_LayoutNowPlaying.h` -> `src/Config/Config_LayoutTrackInfo.h`
  * `src/Config/Config_LayoutNowPlaying.cpp` -> `src/Config/Config_LayoutTrackInfo.cpp`
* **要件2: コード内のリネーム置換**
  * 構造体名: `Config_LayoutNowPlaying` -> `Config_LayoutTrackInfo`
  * INIセクション名: `[Layout_NowPlaying]` -> `[Layout_TrackInfo]`
  * ゲッター名: `GetLayoutNowPlaying()` -> `GetLayoutTrackInfo()`
  * メソッド名: `LoadSection_LayoutNowPlaying` -> `LoadSection_LayoutTrackInfo`
* **要件3: 依存関係とデフォルトINIの更新**
  * `src/ConfigManager.h`, `src/ConfigManager.cpp` の該当箇所を更新。
  * `src/Config/ConfigManager_DefaultIni.h` 内のセクション名を変更。
  * `src/Widget_TrackInfo.cpp` や `src/LayoutCalculator.cpp` 等の呼び出し元のゲッターを新しい名前に一括置換。
  * `CMakeLists.txt` のビルド対象ファイル名を新しいものへ変更し、ビルドを通すこと。

## 【絶対遵守ルール (Constraints)】
* **スコープの厳守**: 上記のリネーム作業とそれに伴う呼び出し修正のみに留めること。
