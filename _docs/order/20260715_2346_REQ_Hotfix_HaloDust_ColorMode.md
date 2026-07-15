### 作業指示書 REQ: Hotfix Halo Dust カラー生成モジュール化と3モード統合 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_HaloDust_ColorMode.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「Halo Dustのカラー3モード統合が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Visualizer_HaloDust` におけるカラー決定ロジックをモジュール化し、INIファイルから「カラー固定」「RGBモード」「HSVモード」の3種類を動的に切り替えられるようにする。

*   **要件1: ConfigManager へのモードと固定色設定の追加**
    *   `src/ConfigManager.h`, `src/ConfigManager_Visualizer.cpp`, `src/ConfigManager_DefaultIni.h` を改修する。
    *   `[Visualizer_HaloDust]` セクションに `HaloDustColorMode` (整数型) と `HaloDustFixedColor` (文字列型) を追加する。
    *   デフォルト値は `HaloDustColorMode=2` (HSVモードを初期値とする)、`HaloDustFixedColor="#00FFFF"` 等とし、INIの読み書きとゲッター (`GetHaloDustColorMode`, `GetHaloDustFixedColor`) を実装する。

*   **要件2: カラー生成関数のモジュール化と HsvToRgb の新設**
    *   `src/Visualizer_HaloDust.cpp` の無名名前空間等に、HSVをRGBへ変換するヘルパー関数 `D2D1::ColorF HsvToRgb(float h, float s, float v)` を実装する。
    *   同じく無名名前空間等に、ハッシュ値から色を生成する以下の2つの関数を定義する。
        *   `GenerateColorRGB(size_t hashValue)` : 既存の「R, G, B成分の抽出および青・緑シフト補正計算」をそのまま移植し、`D2D1::ColorF` として返す。
        *   `GenerateColorHSV(size_t hashValue)` : ハッシュ値から 0.0〜359.9 の HUE（色相）角度を生成し、`HsvToRgb(hue, 1.0f, 1.0f)` を呼び出して彩度・明度MAXの純色を `D2D1::ColorF` として返す。

*   **要件3: 描画ロジックにおける3モード動的切り替え**
    *   `src/Visualizer_HaloDust.cpp` の `DrawCircleParticle` 内の色決定部分において、`m_config->GetHaloDustColorMode()` の値に応じて以下の分岐を行う。
        *   **モード 0 (固定)**: `WidgetCommon::HexToColorF(m_config->GetHaloDustFixedColor())` を呼び出し、INIで指定された絶対的な固定色を適用する。
        *   **モード 1 (RGB)**: `GenerateColorRGB(hashValue)` を呼び出す。
        *   **モード 2 (HSV)**: `GenerateColorHSV(hashValue)` を呼び出す。
    *   決定したベースカラーを、既存のパーティクルやレーザーの描画処理（`coreColor` / `glowColor`）へ渡す。

#### 【絶対遵守ルール (Constraints)】
*   **ファイル分割アーキテクチャの厳守**: `ConfigManager` の修正は必ず物理分割された `ConfigManager_Visualizer.cpp` および関連ヘッダにて行うこと。
*   **ユーティリティの活用**: モード0の固定色変換には、必ず既存の `WidgetCommon::HexToColorF` ユーティリティを使用すること。
