# RES:実装計画・作業レポート Phase 18-5: グローバルホットキーによる忍者コントロールとネオン表示機能

## 1. 実装目的
OS全体で反応するグローバルホットキー（`RegisterHotKey`）を実装し、キーバインドをINIファイルからフルカスタマイズ可能とする。さらに、現在のキー設定を画面右上に「ネオン文字」で一覧表示する裏技的なチートシート機能を追加する。

## 2. アーキテクチャ設計
### 要件1: INI設定とキーバインド管理 (`src/ConfigManager.h/cpp`)
    - INIファイルに `[GlobalHotkeys]` セクションを追加。
    - 各種アクションの修飾キー(Modifier)と仮想キーコード(VK)を保存・取得するメンバ変数とゲッター/セッターを実装。
    - アクション: `NextTrack`, `PrevTrack`, `PlayPause`, `Stop`, `VolUp5`, `VolDown5`, `VolUp25`, `VolDown25`, `PrevPlaylist`, `NextPlaylist`, `ActiveTopMost`, `ActiveBottom`, `ExitApp`
    - チートシート表示トグル `ShowHotkeys` (bool) の追加とゲッター/セッターの実装。

### 要件2: ホットキーの登録と捕捉 (`src/Window.h/cpp`, `src/Application.h/cpp`)
    - `Window` クラスで初期化時に `RegisterHotKey` を用いて設定されたキーバインドをOSに登録する。
    - アクションごとに一意のホットキーIDを割り当てて管理。
    - `WindowProc` で `WM_HOTKEY` メッセージを捕捉し、登録されたコールバック関数または `Application` クラスへのイベントルーティングを実装。
    - `Application` クラスで各アクションに応じたメソッド呼び出し（再生制御、音量制御、プレイリスト切り替え、最前面/背面固定、終了等）を接続。
    - 設定のホットリロード時やキーバインド変更時に古いホットキーを `UnregisterHotKey` で解除し、再登録する仕組みを構築。

### 要件3: システムトレイへのトグルメニュー追加 (`src/Window.h/cpp`)
    - `TRAY_MENU_ORDER` の「詳細設定 (Advanced)」サブメニュー内に、`ID_TRAY_SHOW_HOTKEYS` (ホットキー表示) を追加。
    - `WM_COMMAND` での同メニュー選択時に `ConfigManager` の `ShowHotkeys` をトグル反転し、設定を保存する。
    - メニュー表示時に `ShowHotkeys` の状態に合わせてチェックマークを反映する。

### 要件4: ネオン文字によるホットキー右上一覧表示 (`src/LayoutCalculator.cpp`, `src/Widgets.cpp`)
    - `src/Widgets.h/cpp` に `GlobalHotkeysWidget` クラスを新規追加し、`IWidget` インターフェースを実装。
    - `ShowHotkeys` 有効時のみ、`ConfigManager` から現在のキーバインド文字列を生成。
    - `LayoutCalculator` で画面右上を起点とし、プレイリスト展開などに影響されない絶対的な描画矩形を算出。
    - テキスト描画は `DWRITE_TEXT_ALIGNMENT_TRAILING` で右寄せに設定し、HUDのように右端へ整列。
    - `Renderer` の描画順序にて、`GlobalHotkeysWidget` が `PlaylistWidget` の手前（背面側）に描画されるよう `m_widgets` の登録順を調整し、プレイリスト展開時にはネオン文字が奥へ沈み込むようにする。
    - 描画時、光るネオン文字の表現として、太い半透明のグロー文字（ドロップシャドウ）と、細い白のコアテキストを重ねて描画する。

## 3. 実装タスクリスト
[x] タスク1: INI設定とキーバインド管理の実装 (`src/ConfigManager.h/cpp`)
    - `ConfigManager` に `[GlobalHotkeys]` のパースおよび各アクションのゲッター/セッターを追加。
[x] タスク2: ホットキーの登録と捕捉・実行の実装 (`src/Window.h/cpp`, `src/Application.h/cpp`)
    - `Window` に `RegisterHotKey`/`UnregisterHotKey` の処理と `WM_HOTKEY` 捕捉を追加。
    - `Application` で各操作を実行するロジックを連携。
[x] タスク3: システムトレイへのトグルメニュー追加 (`src/Window.h/cpp`)
    - トレイメニューの「詳細設定」に「ホットキー表示」を追加し、トグル処理とUIへの反映を実装。
[x] タスク4: ネオン文字によるホットキー右上一覧表示の実装 (`src/LayoutCalculator.cpp`, `src/Widgets.h/cpp`, `src/Renderer.cpp`)
    - `GlobalHotkeysWidget` を実装し、ネオン風のテキスト描画処理を構築。
    - `LayoutCalculator` で右上固定配置の計算追加。
    - `Renderer` のWidget登録順序を調整。
