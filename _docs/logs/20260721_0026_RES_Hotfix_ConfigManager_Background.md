# RES:HOTFIX作業レポート: ConfigManager_Background.cpp の分離

## 1. 実装目的
`ConfigManager_Window.cpp` に混在していた `[Background]` セクションの設定パース・保存ロジックを分離し、データ層の責務を完全に独立させること。

## 2. 調査内容
`ConfigManager_Window.cpp` には背景アートモードやクロスフェード時間、不透明度などの設定処理が含まれていた。これを新規ファイル `ConfigManager_Background.cpp` に移行し、カプセル化を図る。`ConfigManager.h` への委譲メソッドの追加と、`ConfigManager.cpp` からの呼び出し設定を行うことで、既存のゲッターの振る舞いを変えることなく物理ファイルの移動と責務分離を実現する。

## 3. 対象ファイル
* `src/ConfigManager_Background.cpp` (新規)
* `src/ConfigManager_Window.cpp` (修正)
* `src/ConfigManager.h` (修正)
* `src/ConfigManager.cpp` (修正)
* `CMakeLists.txt` (修正)
* `PROJECT_ARCHITECTURE.md` (修正)

## 4. 実装タスクリスト
[x] タスク1: ConfigManager_Background.cpp の新設と処理移行
[x] タスク2: ビルド設定の更新 (CMakeLists.txtへの追加)
[x] タスク3: ドキュメントの更新 (PROJECT_ARCHITECTURE.mdへの追記)

## 5. 詳細作業内容
* タスク1: ConfigManager_Background.cpp の新設と処理移行
    - `src/ConfigManager_Background.cpp` を新規作成。
    - `src/ConfigManager_Window.cpp` から `SetBackgroundArtMode`, `SetCrossfadeDuration` および `[Background]` の読み込み処理を移行。
    - `src/ConfigManager.h` に `LoadBackgroundSettings()` を追加。
    - `src/ConfigManager.cpp` の `LoadSettings()` から `LoadBackgroundSettings()` を呼び出すよう修正。
* タスク2: ビルド設定の更新
    - `CMakeLists.txt` の `SOURCES` に `src/ConfigManager_Background.cpp` を追加。
* タスク3: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の `ConfigManager` の物理分割ファイル一覧に `3. ConfigManager_Background.cpp: 背景アート・クロスフェード設定用` を追記。
