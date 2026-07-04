# 実装計画書: Phase 10-3 プレイリストの配置（左右）オプション化

## 1. 目的
現在のOZtoneにおいて、右端固定となっているプレイリストUIを、INIファイルの設定によって「左端」または「右端」からスライドインできるように改修する。

## 2. 実装方針

### 要件1: 設定の拡張 (ConfigManager)
* `ConfigManager.h` および `ConfigManager.cpp` を修正し、`PlaylistPosition` (0: 左端, 1: 右端) の設定項目を追加する。デフォルト値は `1` とする。
* 既存の `PlaylistGripRightOffset` は左右どちらにも対応できるよう、変数名および INI ファイルのキー名を `PlaylistGripOffset` に名称変更する。INIのデフォルト値やパース部分も修正する。

### 要件2: ホバー判定の動的化 (Window)
* `Window::IsInPlaylistRegion` メソッド内にて、`m_config->GetPlaylistPosition()` の値を取得する。
* **左端 (0) の場合**: `logicalX <= hoverWidth` をX座標の判定条件とする。
* **右端 (1) の場合**: `logicalX >= logicalWidth - hoverWidth` とする（既存の実装と同様）。

### 要件3: レイアウト計算の動的化 (LayoutCalculator)
* `LayoutCalculator::CalculatePlaylistLayout` を修正し、`PlaylistPosition` に応じた座標計算を行う。
* `slideX` は `playlistWidth` (非表示) から `0` (展開) まで変動する値として扱う。
* **左端 (0) の場合**:
  * `playlistX = -slideX`
  * `gripX = playlistX + playlistWidth + gripOffset`
  * `clipRect = {0, playlistY, playlistX + playlistWidth, logicalHeight}`
* **右端 (1) の場合**:
  * `playlistX = logicalWidth - playlistWidth + slideX`
  * `gripX = playlistX - gripOffset`
  * `clipRect = {playlistX, playlistY, logicalWidth, logicalHeight}`

### 要件4: 描画の適応 (Widgets)
* `PlaylistWidget::CreateResources` において、左向き矢印(`◀`)のジオメトリに加え、新たに右向き矢印(`▶`)のジオメトリ(`m_playlistGripArrowRightGeometry`)を作成する。
* `PlaylistWidget::Draw` において、`ConfigManager::GetPlaylistPosition()` を参照し、左端設定時は右向き矢印のジオメトリを描画し、右端設定時は左向き矢印を描画する。また、グリップ線の描画位置(`layout.gripX`)も要件3の計算結果に従って正しく描画されるようにする。

---

## 3. タスクリスト
- [x] タスク1: `ConfigManager` の改修 (`PlaylistPosition` 追加と `PlaylistGripOffset` への名称変更、`DEFAULT_INI_CONTENT` の更新)
- [x] タスク2: `Window::IsInPlaylistRegion` の改修 (左右でのホバー判定分岐実装)
- [x] タスク3: `LayoutCalculator::CalculatePlaylistLayout` の改修 (矩形、クリップ、グリップ座標の計算分岐実装)
- [x] タスク4: `Widgets` (`PlaylistWidget`) の改修 (右向き矢印のジオメトリ追加および描画時の分岐実装)

## 4. 詳細作業内容
* **ConfigManagerの修正**:
  * `ConfigManager.h` および `ConfigManager.cpp` において、設定値 `PlaylistPosition` (デフォルト: 1) を追加しました。
  * `PlaylistGripRightOffset` を `PlaylistGripOffset` に名称変更し、INIファイルのパース処理とデフォルトコンテンツを更新しました。
* **Windowの修正**:
  * `Window::IsInPlaylistRegion` 内のX座標判定において、`m_config->GetPlaylistPosition()` が 0 (左端) の場合は `logicalX <= hoverWidth` を満たすように分岐処理を実装しました。
* **LayoutCalculatorの修正**:
  * `LayoutCalculator::CalculatePlaylistLayout` において、`config->GetPlaylistPosition() == 0` の場合に `playlistX`, `gripX`, `clipRect` を左端展開用の座標系で計算するように修正しました。
* **Widgetsの修正**:
  * `Widgets.h` の `PlaylistWidget` クラスに `m_playlistGripArrowRightGeometry` を追加しました。
  * `Widgets.cpp` の `CreateResources` メソッドで右向きの矢印ジオメトリを初期化しました。
  * 同ファイルの `Draw` メソッドにおいて、`GetPlaylistPosition()` の値に応じて左右いずれかの矢印ジオメトリを動的に選択して描画するように修正しました。

### [Hotfix] プレイリスト（左配置時）とアプリアイコンのホバー干渉回避
* **Windowの修正**:
  * `Window::IsInPlaylistRegion` メソッドの冒頭にて `IsInLogoRegion(x, y)` をチェックし、真の場合は即座に `false` を返す処理を追加しました。
  * これにより、左上アプリアイコンの領域がプレイリスト展開のホバートリガーから除外され、ウィンドウのドラッグ移動が阻害される不具合を解消しました。
