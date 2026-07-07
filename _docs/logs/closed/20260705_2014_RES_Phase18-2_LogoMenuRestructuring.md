# RES:実装計画・作業レポート Phase 18-2: ロゴ拡張メニューの再構築

## 1. 実装目的
ロゴ拡張メニューの並び順と構成を再構築し、赤い斜線表現を完全に廃止してミニマルなUIへ洗練させる。

## 2. アーキテクチャ設計
### 要件1: 赤い斜線表現の完全廃止と設定削除
    - LogoMenuWidget の斜線描画ロジックを削除し、機能の無効化やOFF状態は不透明度減衰（半透明化）のみで表現する。
    - ConfigManager から斜線用の設定（`MenuStrikeLength`, `MenuStrikeThickness`等）を除外し、INIファイルの読み書きからも削除する。

### 要件2: メニュー配列の再構築とUIの追加・削除 (Window)
    - `Window` クラスで管理する `m_logoMenuItems` の配列を以下の構成に再構築する。
      0. 📽️ ビジュアライザ切替 (既存)
      1. 🖼️ 背景表示切り替え (新規追加: `ID_LOGO_BG_MODE`)
      2. 🔀 シャッフルモード (既存)
      3. ↔️ プレイリスト左右配置 (既存)
      4. ◢ リサイズモード (新規追加: `ID_LOGO_RESIZE_MODE`)
      5. ⚓ 画面固定 (新規追加: `ID_LOGO_LOCK_POS`)
      6. ❌ アプリ終了 (既存)
    - 不要になった「プレイリスト固定」「曲削除」等は完全に削除する。

### 要件3: クリックイベントのフックと機能連動 (Window)
    - `WindowProc` のロゴメニュークリック処理 (`WM_LBUTTONDOWN`) にて、新規追加ボタンの処理を実装する。
      - `ID_LOGO_BG_MODE`: `GetBackgroundArtMode()` を 0→1→2→0 にトグルして `SetBackgroundArtMode()` を呼ぶ。
      - `ID_LOGO_RESIZE_MODE`: `GetEnableResize()` を反転して `SetEnableResize()` を呼ぶ。
      - `ID_LOGO_LOCK_POS`: `GetLockWindowPosition()` を反転して `SetLockWindowPosition()` を呼ぶ。

### 要件4: アフォーダンスと状態表現の描画 (LogoMenuWidget)
    - 各アイコンの状態に応じた表現と、ホバー時の上部説明テキスト動的切り替えを実装。
    - 背景表示切替: `GetBackgroundArtMode()` が 1(非表示)なら半透明、0(再生中)なら白、2(固定)なら白＋右下に「2」のインジケーター。
    - リサイズモード: `GetEnableResize()` が trueなら白、falseなら半透明。
    - 画面固定: `GetLockWindowPosition()` が true(固定ON)なら⚓(白)＋右下に🔒(白字黒フチ)、false(移動可)なら⚓(半透明)＋右下に🔓(白字黒フチ)。
    - 既存機能（シャッフル、左右配置、アプリ終了、ビジュアライザ）の既存ロジックは適切に維持・調整する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManagerから赤い斜線設定の削除
    - `ConfigManager.h`, `ConfigManager.cpp` から `MenuStrikeLength`, `MenuStrikeThickness` 関連のコードおよびデフォルト設定(`DEFAULT_INI_CONTENT`)からの削除を完了。
[x] タスク2: LogoMenuWidgetから赤い斜線描画ロジックの削除
    - `LogoMenuWidget.h`, `LogoMenuWidget.cpp` から `m_lineBrush` の生成および破棄、`DrawLine` による斜線描画処理を削除。
[x] タスク3: Windowクラスでのメニュー配列再構築
    - `Window.h` のコマンドIDを整理し、`Window.cpp` の `m_logoMenuItems` を要求通りの順序（ビジュアライザ、背景表示、シャッフル、プレイリスト配置、リサイズ、固定、終了）に再構成。
[x] タスク4: WindowProcでのクリックイベント実装
    - `WM_LBUTTONDOWN` ハンドラ内に `ID_LOGO_BG_MODE`, `ID_LOGO_RESIZE_MODE`, `ID_LOGO_LOCK_POS` の処理を追加し、状態をトグル・切り替えするよう実装した。
[x] タスク5: LogoMenuWidgetでのアフォーダンスと状態表現の実装
    - 各機能の状態に応じたインジケーター（1/2/🔒/🔓）の描画、および無効時のアイコン半透明化を実装。説明テキストの動的更新も完了。
[x] タスク6: ドキュメントの更新
    - 本実装計画書のステータス更新と、`PROJECT_ARCHITECTURE.md` のロゴ拡張メニューに関する仕様説明の修正を実施。

## 4. 詳細作業内容
### タスク1: ConfigManagerから赤い斜線設定の削除
    - ConfigManagerにおける `MenuStrikeLength`, `MenuStrikeThickness` プロパティ、INIからの読み取り・デフォルト値書き出し部分をすべて削除した。これにより不要な設定情報へのアクセスを無くした。
### タスク2: LogoMenuWidgetから赤い斜線描画ロジックの削除
    - `LogoMenuWidget` クラスから `m_lineBrush` を削除し、OFF時に赤い斜線を引いていたロジックを完全に削除。機能無効時は `m_inactiveIconBrush` (Alpha 0.4の白) のみで描画するようにした。
### タスク3: Windowクラスでのメニュー配列再構築
    - `Window::ID_LOGO_BG_MODE`, `ID_LOGO_RESIZE_MODE`, `ID_LOGO_LOCK_POS` を新設し、使われなくなった `ID_LOGO_PIN_PLAYLIST` は削除。メニュー配列を7項目で再定義した。
### タスク4: WindowProcでのクリックイベント実装
    - `WindowProc` でのメニュークリック判定において、新設した3つのコマンドに対する状態更新処理（`ConfigManager`への設定反映）を実装した。
### タスク5: LogoMenuWidgetでのアフォーダンスと状態表現の実装
    - `LogoMenuWidget::Draw` 内でアイコンの `active` 状態を動的判定するようにし、右下のインジケーター（Visualizerの1/2、背景の2、画面固定の🔒🔓など）を縁取り付きで描画。ホバー時の上部説明文言も設定に応じて正しく表示されるように対応。
### タスク6: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の記述を更新し、赤い斜線ではなく半透明化になったこと、および新規追加されたメニュー内容を反映した。本作業レポートの全タスクにも完了チェックを入れた。

### Hotfix (2026-07-05)
ビルド時に発生した型の暗黙的キャストによるワーニング（データ損失の可能性）を解消するため、以下の修正を実施した。
- `LogoMenuWidget.cpp`: `indicatorText.length()` (`size_t`) を `UINT32` へ明示的にキャスト (`C4267` 対策)
- `Visualizer.cpp`: `spectrumSize - 1` (`size_t`) を `float` へ明示的にキャスト (`C4244` 対策)

### Hotfix 2 (2026-07-05)
- **終了アイコンの配置変更**: `Window.cpp` におけるメニュー配列を再構築し、終了アイコン(❌)を一番左(配列先頭)へ移動した。
- **インジケーター設定の外部化**: ロックアイコン(🔒/🔓)などのサポートアイコンについて、INIから独立してサイズとオフセットを調整できるよう `LockIconFontSize`, `LockIconOffsetX`, `LockIconOffsetY` を `ConfigManager` に追加し、`LogoMenuWidget` で読み込むよう修正した。
