# RES:実装計画・作業レポート Phase 23-8: 背景マネージャーの純化

## 1. 実装目的
背景描画機構を「4層レイヤー構造」および「NEWスワップ式のクロスフェード」へと進化させ、Rendererから背景アートの位置や拡大率などの状態、および設定の依存を完全に排除し、完全な受動態（ペインター）へと純化させる。

## 2. アーキテクチャ設計
### 要件1: INI設定の `[Background]` セクションへの昇格と拡張
- `[Layout_Window]` に存在した `BgOpacity`, `BgDarkenOpacity`, `BackgroundArtMode` を新規の `[Background]` セクションへ移動する。
- クロスフェードの速度を制御するためのパラメータ `CrossfadeDuration` (または `CrossfadeSpeed`) を `[Background]` セクションに新規追加する。
- 既存のゲッターメソッド名は維持しつつ、パース先とセーブ先を変更する。

### 要件2: `BackgroundManager` のレイヤー構造定義
- 描画指示書となる `BackgroundLayer` 構造体（Type、Bitmap、X、Y、Scale、Color、Opacity等）を定義する。
- 動的に構築されるレイヤーリスト（1.下敷き、2.OLD画像、3.NEW画像、4.ポストエフェクト[カラーフィル]）を返却する `GetLayers()` を提供する。

### 要件3: スワップ式クロスフェードの実装
- 追加した速度制御設定を用いてフェード進行速度を制御する。
- OLDとNEWそれぞれに独立したWIC画像とフレーミング情報を保持させ、フェード中のオフセットのズレを防ぐ。
- NEWの不透明度が1.0fに達した時点で、NEWの画像・フレーミング情報をOLDにスワップ（上書き）し、NEWを空にして完了させる。

### 要件4: Renderer からの背景状態およびINI参照の完全パージ
- Rendererの `m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale` などを全削除する。
- フレーミング変更の操作先をRendererからBackgroundManagerへ繋ぎ変える。
- `DrawBackground` メソッド内の独自の描画条件分岐やINI設定直接呼び出しを削除し、受け取ったレイヤーの配列を順番に描画する処理に純化させる。

## 3. 実装タスクリスト
[x] タスク1: INI設定の `[Background]` セクション昇格と新規パラメータ追加
    - ConfigManagerにおけるパース/保存処理の更新、DefaultIniの書き換え。
[x] タスク2: `BackgroundLayer` 構造体の定義と `BackgroundManager` レイヤー構築ロジック追加
    - BackgroundLayerの定義と、`GetLayers()` による4層構築ロジックの実装。
[x] タスク3: スワップ式クロスフェードのロジック実装とフレーミング管理の移行
    - UpdateAnimationの改修、OLD/NEW独立のフレーミング保持、1.0f到達時のスワップ処理の実装。
[x] タスク4: `Renderer` からの背景状態・INI参照の完全パージとレイヤー描画処理の実装
    - Renderer内部のフレーミング変数の削除、DrawBackgroundのレイヤー反復描画化。マウスコールバックからの通知先をBackgroundManagerへ変更。
[x] タスク5: アーキテクチャドキュメントの更新
    - PROJECT_ARCHITECTURE.md への反映確認と更新。

## 4. 詳細作業内容
### タスク1: INI設定の `[Background]` セクション昇格と新規パラメータ追加
    **【対象ファイル】**
    - `src/ConfigManager_DefaultIni.h`
    - `src/ConfigManager.h`
    - `src/ConfigManager_Window.cpp`
    **【作業内容】**
    - `ConfigManager_DefaultIni.h`のデフォルト設定において、既存の`[Layout_Window]`セクションにあった背景用パラメータを新規の`[Background]`セクションへ昇格し、`CrossfadeDuration=0.5`を追加。
    - `ConfigManager.h`に、`m_crossfadeDuration`のメンバ変数およびgetter/setter定義を追加。
    - `ConfigManager_Window.cpp`におけるINI読み書き処理において、対象セクションを`Layout_Window`から`Background`へ変更し、新たに`CrossfadeDuration`のパース処理を実装。

### タスク2: `BackgroundLayer` 構造体の定義と `BackgroundManager` レイヤー構築ロジック追加
    **【対象ファイル】**
    - `src/BackgroundManager.h`
    - `src/BackgroundManager.cpp`
    - `src/Application_Initialize.cpp`
    **【作業内容】**
    - `BackgroundManager.h` にて、描画レイヤーの種類を示す `BackgroundLayerType` 列挙型と、画像・不透明度・カラー・座標・拡大率を保持する `BackgroundLayer` 構造体を定義（Doxygenスタイルのコメントを付与）。
    - `BackgroundManager` が `ConfigManager` の設定を参照できるように、`Initialize` メソッドの引数に `const ConfigManager*` を追加。
    - `Application_Initialize.cpp` にて、`BackgroundManager::Initialize` の呼び出し時に `&m_config` を渡すよう修正。
    - `BackgroundManager.cpp` にて `GetLayers()` メソッドを実装。現在の `BackgroundArtMode` と `BgDarkenOpacity` の設定、および `m_fadeProgress` に基づき、下敷き（Black）、OLD画像（Opacity: 1.0f）、NEW画像（Opacity: fadeProgress）、ポストエフェクト（Black、Opacity: BgDarkenOpacity）の最大4層からなるレイヤーリストを動的構築して返却するロジックを追加。

