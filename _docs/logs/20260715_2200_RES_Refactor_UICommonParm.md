# RES:HOTFIX作業レポート: UIパラメーター共通化リファクタリング

## 1. 実装目的
* 分散しているUI関連の設定パラメーター（フォント、シャドウ、カラー、ディレイなど）を `[UI_Common_Parm]` セクションに集約し、SSOT (Single Source of Truth) を実現する。
* 各UIのサイズや座標計算には影響を与えず、見た目を一切変えることなく内部参照先だけを共通化する。

## 2. 調査内容
* 既存の各コンポーネント（Window, Playlist, PlaybackControls, LogoMenu, OSD, GlobalHotkeys, System等）の設定読み込み処理（`ConfigManager_*.cpp`）において、重複したキーが定義・ロードされていることを確認。
* フォント（ベース、等幅、アイコン、OSD等）、影のオフセット/不透明度/カラー、フォーカス（ホバー）時のカラー、フェードアウト速度や離脱ディレイなどを共通化の対象として選定。

## 3. 対象ファイル
* `D:\ozlab\oztone\src\ConfigManager_DefaultIni.h`
* `D:\ozlab\oztone\src\ConfigManager.h`
* `D:\ozlab\oztone\src\ConfigManager.cpp`
* `D:\ozlab\oztone\src\ConfigManager_Window.cpp`
* `D:\ozlab\oztone\src\ConfigManager_System.cpp`
* `D:\ozlab\oztone\src\ConfigManager_LogoMenu.cpp`
* `D:\ozlab\oztone\src\ConfigManager_Playback.cpp`
* `D:\ozlab\oztone\src\ConfigManager_Playlist.cpp`
* 各UI描画・レイアウト更新用の `Widget_*.cpp` および `Renderer_*.cpp`、`LayoutCalculator.cpp`

## 4. 実装タスクリスト
[x] タスク1: `ConfigManager_DefaultIni.h` を更新し `[UI_Common_Parm]` を新設。各コンポーネントから重複設定キーを削除。
[x] タスク2: `ConfigManager.h` および `ConfigManager.cpp` で `LoadCommonSettings()` を実装し、共通変数を追加。不要になった個別パラメーターを削除。
[x] タスク3: 個別の `ConfigManager_*.cpp` ファイルから削除対象のキー読み込み処理を取り除く。
[x] タスク4: 各UIコンポーネントの描画ロジック（`Widget_*.cpp`、`LayoutCalculator.cpp` 等）から、新しい共通パラメーターを取得するようにコードを置換・修正。
[x] タスク5: ビルドを行い、エラーがないこと、および正常動作することを確認（要件4のプレイリスト専用離脱ディレイの維持も確認済）。

## 5. 詳細作業内容
* タスク1: `[UI_Common_Parm]` を追加
    - フォント関連: `BaseFontFamily`, `MonoFontFamily`, `IconFontFamily`, `OsdFontFamily` を追加。
    - 共通色: `FocusColor` を追加（Hover/PlayingItemColorの共通化）。
    - 影設定: `EnableShadow`, `ShadowColor`, `ShadowOffsetX`, `ShadowOffsetY`, `ShadowOpacity` を追加。
    - 動作速度: `HoverFadeOutSpeed`, `BaseLeaveDelay` を追加。
    - 既存のセクション (`[Layout_Window]`, `[Layout_Playlist]` 等) にあった対象キーを削除。

* タスク2〜3: `ConfigManager` の更新
    - `LoadSettings()` から `LoadCommonSettings()` を呼び出すように変更。
    - 共通パラメーター用のゲッター (`GetBaseFontFamily()` など) を追加し、個別UIの重複ゲッターを削除。
    - 各 `ConfigManager_*.cpp` の不要な読み込み処理を削除。

* タスク4: 呼び出し元の置換
    - `LayoutCalculator.cpp` や各 `Widget_*.cpp` 内の古いゲッター (`GetTrackCountFontFamily`, `GetVolumeEnableShadow`, `GetSkipTextShadowColor` など) を新しい共通ゲッターに一括置換。
    - 要件4であるプレイリストの独立した離脱ディレイ（`PlaylistLeaveDelay`）は仕様通り別枠として維持し、フェードアウトには共通の `HoverFadeOutSpeed` を使用するよう確認済み。

* タスク5: ビルド確認
    - `ConfigManager.h` 側のゲッター関数との不一致によるビルドエラーを修正。全コンパイルが通り、見た目の変更なく動作する状態に修正を完了。
