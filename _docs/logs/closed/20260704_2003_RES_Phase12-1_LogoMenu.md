# Phase 12-1: ロゴ拡張メニュー (絵文字アイコン) とタイピング演出の実装計画

## 1. 目的と概要
本フェーズでは、左上のアプリアイコン（OZロゴ）にマウスホバーした際、右側に向かって5つの機能アイコンが滑らかにスライド展開する「拡張メニュー」を実装する。
デザイン面では、白単色化されたアイコン群、トグルOFF時の赤い斜線描画、アニメーションに同期したタイピング風のテキスト表示（`Beyond the r'ai'nbow awaits the game you envisioned in your lullaby days.`）など、リッチでクールな演出を施す。
アーキテクチャとしては、Phase 9で確立した「LayoutCalculatorでのレイアウト計算」「Widgetでのカプセル化された描画」「UpdateAnimationでの状態管理」の責務分離を厳守し、メニュー順序も配列ベースで管理可能な設計とする。

## 2. アーキテクチャ設計

### 2.1. データ構造と配列管理 (Window.h/cpp, Widgets.h/cpp)
メニュー項目の順序や内容を柔軟に変更できるよう、コマンドID、表示文字（絵文字）、トグル状態等のメタデータを保持する構造体を定義し、配列として管理する。
*   **構造体定義**: `struct LogoMenuItem { UINT commandId; std::wstring iconText; bool isToggle; bool toggleState; };`
*   **配列定義**: 現在の想定順（アプリ終了❌, 曲削除🗑️, プレイリスト固定📜, ビジュアライザ切替📽️, シャッフルダミー🔀）で初期化する。

### 2.2. INI設定管理 (ConfigManager.h/cpp)
新セクション `[Layout_LogoMenu]` を追加し、メニュー関連のすべてのレイアウト・デザインパラメータをINIファイルから制御可能にする。
*   **追加キー**: `MenuIconSize` (アイコンフォントサイズ), `MenuIconSpacing` (アイコン間の余白), `MenuScrollDuration` (展開アニメーションの完了時間), `MenuFontFamily` (絵文字等に使用するフォント), `MenuTextColor` (タイピングテキストの色), `MenuTypingFontFamily`, `MenuTypingFontSize` など。

### 2.3. レイアウト計算 (LayoutCalculator.h/cpp)
*   新構造体 `LogoMenuLayout` および要素ごとの `LogoMenuItemLayout` を追加。
*   `CalculateLogoMenuLayout` メソッドを追加し、現在の進行度（`progress`）を引数に取る。
*   ロゴの右端座標を発射位置とし、各アイコンの最終到達座標を計算。`progress` に応じて現在の描画座標を補間計算する。また、タイピングテキストの描画矩形も同時に算出する。

### 2.4. アニメーションと描画 (Widget / Renderer)
*   **新規Widget追加**: `AppLogoWidget` とは別に `LogoMenuWidget` を新規作成する（または密結合を避けるため独立させ、Renderer内でAppLogoWidgetの直後に描画する）。
*   **`UpdateAnimation`**:
    *   ホバー状態（`isLogoHovered`）と `MenuScrollDuration` を元に、進行度 `m_menuProgress` (0.0f〜1.0f) を更新。
    *   イージング関数 `easeOut` (Cubic等) を適用して、滑らかなスライドと減速を表現する。
*   **`Draw`**:
    *   白単色のシルエット描画：絵文字フォントで描画するため、D2Dのカラーフォント機能を無効化（あるいは単色ブラシでマスク）し、白単色で描画する。
    *   トグルOFF状態の表現：対象アイコンをグレーアウト（半透明等）で描画し、上から赤い斜線のジオメトリを描画する。
    *   タイピング演出：`progress` に応じて文字列 `Beyond the r'ai'nbow...` の表示文字数を計算し、部分文字列を描画する。

### 2.5. インタラクションと優先制御 (Window.h/cpp, Application.h/cpp)
*   **ホバー領域の動的拡張**: `IsInLogoRegion` に加えて、メニュー展開中は展開幅を含めた `IsInLogoMenuRegion` を判定に組み込む。
*   **排他制御の更新**:
    *   プレイリスト（左配置時）のホバー展開よりも、ロゴメニューのホバーを優先する。
    *   ロゴメニュー展開中は、背面にあるUI要素（シークバーなど）の判定をスキップする。
