# RES:HOTFIX作業レポート: NextTrack表示機能の完全パージ

## 1. 実装目的
機能の陳腐化に伴い、不要となった `NextTrack` UI機能（次の曲の表示機能）を完全に削除・パージする。

## 2. 調査内容
次曲の先読み（PrefetchNextTrack）等のバックグラウンド解析ロジック自体は再生のシームレス化のために維持しつつ、UI表示や設定項目のみをプロジェクト全体から安全にパージする方針で実装する。

## 3. 対象ファイル
* `CMakeLists.txt`
* `src/ConfigManager.h` / `src/ConfigManager_Playback.cpp` / `src/ConfigManager_DefaultIni.h`
* `src/LayoutCalculator.h` / `src/LayoutCalculator.cpp`
* `src/Renderer.h` / `src/Renderer.cpp` / `src/Renderer_Initialize.cpp`
* `src/Application_Render.cpp`
* `src/Widget_NextTrack.h` / `src/Widget_NextTrack.cpp` (削除)
* `PROJECT_ARCHITECTURE.md`

## 4. 実装タスクリスト
- [x] タスク1: Widget_NextTrackの物理削除
- [x] タスク2: ConfigManagerのクリーンアップ
- [x] タスク3: LayoutCalculatorのクリーンアップ
- [x] タスク4: RendererとApplicationの連携解除

## 5. 詳細作業内容
* タスク1: Widget_NextTrackの物理削除
    - `src/Widget_NextTrack.h` と `src/Widget_NextTrack.cpp` を物理的に削除した。
    - `CMakeLists.txt` のソースリストから上記2ファイルを除外した。
* タスク2: ConfigManagerのクリーンアップ
    - `ConfigManager.h` と `ConfigManager_Playback.cpp` から `[Layout_NextTrack]` に関連する変数、ゲッター、INIロード処理を削除した。
    - `ShowNextTrack` 設定を削除した。
    - `ConfigManager_DefaultIni.h` のデフォルトINIから該当セクションと設定値を削除した。
* タスク3: LayoutCalculatorのクリーンアップ
    - `LayoutCalculator.h` と `LayoutCalculator.cpp` から `NextTrackLayout` 構造体と `CalculateNextTrackLayout` を削除した。
* タスク4: RendererとApplicationの連携解除
    - `Renderer.h` および `Renderer.cpp` から `SetNextTrackInfo` メソッドとその保持変数を削除した。
    - `Renderer_Initialize.cpp` における `Widget_NextTrack` の登録処理を削除した。
    - `Application_Render.cpp` のメインループから `m_renderer.SetNextTrackInfo(...)` 呼び出しを削除した。
* 追加作業
    - `PROJECT_ARCHITECTURE.md` から `NextTrackWidget` に関する記述を削除し、ドキュメントの整合性を保った。
