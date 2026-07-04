# 実装レポート：Phase 8-4: 忍者UIの極み・表示トグル機能の実装とビジュアライザモードのリファクタリング

## 実装目的
限界サイズを見極めるテストに向け、画面上の全UI要素をINIから自由にON/OFFできる表示トグル状態を構築する。
また、ストイックな思想に基づき、ビジュアライザのデフォルト状態を「非表示（None）」とし、UI要素を究極まで削ぎ落とせるようにする。

## 変更ファイル
- `src/ConfigManager.h`
  - `[Visibility]` セクションの各種UI表示フラグ（ShowAppLogo, ShowNowPlaying, ShowNextTrack, ShowSeekBar, ShowPlaybackControls, ShowVolumeControl）の保持変数とゲッターを追加。
- `src/ConfigManager.cpp`
  - `DEFAULT_INI_CONTENT` に `[Visibility]` セクションを追加し、デフォルト値をすべて1（有効）に設定。
  - `VisualizerMode` のデフォルト値を0（None）に変更。
  - `LoadSettings` にて `[Visibility]` セクションの各値を取得する処理を追加。
- `src/Window.h`
  - トレイメニューアイテムの定数に `ID_TRAY_VIS_NONE` を追加し、既存の定数値を調整。
- `src/Window.cpp`
  - `TRAY_MENU_ORDER` および `WM_TRAYICON` におけるコンテキストメニューの生成処理に「ビジュアライザ: 非表示」を追加。
  - `VisualizerMode` の値（0: None, 1: PrismBeat, 2: Halo Dust）に応じて正しくラジオボタンのチェックと設定保存が行われるよう `WM_COMMAND` の処理を修正。
- `src/Renderer.cpp`
  - `Renderer::Render` にて、各描画ブロック（アプリアイコン、現在の曲、次の曲、シークバー、再生コントロール、音量コントロール、ビジュアライザ）を、`ConfigManager` から取得した各フラグの判定文（`if`）で囲み、無効時は描画処理を完全にスキップするように修正。
- `src/Visualizer.cpp`
  - `VisualizerMode` のID変更（1: PrismBeat, 2: Halo Dust）に伴い、`Draw` 関数内の条件分岐を更新。

## 懸念点・備考
- 今回の対応によりUIが非表示状態でも、`Window` 側のクリック判定やホバー状態管理のロジックはそのまま残っているため、「透明なUI」としてマウス操作（ドラッグや再生制御、音量制御）を引き続き行うことが可能です。これは要件通りの仕様ですが、UIが見えない状態での意図しない操作が発生する可能性がある点にご留意ください。
- INIファイル内の `[Visibility]` セクションは、トレイメニューなどからの動的な切り替え機能は実装しておらず、現状は手動で編集して再起動または設定の再読み込みを行うことで反映されます。
