### 作業指示書 REQ: Hotfix VolumeTooltip のベクターアイコン化と独立サイズ化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VolumeTooltipVector.md）を新規作成し、原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
Win32 APIのINIファイルにおけるUnicode絵文字の文字化け問題を根本解決するため、音量ツールチップの中身を「テキスト」から「ID2D1PathGeometryを用いたベクター図形」へと仕様変更する。
また、吹き出しの枠サイズとアイコンのサイズを独立してカスタマイズできるようINI設定を拡張する。

*   **要件1: ConfigManager の設定入れ替え**
    *   `src/ConfigManager_Playback.cpp` (および関連ヘッダやデフォルトINI定義) の `[Layout_VolumeControl]` セクションから、不要となった `TooltipText`, `TooltipFontSize`, `TooltipFontFamily`, `TooltipTextColor` 関連の変数およびゲッターを完全に削除する。
    *   `TooltipWidth`, `TooltipHeight`, `TooltipOffsetY`, `TooltipBgColor`, `TooltipBgOpacity` はそのまま維持する。
    *   新たに `TooltipIconSize` (float, デフォルト値: 24.0f) を追加し、読み書き処理とゲッターを実装する。
*   **要件2: VolumeControlWidget へのジオメトリ追加**
    *   `src/VolumeControlWidget.h` からテキストレイアウト関連の変数を削除し、代わりに内部生成ジオメトリ用の変数 `ComPtr<ID2D1PathGeometry> m_tooltipIconGeometry` を追加する。
    *   `CreateResources` 内で `ID2D1Factory::CreatePathGeometry` を用い、マウスと上下矢印を模したシンプルなベクター図形（基準サイズ 1.0f x 1.0f の正規化されたパス、または適当な固定サイズ）を構築して `m_tooltipIconGeometry` に保持する。
*   **要件3: Draw メソッドでのベクター独立描画**
    *   `Draw` メソッド内において、吹き出しの背景（面）を描画した後、テキストを描画していた処理を削除する。
    *   代わりに `m_tooltipIconGeometry` を用い、ツールチップ文字色として使っていた白色のブラシで `FillGeometry` を実行してアイコンを描画する。
    *   描画の際、`D2D1::Matrix3x2F::Scale` に `ConfigManager` から取得した `TooltipIconSize` を適用し、さらに `D2D1::Matrix3x2F::Translation` を用いて「吹き出し枠のちょうど中央」にアイコンが配置されるようにアフィン変換を合成して `SetTransform` すること。
    *   描画時の不透明度には、吹き出し背景と同様に `m_tooltipAlpha` と `ctx.controlAlpha` の乗算値を適用し、フェード効果を維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **既存アニメーションの維持**: 吹き出し自体がフワッと現れて消えるフェードアニメーションのロジックは一切壊さないこと。
