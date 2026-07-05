# RES:実装計画・作業レポート Phase 18-1: プレイリスト表示固定（ピン留め）機能

## 1. 実装目的
プレイリストを常に展開した状態に固定（ピン留め）し、他のUI要素がプレイリストに被らないよう描画領域を動的に避ける（圧縮・移動する）機能を実装する。これにより、プレイリストを常時表示したままでの快適な操作体験を提供する。

## 2. アーキテクチャ設計
### 要件1: 設定の拡張 (ConfigManager)
    - `IsPlaylistPinned` (bool) のプロパティと、関連するゲッター・セッターを追加し、`OZtone.ini` の `[General]` セクション等で保存・読み込み可能にする。
    - `[Layout_Playlist]` セクションにピン留めのサブアイコン（南京錠）を描画するためのオフセットおよびフォントサイズ設定 (`PinSubIconOffsetX`, `PinSubIconOffsetY`, `PinSubIconFontSize`) を追加する。

### 要件2: ピン留めトグルボタンの実装 (PlaylistWidget / Window / Application)
    - `PlaylistWidget` のツールバー描画処理にて、ピン留め状態に応じたトグルボタンを描画する。
      - OFF時（自動格納）: ベースアイコン「📌」（不透明度0.4f程度）の右下に、白文字＋黒フチで「🔓」（半分のサイズ）を描画。ツールチップ文字列は「表示モード: 自動格納」。
      - ON時（画面固定）: ベースアイコン「📌」（不透明度1.0fの白）の右下に、白文字＋黒フチで「🔒」（半分のサイズ）を描画。ツールチップ文字列は「表示モード: 画面固定」。
    - `Window` または `Application` のマウスイベント（プレイリスト領域クリック処理）において、ツールバーのピン留めボタンがクリックされたことを判定し、`ConfigManager` の `IsPlaylistPinned` を反転させる処理を追加する。

### 要件3: LayoutCalculator の動的領域圧縮
    - `LayoutCalculator` の各レイアウト計算関数（`CalculateTrackInfoLayout`, `CalculateSeekBarLayout`, `CalculatePlaybackControlsLayout`, `CalculateVolumeControlLayout`, `CalculateVisualizerLayout` 等）を改修する。
    - 以下の条件を満たす場合にオフセットとキャンバス幅の調整を行う。
      - 条件: `IsPlaylistPinned == true` かつ 「現在のウィンドウ幅 > 最小幅(495) + PlaylistWidth」
    - 調整内容:
      - `PlaylistPosition == 0` (左配置時): 各UIのX座標の基点を `PlaylistWidth` 分だけ右にずらし、有効キャンバス幅を `PlaylistWidth` 分狭めて計算する。
      - `PlaylistPosition == 1` (右配置時): 各UIのX座標の基点はそのままとし、有効キャンバス幅のみを `PlaylistWidth` 分狭めて計算する。

### 要件4: 排他制御の解除 (Window)
    - `Window::WindowProc` などにおけるホバー・クリックの排他制御（`m_isPlaylistHovered` による判定）において、`IsPlaylistPinned` が true の場合は排他制御をスキップし、他のUI要素（シークバーやコントロール等）を操作できるようにする。

## 3. 実装タスクリスト
[x] タスク1: ConfigManagerの実装拡張
    - `IsPlaylistPinned` プロパティの追加。
    - `PinSubIconOffsetX`, `PinSubIconOffsetY`, `PinSubIconFontSize` の追加。
    - INIファイルの読み書きと初期値設定。
[x] タスク2: ツールバーへのピン留めトグルボタン描画追加
    - `PlaylistWidget` にピン留めボタンの描画処理を追加。
[x] タスク3: ピン留めボタンのクリック操作対応
    - ツールバー上のクリック判定と状態反転のロジック追加。
[x] タスク4: LayoutCalculator の動的領域圧縮処理の追加
    - `LayoutCalculator` へ圧縮条件およびXオフセット／キャンバス幅縮小ロジックを組み込む。
[x] タスク5: 排他制御の解除
    - `Window` クラスの入力処理における排他制御条件の緩和。
[x] タスク6: PROJECT_ARCHITECTURE.md の更新
    - 追加したアーキテクチャの変更点をドキュメントへ反映。

## 4. 詳細作業内容
### タスク1: ConfigManagerの実装拡張
    - `ConfigManager.h`, `ConfigManager.cpp` に `IsPlaylistPinned` とピンアイコンのオフセット・フォントサイズ設定を追加。
    - `OZtone.ini` の `[General]` および `[Layout_Playlist]` セクションへ保存・読み込み処理を実装。
### タスク2: ツールバーへのピン留めトグルボタン描画追加
    - `PlaylistWidget.h`, `PlaylistWidget.cpp` に `m_pinSubIconFormat` を追加し、ツールバー描画処理にて「📌」とサブアイコン「🔓/🔒」を描画。
    - `WidgetContext` に `isPlaylistPinnedHovered` を追加し、ホバー時のハイライト描画とツールチップ文字列変更を実装。
