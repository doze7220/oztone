### 作業指示書 REQ: Hotfix Halo Dust 純粋なHUEカラー生成のテスト (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_HaloDust_PureHue.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「Halo Dustの純粋HUEカラーテストの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Visualizer_HaloDust.cpp` の色生成ロジックにおいて、既存の「RGBベースの計算」や「青・緑へのカラーシフト」を一旦すべてパージ（またはコメントアウト）し、ハッシュ値から純粋なHUE（色相）を算出して完全な純色（彩度100%、明度100%）を生成するテスト実装を行う。

*   **要件1: HSV to RGB 変換ヘルパーの追加**
    *   `src/Visualizer_HaloDust.cpp` の無名名前空間などに、以下のシグネチャを持つ単純なHSV→RGB変換関数を自作して追加する。
        `D2D1::ColorF HsvToRgb(float h, float s, float v);`
    *   h は 0.0〜360.0、s と v は 0.0〜1.0 の範囲とし、戻り値として完全なRGB値を持つ `D2D1::ColorF` を返すこと。

*   **要件2: ハッシュ値からHUEへの直結とカラー決定**
    *   `DrawCircleParticle` メソッド内の色決定ロジック（`trackTitle` と `trackArtist` のハッシュ値から色を作る部分）を改修する。
    *   既存の RGB 値ごとのビットシフトや除算、および「青・緑シフト」等の色補正計算をすべてスキップする。
    *   得られたハッシュ値 (`size_t`) をもとに、`float hue = static_cast<float>(hashValue % 3600) / 10.0f;` のように 0.0〜359.9 の HUE（色相）角度を生成する。
    *   先ほど追加した `HsvToRgb(hue, 1.0f, 1.0f)` を呼び出し、彩度と明度がMAXの完全な純色を取得する。
    *   既存の「ベースカラー(#888888)との加算合成」なども今回はテストのためスキップし、取得したHSV純色をそのままパーティクルやレーザーのベースカラー（`coreColor` や `glowColor` の源泉）として適用する。

#### 【絶対遵守ルール (Constraints)】
*   **影響範囲の限定**: 本改修は `Visualizer_HaloDust.cpp` の色生成ロジックのみに限定し、物理演算や描画の本体ロジックには一切触れないこと。
