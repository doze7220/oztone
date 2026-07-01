【目的】
Phase 8-4: 忍者UIの極み・表示トグル機能の実装とビジュアライザモードのリファクタリングを行います。
限界サイズを見極めるテストに向け、画面上の全UI要素をINIから自由にON/OFFできる状態を構築します。

【実装要件】
1. ビジュアライザモードのリファクタリング
対象: src/ConfigManager.h, src/ConfigManager.cpp, src/Window.h, src/Window.cpp, src/Renderer.cpp
- ConfigManager の VisualizerMode を以下のように再定義してください。
  - 0: None（非表示）
  - 1: PrismBeat（直線型オシロスコープ）
  - 2: Halo Dust（円形パーティクル・スペアナ）
- デフォルト値はストイックに「0 (None)」から始まるように変更してください。
- Window.h / Window.cpp にトレイメニューアイテム `ID_TRAY_VIS_NONE` を追加し、メニュー構築と `WM_COMMAND` でのチェック・保存処理を修正してください。
- Renderer::Render にて、取得した VisualizerMode が `0` 以外の場合のみ `m_visualizer.Draw()` を呼び出し、`0` の場合は描画を完全にスキップ（負荷ゼロ）するようにしてください。

2. 各種UIの表示トグル機能の追加 (ConfigManager)
対象: src/ConfigManager.h, src/ConfigManager.cpp
- OZtone.ini に新しく `[Visibility]` セクションを追加し、以下の bool 値フラグ（デフォルトはすべて true または 1）の読み書きとゲッターを実装してください。
  - ShowAppLogo (左上アプリアイコン)
  - ShowNowPlaying (左下現在の曲)
  - ShowNextTrack (右下次の曲)
  - ShowSeekBar (シークバー)
  - ShowPlaybackControls (再生コントロール)
  - ShowVolumeControl (音量コントロール)

3. Rendererの描画スキップ対応 (Renderer)
対象: src/Renderer.cpp
- Renderer::Render 内の各描画ブロック（対応するドロップシャドウの描画やオフセット計算を含める）を、ConfigManager から取得した表示フラグの `if` 文で囲んでください。
- フラグが `false` の場合は、描画APIを一切呼び出さずスキップするようにしてください。
- ※今回はWindowクラス側のホバー判定や操作フック（再生や音量変更）はそのまま残し、「透明なUI」として機能させたままで問題ありません。

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点と懸念点をまとめた実装レポートを出力してください。
