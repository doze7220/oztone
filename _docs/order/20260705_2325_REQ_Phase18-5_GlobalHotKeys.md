### 作業指示書 REQ: Phase 18-5: グローバルホットキーによる忍者コントロールとネオン表示機能 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-5_GlobalHotkeys.md`）として新規作成すること。
3. レポートのフォーマットは、`PROJECT_CONSTITUTION.md` の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------
#### 【実装要件】
OS全体で反応するグローバルホットキー（`RegisterHotKey`）を実装し、キーバインドをINIファイルからフルカスタマイズ可能とする。さらに、現在のキー設定を画面右上に「ネオン文字」で一覧表示する裏技的なチートシート機能を追加する。

* **要件1: INI設定とキーバインド管理 (`src/ConfigManager.h/cpp`)**
  * 新たに `[GlobalHotkeys]` セクションを追加する。
  * 以下の各アクションに対して、修飾キー（1=ALT, 2=CTRL, 4=SHIFT, 8=WIN）と仮想キーコード（VK）の組み合わせを保存・取得できるようにする。
  * アクション: `NextTrack`, `PrevTrack`, `PlayPause`, `Stop`, `VolUp5`, `VolDown5`, `VolUp25`, `VolDown25`, `PrevPlaylist`, `NextPlaylist`, `ActiveTopMost`, `ActiveBottom`, `ExitApp`
  * デフォルト値は、競合が少ない `CTRL + SHIFT + 矢印キー等` の組み合わせを設定しておく。
  * また `ShowHotkeys` (bool, デフォルトfalse) の設定値とゲッター/セッターを追加する。

* **要件2: ホットキーの登録と捕捉 (`src/Window.h/cpp`, `src/Application.h/cpp`)**
  * `Window` 側で `RegisterHotKey` を用いてキーバインドをOSに登録し、`WM_HOTKEY` を捕捉してコールバックを発火させる。
  * `Application` 側でコールバックを受け取り、再生制御、音量制御、プレイリスト切替、アプリ終了などの各アクションを実行する。

* **要件3: システムトレイへのトグルメニュー追加 (`src/Window.h/cpp`)**
  * Phase 18-3で作成したシステムトレイの「詳細設定」サブメニュー内に、メニューID `ID_TRAY_SHOW_HOTKEYS` (ホットキー表示) を追加する。
  * 選択時に `ConfigManager` の `ShowHotkeys` をトグル反転させる処理を実装する。

* **要件4: ネオン文字によるホットキー右上一覧表示 (`src/LayoutCalculator.cpp`, `src/Widgets.cpp`)**
  * `Widgets.h/cpp` に新たに `GlobalHotkeysWidget` を追加する。
  * `ShowHotkeys` が有効な場合のみ、`ConfigManager` から現在のキーバインド設定を読み取り、`CTRL + SHIFT + RIGHT : Next Track` のような文字列リストを生成する。
  * **【レイアウトと描画の絶対条件（チートシート仕様）】**: 
    * `LayoutCalculator` にて描画矩形を計算する際、起点は常に「画面右上」の固定座標とする。プレイリストの展開状態によるシフトは一切行わず、そのまま背後に隠れるストイックでハッカーライクな挙動とすること。
    * `Renderer` での描画順序は `PlaylistWidget` よりも前（背面側）とし、プレイリスト展開時にはネオン文字が半透明の黒板の奥へ沈み込むようにすること。
    * テキストフォーマットは `DWRITE_TEXT_ALIGNMENT_TRAILING` (右寄せ) を指定し、HUDのように右端に整列させること。
    * テキストは発光しているような「ネオン文字（太い半透明のグロー文字＋細い白のコア文字等）」で美しく描画すること。

#### 【初期設定ホットキー】
[GlobalHotkeys]
; チートシート(ホットキー一覧)の表示トグル
ShowHotkeys=0

; =========================================================
; Modifiers (修飾キー): 1=ALT, 2=CTRL, 4=SHIFT, 8=WIN 
; （論理和で組み合わせ可能。例: CTRL(2) + SHIFT(4) = 6, CTRL(2) + SHIFT(4) + ALT(1) = 7）
; VK: 仮想キーコード (33=PgUp, 34=PgDn, 35=End, 36=Home, 37=Left, 38=Up, 39=Right, 40=Down, 46=Del)
; =========================================================

; --- 再生コントロール (CTRL + WIN + 矢印) ---
;プレイコントロール：次の曲
Modifier_NextTrack=10
VK_NextTrack=39