[x] タスク5: PROJECT_ARCHITECTURE.md の更新
    - グローバルホットキー、チートシート機能に関するアーキテクチャ仕様を追記。

## 4. 詳細作業内容
### タスク1: INI設定とキーバインド管理の実装
    - `ConfigManager.h/cpp` に `m_showHotkeys` および各アクションの `m_mod***`, `m_vk***` メンバ変数を追加。
    - `DEFAULT_INI_CONTENT` に `[GlobalHotkeys]` セクションと計画立案時の初期値を追記。
    - `LoadSettings` にて `GetPrivateProfileIntW` を用いて設定値を読み込む処理を追加。
    - `Set***Hotkey` および `SetShowHotkeys` にて `WritePrivateProfileStringW` でINIへ保存する処理を実装。

### タスク2: ホットキーの登録と捕捉・実行の実装
    - `Window.h` に `HotkeyID` 列挙型を追加し、`RegisterHotkeys`, `UnregisterHotkeys` メソッド、`SetHotkeyCallback` を定義。
    - `Window.cpp` にて、`RegisterHotKey` (MOD_NOREPEAT 含む) と `UnregisterHotKey` を用いたホットキーのOS登録・解除ロジックを実装。
    - `Window.cpp` の `WindowProc` で `WM_HOTKEY` メッセージを捕捉し、コールバックを発火。
    - `Application.cpp` にて `HandleMediaCommand` を分離・作成し、メディアコントロール関連の処理を整理。
    - `Application::Initialize` 内で `SetHotkeyCallback` を実装し、再生制御、音量増減、プレイリスト切り替え、Z-Order変更、アプリ終了などの各アクションに応じた処理を記述。
    - `Application::Run` ループ内の設定リロード時に、`m_window.RegisterHotkeys()` を呼び出して再登録する仕組みを追加。

### タスク3: システムトレイへのトグルメニュー追加
    - `Window.h` のID定義に `ID_TRAY_SHOW_HOTKEYS` (1071) を追加。
    - `Window.cpp` の `WM_TRAYICON` における「詳細設定 (Advanced)」サブメニュー生成部にて、`ID_TRAY_SHOW_HOTKEYS` メニューアイテムを追加。
    - メニュー生成時に `m_config->GetShowHotkeys()` に応じてチェックマーク ( `MF_CHECKED` ) を設定。
    - `WM_COMMAND` 処理にて `ID_TRAY_SHOW_HOTKEYS` が選択された際、`SetShowHotkeys` を用いてフラグを反転させるトグル処理を追加。

### タスク4: ネオン文字によるホットキー右上一覧表示の実装
    - `LayoutCalculator.h/cpp` に `CalculateGlobalHotkeysLayout` を追加し、プレイリストシフトに影響されない固定幅の描画矩形を算定。
    - `Widgets.h/cpp` に `GlobalHotkeysWidget` を新規追加。`GenerateHotkeysString` で現在のキーバインド文字列を生成。
    - `DWRITE_TEXT_ALIGNMENT_TRAILING` を用い、右寄せ描画。
    - 青緑系(`Cyan`, Alpha: 0.6)の太めのグローテキストと白(`Alpha`: 0.9)のコアテキストを重ね描画してネオン文字を再現。
    - `Renderer.cpp` の初期化時に、`GlobalHotkeysWidget` を `PlaylistWidget` より前に登録（リスト展開時に奥へ隠れる挙動）。

### タスク5: PROJECT_ARCHITECTURE.md の更新
    - `ConfigManager` の章にグローバルホットキー設定の管理についての説明を追記。
    - `Widget` コンポーネントの章に `GlobalHotkeysWidget` の挙動（ネオン文字、Zオーダー等）についての仕様を追記。

## 5. HOTFIX
### 原因
チートシート（ネオン表示）のデザインパラメータ（フォントサイズ、行間、コア色、グロー色、シャドウなど）がハードコードされており、ユーザーが好みのデザインに微調整できない状態であった。

### 対応内容
- `ConfigManager.h/cpp` に `[Layout_GlobalHotkeys]` セクションとそのプロパティ（`FontFamily`, `FontSize`, `LineSpacing`, `CoreColor`, `GlowColor`, `ShadowColor`, `ShadowOpacity`）を追加。
- ゲッター関数を実装し、INIからの値の読み取り（およびデフォルト値のセット）を構築。
- `Widgets.h` の `GlobalHotkeysWidget` クラスにドロップシャドウ用の `m_shadowBrush` を追加。
- `Widgets.cpp` の `CreateResources` および `Draw` において、`ConfigManager` から取得した値を適用するようロジックを修正（テキストレイアウトの初期化、ネオン色と影の描画設定等）。
- `PROJECT_ARCHITECTURE.md` に、チートシート機能がフルカスタマイズに対応した旨を追記。
