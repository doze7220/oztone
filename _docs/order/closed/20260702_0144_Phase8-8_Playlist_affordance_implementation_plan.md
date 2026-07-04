# Phase 8-8 ステップ3: プレイリストのアフォーダンス向上

プレイリスト展開用の「引き出しグリップ（縦線と◀アイコン）」をウィンドウ右端に描画し、直感的なUIへの誘導を実装します。レイアウトとカラーは全てINIファイル（ConfigManager）で管理します。

## ユーザーレビュー事項
要件にはグリップの基準X座標の算出式が `gripX = m_playlistSlideX - PlaylistGripRightOffset` と指定されていましたが、実際の描画におけるプレイリストの左端X座標（`playlistX`）は `renderTargetSize.width - playlistWidth + m_playlistSlideX` となっております。
そのため、画面上の絶対座標としては `gripX = playlistX - PlaylistGripRightOffset` とするのが正しいと判断し、そのように実装する計画としています。
もし `gripX` の基準が異なる場合はご指摘ください。

## 提案する変更

### ConfigManager
設定の読み書き、ゲッター、および初期値を追加します。

#### [MODIFY] src/ConfigManager.h
- `[Layout_Playlist]` 関連のメンバ変数として以下を追加：
  - `float m_playlistGripRightOffset;`
  - `float m_playlistGripLineWidth;`
  - `std::wstring m_playlistGripLineColor;`
  - `float m_playlistGripArrowHeight;`
  - `float m_playlistGripArrowWidth;`
  - `std::wstring m_playlistGripArrowColor;`
  - `float m_playlistGripShadowOffsetX;`
  - `float m_playlistGripShadowOffsetY;`
  - `float m_playlistGripShadowOpacity;`
- 対応するゲッターメソッドを追加。

#### [MODIFY] src/ConfigManager.cpp
- コンストラクタで上記変数の初期値を設定。
- `LoadSettings` メソッド内で INI ファイルから値を読み込む処理を追加。`DEFAULT_INI_CONTENT` にもデフォルト値を記載。

---

### Renderer
グリップの初期化と描画処理を追加します。

#### [MODIFY] src/Renderer.h
- 新たなメンバ変数を追加：
  - `Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistGripLineBrush;`
  - `Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_playlistGripArrowBrush;`
  - `Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_playlistGripArrowGeometry;`

#### [MODIFY] src/Renderer.cpp
- **Initialize() メソッド内:**
  - `ConfigManager` から取得した `PlaylistGripLineColor` および `PlaylistGripArrowColor` を元に、専用ブラシを生成。
  - `PlaylistGripArrowWidth` と `PlaylistGripArrowHeight` を用いて左向きの三角形のパスを作成し、`m_playlistGripArrowGeometry` に格納。

- **Render() メソッド内:**
  - プレイリストが閉じている状態でも右端から引き出しが見えるよう、グリップ描画部分は常に描画されるようにする。
  - `playlistX = renderTargetSize.width - playlistWidth + m_playlistSlideX;` の計算の後、`gripX = playlistX - PlaylistGripRightOffset;` とする。
  - プレイリスト背景板の描画付近で以下を行う：
    - **影描画:** 本体の描画位置にオフセットを足した座標に、`PlaylistGripShadowOpacity` の透明度を設定した黒ブラシで縦線と矢印の影を描画。縦線は画面上端から下端まで。
    - **縦線本体:** `(gripX, 0)` から `(gripX, logicalHeight)` まで専用ブラシで描画。
    - **矢印本体:** `logicalHeight / 2.0f` 付近に `TranslateTransform` を用いて矢印ジオメトリを配置し、`FillGeometry` する。

## 確認計画
1. `ConfigManager` に追加した項目が `OZtone.ini` に正しく出力され反映されるか。
2. プレイリストが閉じている状態でも右端にグリップが見えるか。
3. マウスをホバーするとグリップがプレイリストと共にスライドしてくるか。
4. シャドウ、線の太さ、色などが設定通りに反映されるか。