*   **クリック検知**: `WM_LBUTTONDOWN` 時にロゴメニュー領域内であれば、クリック座標から対応する `LogoMenuItem` を逆算して特定し、対応する処理（`WM_CLOSE`送信、フラグ反転など）を実行する。プレイリスト固定機能（📜）がONの時は、プレイリスト側のホバー縮小ロジックを無効化し、常時展開状態を強制する。

---

## 3. タスクリスト

*   [x] **タスク1: ConfigManager の拡張**
    *   `[Layout_LogoMenu]` セクションと関連設定値の getter/setter を実装する。
*   [x] **タスク2: メニュー配列とデータ構造の定義**
    *   コマンドIDやアイコン文字を管理する `LogoMenuItem` 構造体と配列を定義する。
*   [x] **タスク3: LayoutCalculator の拡張**
    *   `LogoMenuLayout` の計算ロジック（アイコンの到達位置、easeOut進行度に応じた現在位置、テキスト領域の算出）を実装する。
*   [x] **タスク4: LogoMenuWidget の実装 (1) - 基本描画**
    *   アイコンの白単色描画と、アニメーション進行度に応じたタイピングテキスト描画を実装する。
*   [x] **タスク5: LogoMenuWidget の実装 (2) - トグル状態演出**
    *   トグルOFF時の半透明化と、赤い斜線描画を追加する。
*   [x] **タスク6: アニメーション状態管理の組み込み**
    *   `Widget::UpdateAnimation` および `Renderer` 側で進行度（`m_menuProgress`）の更新とイージングを実装する。
*   [x] **タスク7: Window側 - ホバー判定領域と排他制御の更新**
    *   メニュー領域の動的ホバー判定を追加し、プレイリスト（左配置）よりも優先されるようにする。
*   [x] **タスク8: Window側 - クリックハンドリングと機能実装**
    *   各アイコンのクリック判定を実装し、アプリ終了やプレイリスト固定化等のアクションをフック・実行する。

## 4. 詳細作業内容
* **Window.h / Window.cpp**:
  * `LogoMenuItem`の初期化をコンストラクタに追加し、絵文字と各種トグル状態を設定しました。
  * `IsInLogoMenuRegion`を追加し、展開領域へのホバー維持を判定する処理を実装しました。
  * `WM_MOUSEMOVE`での排他制御にロゴ拡張メニューのフラグを含め、プレイリストより優先して反応するように修正しました。
  * `WM_LBUTTONDOWN`でのヒットテストを追加し、トグル状態の変更や関連機能（アプリ終了など）の呼び出しを実装しました。
* **LayoutCalculator.h / LayoutCalculator.cpp**:
  * `LogoMenuItemLayout` および `LogoMenuLayout` を追加し、メニュー展開時のアイコン到達位置・現在位置の計算、ならびにタイピングテキストの描画矩形計算を実装しました。
* **Widgets.h / Widgets.cpp**:
  * `LogoMenuWidget`クラスを新設し、アニメーション進行度に応じたテキスト描画や、トグルOFF時のアイコン半透明化と斜線描画を実装しました。
* **Renderer.h / Renderer.cpp / Widget.h**:
  * `WidgetContext`に `isLogoMenuHovered`と`logoMenuItems`を追加し、状態をウィジェットに受け渡せるようにしました。
  * `Application.cpp`側の呼び出しパラメータを更新し、`Window`から`Renderer`へ正確な状態が伝達されるようにしました。

* **[hotfix] レイアウトのINI設定による微調整対応**:
  * メニューアイコンの開始X座標オフセット (`MenuIconOffsetX`) およびY座標オフセット (`MenuIconOffsetY`) (いずれもロゴアイコン右上基準) を `[Layout_LogoMenu]` に追加し、`LayoutCalculator` 内での座標計算に反映させました。
  * タイピングテキストのX, Y座標オフセット (`MenuTextOffsetX`, `MenuTextOffsetY`) を追加しました。
  * タイピングテキストの文字送り間隔 (`MenuTypingLetterSpacing`) を追加し、`Widgets.cpp` の描画処理にて `IDWriteTextLayout1::SetCharacterSpacing` を用いて反映しました。
  * トグルOFF時の斜線長さをアイコン中心からの総延長 (`MenuStrikeLength`)、太さを (`MenuStrikeThickness`) として設定可能にしました。
