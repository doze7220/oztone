### 作業指示書 REQ: Hotfix Task 1 & Task 2 : ツールチップ設定の共通化とビルド修正
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはTask 1のビルドエラー修正（Hotfix）と、Task 2のConfigManager設定追加・抽出を統合したものである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従ってソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
3. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク2」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。（Task 1のHotfixを行った旨も記載すること）
4. チャットにて「Task 1のHotfixとTask 2(ConfigManagerの拡張)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: Task 1 Hotfix (ビルドエラーの解消)**
    *   `src/Widget_VolumeControl.cpp` に `#include "WidgetCommon.h"` を追加し、`WidgetCommon::DrawMouseScrollTooltip` の名前空間が解決できずに発生しているビルドエラー（C2653, C3861）を修正する。
*   **要件2: ツールチップ共通設定の抽出 (ConfigManager)**
    *   現在 `[Layout_VolumeControl]` に混入しているツールチップのデザイン設定（`TooltipIconSize`, `TooltipBgColor`, `TooltipBgOpacity`, `TooltipWidth`, `TooltipHeight`）を、新たに設ける `[Layout_Tooltip]` セクションの管理へと抽出・移行する。
    *   `src/ConfigManager.h` および関連する cpp ファイルを修正し、`[Layout_VolumeControl]` の読み込み処理から上記を外し、共通設定として読み込めるようにする（※既存の `GetVolumeTooltip...` などのゲッター名も汎用的な名前にリネームし、呼び出し元の `VolumeControlWidget` 側もそれに合わせること）。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` を更新し、新セクションと設定を正しく配置する。
*   **要件3: 個別オフセット設定の追加 (Task 2本来の目的)**
    *   仮想スクロール時のツールチップ表示位置を制御するため、`[Layout_NowPlaying]` セクションに `TooltipOffsetX` および `TooltipOffsetY` を追加する（デフォルト値は微調整可能な値、例: `TooltipOffsetX=-10.0`, `TooltipOffsetY=-20.0` とする）。
    *   同様に、既存の `[Layout_VolumeControl]` にある `TooltipOffsetX/Y` は、VolumeControlウィジェット専用のオフセットとしてそのまま維持する。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の最小化** : ConfigManagerの拡張とVolumeControlWidgetのリファクタリング・バグ修正のみを行う。Window層でのホバー判定追加等はまだ実装しないこと。