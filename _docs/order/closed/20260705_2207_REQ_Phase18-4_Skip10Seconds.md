### 作業指示書 REQ: Phase 18-4: 10秒スキップ機能の追加 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-4_Skip10Seconds.md`）として新規作成すること。
3. レポートのフォーマットは、`PROJECT_CONSTITUTION.md` の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------
#### 【実装要件】
再生コントロールに「10秒戻る / 10秒進む」ボタンを追加し、面で構成された美しいベクターアイコンとインジケーターで実装する。

* **要件1: バックエンドのシーク処理拡張 (`src/AudioPlayer.h/cpp`, `src/ConfigManager.h/cpp`)**
  * `ConfigManager` に `SkipSeconds` (デフォルト: 10.0f) の設定値とゲッター/セッターを追加する。
  * `AudioPlayer` に `Seek(float targetSeconds)` を追加する。
  * 現在位置から `±SkipSeconds` の位置へ `miniaudio` のシークAPIを用いて移動させる。0秒未満は0秒に、曲長を超える場合は曲の末尾となるよう安全なクランプ処理を行う。

* **要件2: シャープな面(Fill)によるベクターアイコンの構築 (`src/PlaybackControlsWidget.cpp`)**
  * 新しいアイコン「≪」および「≫」用の `ID2D1PathGeometry` を生成する。
  * **【デザインの絶対条件】**: 線（Stroke）で描画するのではなく、既存の「▶」と同様に **面（FillGeometry）** として多角形パスを構築すること。線の端がダサくなるのを防ぐため、斜めにカットされた上辺・底辺が「完全な水平な直線」となるような頂点座標を計算してパスを生成し、シャープで美しいシルエットを実現すること。

* **要件3: インジケーターフォントの用意と描画 (`src/PlaybackControlsWidget.cpp`)**
  * アイコンに添えるように「10」という秒数を示すインジケーターテキストを描画する。
  * ロゴメニューのインジケーターと同等の「小さくて太字、白文字＋黒フチ」のテキストフォーマット（例: `m_indicatorTextFormat`）を使用・初期化する。

* **要件4: レイアウトとクリック判定の拡張 (`src/LayoutCalculator.cpp`, `src/Window.cpp`, `src/Application.cpp`)**
  * `LayoutCalculator::CalculatePlaybackControlsLayout` にて、ボタンの数を既存の3つから5つ（`[ ⏮ ] [ ≪₁₀ ] [ ▶/⏸ ] [ ₁₀≫ ] [ ⏭ ]`）へと拡張し、それぞれの描画・当たり判定矩形を計算する。
  * `Window::GetPlaybackButtonAt` 等のクリック判定を5ボタン対応へと拡張し、Application へ通知する。
  * `Application` にて、対象ボタンが押された際に `AudioPlayer` の現在時間に対して `±SkipSeconds` の計算を行い、`AudioPlayer::Seek` を呼び出す。

#### 【作業終了後】
1. 作業レポートに詳細作業内容を記載する（タスクリストに含める）こと。
2. `PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **パスの品質**: ジオメトリの頂点計算は、適当な線ではなく、数学的に美しい平行四辺形・多角形の組み合わせになるよう丁寧に設計すること。
