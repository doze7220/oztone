# RES:HOTFIX作業レポート: Phase 8-2 ControlHoverHeightの所属セクション適正化

## 1. 実装目的
`ControlHoverHeight`設定項目の所属セクションを、本来あるべき`[Layout_PlaybackControls]`セクションへ移動し、設定管理の整合性を確保する。

## 2. 調査内容
原因：歴史的経緯により、プレイバックコントロール関連の設定である`ControlHoverHeight`が、誤って`[Layout_Window]`セクションに配置されていたため、配置の不整合が発生していた。
修正方針：INIファイルのデフォルト文字列および各設定読み込み処理において、`ControlHoverHeight`の所属を`[Layout_Window]`から`[Layout_PlaybackControls]`へ適正化する。

## 3. 対象ファイル
* `src/ConfigManager_DefaultIni.h`
* `src/ConfigManager_Window.cpp`
* `src/ConfigManager_Playback.cpp`

## 4. 実装タスクリスト
[x] タスク1: DEFAULT_INI_CONTENT の整理 - `ControlHoverHeight`を`[Layout_PlaybackControls]`へ移動
[x] タスク2: ConfigManager_Window.cpp からのパージ - パース処理を削除
[x] タスク3: ConfigManager_Playback.cpp への統合 - パース処理を追加

## 5. 詳細作業内容
* タスク1: DEFAULT_INI_CONTENT の整理
    - `src/ConfigManager_DefaultIni.h`の`DEFAULT_INI_CONTENT`において、`ControlHoverHeight=50.0`を`[Layout_Window]`から`[Layout_PlaybackControls]`内へ移動した。
* タスク2: ConfigManager_Window.cpp からのパージ
    - `src/ConfigManager_Window.cpp`内の`LoadWindowSettings`における`ControlHoverHeight`の読み込み処理を削除した。
* タスク3: ConfigManager_Playback.cpp への統合
    - `src/ConfigManager_Playback.cpp`内の`LoadPlaybackSettings`に、`ControlHoverHeight`の読み込み処理を移行・追加した。
