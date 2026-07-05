# RES:実装計画・作業レポート Phase 18-4: 10秒スキップ機能の追加

## 1. 実装目的
再生コントロール領域に「10秒戻る / 10秒進む」ボタンを追加し、ユーザーが素早く楽曲の特定位置へシークできる機能を提供する。UIは面(Fill)で構成された美しいベクターアイコンとインジケーターテキストで実装し、シーク処理の安全なクランプ制御も備える。

## 2. アーキテクチャ設計
### 要件1: バックエンドのシーク処理拡張 (`src/AudioPlayer.h/cpp`, `src/ConfigManager.h/cpp`)
    - `ConfigManager` に `SkipSeconds` (デフォルト: 10.0f) を設定値として追加し、ゲッターおよびセッターを実装する。
    - `AudioPlayer` クラスに `Seek(float targetSeconds)` メソッドを追加し、内部で miniaudio のシークAPIを呼び出す。0秒未満は0秒、曲長を超える場合は末尾へと安全なクランプ処理を行う。

### 要件2: シャープな面(Fill)によるベクターアイコンの構築 (`src/PlaybackControlsWidget.cpp`)
    - 新規アイコン「≪ (10秒戻る)」および「≫ (10秒進む)」用の `ID2D1PathGeometry` を生成する。
    - アイコンは線(Stroke)ではなく面(FillGeometry)による多角形パスとして構築し、上辺・底辺が完全な水平線となるよう精密な座標計算を行うことでシャープなシルエットを実現する。

### 要件3: インジケーターフォントの用意と描画 (`src/PlaybackControlsWidget.cpp`)
    - アイコンに添える「10」という秒数を示すインジケーターテキストを描画する。
    - ロゴメニューと同等の「小さくて太字、白文字＋黒フチ」のテキストフォーマット（例: `m_indicatorTextFormat`）を使用・初期化する。

### 要件4: レイアウトとクリック判定の拡張 (`src/LayoutCalculator.cpp`, `src/Window.cpp`, `src/Application.cpp`)
    - `LayoutCalculator::CalculatePlaybackControlsLayout` のボタン数を既存の3つから5つ（`[⏮] [≪₁₀] [▶/⏸] [₁₀≫] [⏭]`）に拡張し、各ボタンの描画・当たり判定矩形を再計算する。
    - `Window::GetPlaybackButtonAt` 等のクリック判定を5ボタン仕様へ拡張する。
    - `Application` のクリックイベント処理にて対象ボタンのインデックスを判別し、`AudioPlayer` の現在時間に対して `±SkipSeconds` の計算後、`AudioPlayer::Seek` を呼び出す。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManagerとAudioPlayerのバックエンド拡張
    - `ConfigManager` に `SkipSeconds` の読み書き処理を追加。
    - `AudioPlayer` にシーク用 `Seek(float)` メソッドとクランプ処理を追加。
- [x] タスク2: 再生コントロールのレイアウトおよびクリック判定の拡張
    - `LayoutCalculator::CalculatePlaybackControlsLayout` で5ボタンのレイアウトを算出するよう変更。
    - `Window` と `Application` のマウスクリックハンドラおよびヒットテストロジックを5ボタン対応に更新。
- [x] タスク3: PlaybackControlsWidgetのアイコン・インジケーター描画の実装
    - 面(Fill)による「≪」「≫」アイコンの `ID2D1PathGeometry` 生成。
    - インジケーター用テキストフォーマット（10秒）の作成と描画処理の実装。
- [x] タスク4: ドキュメントの更新
    - 完了後、`PROJECT_ARCHITECTURE.md` に追加機能の情報を追記・更新。

## 4. 詳細作業内容
### タスク1: ConfigManagerとAudioPlayerのバックエンド拡張
    - `ConfigManager` に `m_skipSeconds` メンバ変数を追加し、ゲッター・セッターを実装。INIからのロードおよびデフォルト値(10.0f)の初期化を行いました。
    - `AudioPlayer::Seek(float)` を実装し、0秒未満や曲の末尾を超えないようにクランプ処理を追加。`ma_sound_seek_to_pcm_frame` を呼び出して安全にシークできるように実装しました。

### タスク2: 再生コントロールのレイアウトおよびクリック判定の拡張
    - `Window::GetPlaybackButtonAt` にて5つのボタン（Previous, Skip Back, Play/Pause, Skip Forward, Next）のクリック判定座標を修正しました。
    - `Window.h` に `SetSkipCommandCallback` を追加し、`Application::Initialize` 側で10秒スキップ時の AudioPlayer::Seek をコールバック実行する処理を記述しました。

### タスク3: PlaybackControlsWidgetのアイコン・インジケーター描画の実装
    - `PlaybackControlsWidget.cpp` 内で `m_chevronLeftGeometry` および `m_chevronRightGeometry` を新規作成。シャープなエッジを持つポリゴンで「≪」と「≫」のパーツを描画する処理（DrawChevron）を実装しました。
    - `m_indicatorTextLayout` を使って「10」のテキストに黒いフチ（シャドウ）を付与し、アイコンに隣接させて描画しました。

### タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の概要・実装済みクラスリファレンス等の説明に、10秒スキップボタン・`SetSkipSeconds` 等の仕様を追記しました。

### Hotfix: 10秒スキップアイコンのカスタマイズ対応
    - ユーザー要望により、`ConfigManager` に以下の設定パラメータを新たに追加し、INIファイル(`[Layout_PlaybackControls]`)に出力・読み込みを行うように対応しました。
      - `SkipIconPoints` (≪のベクター座標。デフォルト値: "0.25,-0.5,0.5,-0.5,0.0,0.0,0.5,0.5,0.25,0.5,-0.25,0.0")
      - `SkipTextFontSize` (フォントサイズ。デフォルト値: 10.0)
      - `SkipTextOffsetX` (テキストXオフセット。デフォルト値: 0.2)
      - `SkipTextOffsetY` (テキストYオフセット。デフォルト値: 0.1)
      - `SkipTextShadowColor` (影の色。デフォルト値: "#000000")
      - `SkipTextShadowOpacity` (影の透明度。デフォルト値: 0.8)
      - `SkipTextShadowShift` (影のずらしpx。デフォルト値: 1.0)
    - `PlaybackControlsWidget.cpp` 内の固定値を上記設定値を参照するよう修正し、テキストの影描画時にはコントロール自体の透明度(`ctx.controlAlpha`)と設定の透明度を乗算してフェードするよう改善しました。

### Hotfix 補足: 図形（アイコンおよびインジケーター）の描画ロジック詳細
    - **ベクターアイコンの構築**: INIから取得した `SkipIconPoints` 文字列（カンマ区切り）を `float` 配列にパースし、`ID2D1PathGeometry` を用いて動的に構築しています。`BeginFigure` で開始点（最初のペア）を打ち、以降のペアを `AddLine` で結んで最後に `EndFigure(D2D1_FIGURE_END_CLOSED)` とすることで、塗りつぶし可能な1つの面（ポリゴン）を形成しています（右向き用にはX座標を反転させて流用）。
    - **縁取り（黒フチ）テキストの疑似描画**: `IDWriteTextLayout` にはアウトライン専用の機能がないため、描画時にオフセット位置（左上、左下、右上、右下など斜め4方向）に `SkipTextShadowShift` の分だけずらして `SkipTextShadowColor` と `SkipTextShadowOpacity` を用いてテキストを4回（影として）描画し、最後に本来の位置へ白文字を描画することで、視認性の高いフチ付きテキストを疑似的に表現しています。