### タスク3: ピン留めボタンのクリック操作対応
    - `Window.h`, `Window.cpp` に `IsPlaylistPinnedButtonAt` 関数を追加し、ピン留めボタンのヒットテストを実装。
    - `WM_LBUTTONDOWN` 時にピン留めボタンがクリックされたら `ConfigManager::SetIsPlaylistPinned` を呼び出すよう処理を追加。
### タスク4: LayoutCalculator の動的領域圧縮処理の追加
    - `LayoutCalculator.cpp` に `ApplyPinningOffset` 関数を追加。
    - `CalculateTrackInfoLayout`, `CalculateNextTrackLayout`, `CalculateSeekBarLayout`, `CalculatePlaybackControlsLayout`, `CalculateVolumeControlLayout` において、`IsPlaylistPinned` が true の場合にX座標オフセットとキャンバス幅縮小を自動適用するよう改修。
### タスク5: 排他制御の解除
    - `Window.cpp` の `WM_MOUSEMOVE` 等におけるホバー判定処理を改修。
    - `IsPlaylistPinned` が true の場合はプレイリストのホバー状態の排他制御（他のUIへの干渉無効化）を解除し、プレイリストが表示されたままでも背景のUI（シークバーなど）を通常通り操作できるように修正。
### タスク6: PROJECT_ARCHITECTURE.md の更新
    - プレイリストの「ピン留め（表示固定）」に関する仕様および、`LayoutCalculator` の領域圧縮計算の役割を `PROJECT_ARCHITECTURE.md` へ追記。

### Hotfix: ビルドエラーおよびアイコン表示不具合の修正
    - `PlaylistWidget.h` において誤って削除された `m_toolbarTextFormat` の宣言を復元し、関連するビルドエラーを修正。
    - `Window.cpp` における `CalculatePlaylistLayout` 呼び出し時の引数 (`int` から `float`) の暗黙の型変換警告 (`C4244`) に対応するため、`static_cast<float>` を明示。
    - `LayoutCalculator.cpp` の `CalculatePlaylistLayout` 関数内に、ピン留めトグルボタンの描画領域 (`pinButtonHitRect`) をツールバー右上に配置するための計算ロジックを追加し、アイコンが画面右上にずれて表示される不具合を解消。

### HOTFIX: ピン留め表示とUIアフォーダンスの修正
    - **原因と対応1 (ロゴUIの移動漏れ):** `CalculateAppLogoLayout` および `CalculateLogoMenuLayout` でプレイリストのピン留め・左側表示時の右シフト処理 (`ApplyPinningOffset`) が欠落していたため、これらに `logicalWidth` を渡しオフセットを適用するよう修正した。
    - **原因と対応2 (マウス退出時の展開維持):** プレイリスト範囲外へのマウス退出時に、ピン留めON状態でもプレイリストがスライドアニメーションで格納されてしまう不具合に対し、`PlaylistWidget::UpdateAnimation` 内での展開状態判定条件 (`isExpanded`) を `ctx.isPlaylistHovered || ctx.config->GetIsPlaylistPinned()` に変更し解決した。
    - **原因と対応3 (コンフィグの永続化):** `ConfigManager::SetIsPlaylistPinned` において、設定値 `IsPlaylistPinned` が保存・読み出しされるINIセクションが要件外の `[General]` となっていたため、`[Layout_Playlist]` に修正した。
    - **原因と対応4 (アフォーダンスの調整):** プレイリストピン留め展開時（`IsPlaylistPinned == true`）に、プレイリスト引き出しグリップの矢印が描画されたままであると操作に誤解を招くため、`PlaylistWidget::Draw` 内でピン留めON時は矢印ジオメトリの描画をスキップするよう調整した。

### HOTFIX 2: ロゴアイコンのホバー判定ズレ修正
    - **原因と対応:** 左配置でプレイリストをピン留めした際に、アプリアイコンの描画は右にオフセットされるが、ヒットテスト用の領域判定にオフセットが考慮されていなかった。`Window.cpp` の `IsInLogoRegion` メソッド内にて、マウスクリック/ホバーの判定用論理座標(`logicalX`)から `PlaylistWidth` 分をマイナスするオフセット補正を追加し解決した。なお、`IsInLogoMenuRegion` および `GetLogoMenuButtonAt` に関しては、計算に用いている `CalculateLogoMenuLayout` の内部で既にオフセットが適用された座標が返却されるため、`logicalX` をマイナス補正すると二重に補正がかかり逆に判定がずれる原因となっていたことが判明したため、これらのメソッドにはマイナス補正を行わない（あるいは撤回する）正しい状態へ修正した。