;プレイコントロール：前の曲
Modifier_PrevTrack=10
VK_PrevTrack=37

;プレイコントロール：再生/一時停止
Modifier_PlayPause=10
VK_PlayPause=38

;プレイコントロール：停止
Modifier_Stop=10
VK_Stop=40

; --- 音量コントロール (CTRL + SHIFT + WIN + 矢印) ---
;音量操作：5%アップ
Modifier_VolUp5=13
VK_VolUp5=39

;音量操作：5%ダウン
Modifier_VolDown5=13
VK_VolDown5=37

;音量操作：25%アップ
Modifier_VolUp25=13
VK_VolUp25=38

;音量操作：25%ダウン
Modifier_VolDown25=13
VK_VolDown25=40

; --- プレイリスト切替 (CTRL + WIN + PageUp/PageDown) ---
;前のプレイリスト（最初のプレイリストの場合は最後のプレイリストへ）
Modifier_PrevPlaylist=10
VK_PrevPlaylist=33

;次のプレイリスト（最後のプレイリストの場合は先頭のプレイリストへ）
Modifier_NextPlaylist=10
VK_NextPlaylist=34

; --- ウィンドウアクティブ＆Z-Order制御 (CTRL + WIN + Home/End) ---
;ウィンドウ最前面固定化＆アクティブ化
Modifier_ActiveTopMost=10
VK_ActiveTopMost=36

;ウィンドウ最背面固定化＆アクティブ化
Modifier_ActiveBottom=10
VK_ActiveBottom=35

; --- アプリ終了 (CTRL + WIN + Delete) ---
Modifier_ExitApp=10
VK_ExitApp=46

#### 【作業終了後】
1. 作業レポートに詳細作業内容を記載する（タスクリストに含める）こと。
2. `PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

### 作業指示書 REQ: Phase 18-5 Hotfix: チートシートのINIパラメータ化と2カラム描画 (実装実行)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-5_GlobalHotkeys.md`）の末尾に「HOTFIX」の項目を追加し、原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------
#### 【実装要件】
現在ハードコードされているチートシート（ネオン表示）のデザインパラメータをINIからフルカスタマイズ可能にし、キー名と機能名を分離して視認性を極限まで高める「2カラム・左寄せ描画」へとアーキテクチャを改修する。

* **要件1: ConfigManager の拡張**
  * 新たに `[Layout_GlobalHotkeys]` セクションを追加する。
  * 以下のプロパティを追加し、読み書き処理とゲッターを実装する。
    * `GlowOpacity` (float, 例: 0.8f) ★新規追加：キー名カラムの左端を画面右端からどれだけ左へずらすかの距離。
    * `KeyColumnOffset` (float, 例: 400.0f) ★新規追加：キー名カラムの左端を画面右端からどれだけ左へずらすかの距離。
    * `ActionColumnOffset` (float, 例: 150.0f) ★新規追加：機能名カラムの左端を画面右端からどれだけ左へずらすかの距離。

* **要件2: 機能名取得メソッドの分離 (GlobalHotkeysWidget.cpp)**
  * `GlobalHotkeysWidget` 内（または無名名前空間）に、アクションIDを受け取って機能名（例: ": Next Track"）を返すメソッド `std::wstring GetActionName(int actionId)` を新設し、機能名文字列の変換ロジックをここに集約すること。（※ユーザーが後で日本語へ書き換えやすくするため）

* **要件3: 2カラム・左寄せ描画の実装 (GlobalHotkeysWidget.cpp)**
  * テキストの描画ロジックを改修する。文字列を結合するのではなく、「キー名文字列」と「機能名文字列」を分離して取得・保持する。
  * テキストレイアウトを生成し描画する際、以下の2カラム（表組）構成で描画すること。
    1. **キー名カラム**: 画面右端から `KeyColumnOffset` の値だけ左へ移動したX座標を「左端（起点）」とした矩形を使用し、`DWRITE_TEXT_ALIGNMENT_LEADING` (左寄せ) で描画。
    2. **機能名カラム**: 画面右端から `ActionColumnOffset` の値だけ左へ移動したX座標を「左端（起点）」とした矩形を使用し、同じく `DWRITE_TEXT_ALIGNMENT_LEADING` (左寄せ) で描画。
  * ※これにより、キー名・機能名ともに開始位置が縦一列に美しく整列した表組のレイアウトとなる。
  * `ConfigManager` から読み取ったパラメータを適用し、リソースの再生成（UpdateLayout）と描画を適切に行うこと。