### タスク3: スワップ式クロスフェードのロジック実装とフレーミング管理の移行
    **【対象ファイル】**
    - `src/BackgroundManager.h`
    - `src/BackgroundManager.cpp`
    **【作業内容】**
    - `BackgroundManager.h` に OLD用およびNEW用のフレーミング状態を保持するメンバ変数 (`m_oldScale`, `m_oldOffsetX`, `m_oldOffsetY`, `m_currentScale`, `m_currentOffsetX`, `m_currentOffsetY`) を追加。
    - フレーミング状態を外部から更新するためのセッターメソッド (`SetArtFramingScale`, `SetArtFramingScroll`) を追加。
    - `UpdateAnimation` メソッドを改修し、新規画像が設定された瞬間に現在のNEW画像・フレーミング情報をOLDへスワップし、NEW側のフレーミングを初期値にリセットする処理を実装。
    - `ConfigManager` から `CrossfadeDuration` を取得し、`m_fadeProgress` を進行させるロジックを実装。1.0f到達時にOLD画像およびOLDのフレーミング状態を解放・リセット。
    - `GetLayers` メソッドにて、構築する各レイヤーに対して個別のフレーミング状態 (`x`, `y`, `scale`) を適用するように修正。

### タスク4: `Renderer` からの背景状態・INI参照の完全パージとレイヤー描画処理の実装
    **【対象ファイル】**
    - `src/Renderer.h`
    - `src/Renderer.cpp`
    - `src/Renderer_Draw.cpp`
    - `src/Application_Initialize.cpp`
    **【作業内容】**
    - `Renderer.h` から `m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale` などのフレーミング用メンバ変数、および不要になったメソッド（`SetBackgroundScale`, `SetBackgroundOffset`, `ClampArtFraming` 等）を完全に削除。
    - `Renderer.cpp` の各種処理から、上記変数へのアクセスや初期化処理を削除。
    - `Renderer_Draw.cpp` の `DrawBackground` メソッドを改修。独自の設定（BackgroundArtMode等）参照や分岐を廃止し、`BackgroundManager::GetLayers()` が返す `BackgroundLayer` の配列を受け取って、その指定通りに反復描画するロジックへ純化。
    - `Application_Initialize.cpp` 内のマウス等によるフレーミング操作コールバック（`SetArtFramingZoomCallback`, `SetArtFramingScrollCallback`）の接続先を `Renderer` から `BackgroundManager` へ変更。

### タスク5: アーキテクチャドキュメントの更新
    **【対象ファイル】**
    - `PROJECT_ARCHITECTURE.md`
    **【作業内容】**
    - `BackgroundManager` の項目へ、「4層レイヤー構造の採用」「スワップ式クロスフェード機能の実装」、および「Rendererから背景状態管理を完全に引き継いだ空間の司令塔としての役割」を追記し、最新のアーキテクチャ定義へ更新する。

### HOTFIX1
#### 原因・理由: スワップ式フェードのステートマシン完全修復
    - `UpdateAnimation` における旧来の誤った画像スワップ処理（新規画像セット時にOLDを即時上書きし、NEWのフレーミングをリセットしてしまう等）および、1.0f到達時の単なるOLD解放処理に起因する、NEWフレーミングの意図しない初期化とスワップタイミングの誤り。

#### 対象ファイル: 
    - `src/BackgroundManager.cpp`

#### 対応: 画像セットおよびアニメーション更新ロジックを正しい「スワップ式ステートマシン」へ改修
    - 新規画像セット時はNEW画像にセットするのみとし、OLDへの即時スワップやフレーミング情報の初期値リセットを完全に禁止（削除）した。
    - `m_fadeProgress` が 1.0f に到達した瞬間に、NEWの画像およびフレーミング情報（Scale, OffsetX, OffsetY）をOLDへコピー（上書き格上げ）し、NEW側を解放・空状態にするよう改修した。
    - さらに、フェード完了後（`!m_currentWicImage` 時）に Application 側からのフレーミング更新（`SetArtFramingScale`, `SetArtFramingScroll`）が正しく OLD（アクティブレイヤー）へ適用されるよう、セッターのロジックを修正した。

### HOTFIX2
#### 原因・理由: 背景フレーミング変数のステート分離と操作ロック
    - OLDとNEWのフレーミング変数が同時に上書きされ、フェード中のOLD画像がワープするバグの修正、およびフェード中の操作ロックを実装。

#### 対象ファイル: 
    - `src/BackgroundManager.h`
    - `src/BackgroundManager.cpp`
    - `src/Application_Playback.cpp`

#### 対応: フレーミング設定セッターの純化とロック実装
    - `BackgroundManager` にロード時用の絶対値セッター `SetBackgroundFraming` を新設し、`Application_Playback` 側の呼び出しを置換。NEW変数の更新のみに純化した。
    - マウス操作用セッター (`SetArtFramingScale`, `SetArtFramingScroll`) について、フェード中（NEW画像が存在しフェード完了前）の呼び出しを即座に `return` で弾き返す操作ロック機構を実装し、平常時のみOLD変数を更新するようにした。
    - 現在のフレーミング状態を適切に返すゲッター `GetFraming` を追加し、フェード進行中はNEW、平常時はOLDのステートを返却するようにした。
