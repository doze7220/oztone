# RES:実装計画・作業レポート Phase 20-4: ConfigManager の実装対象別ファイル分割

## 1. 実装目的
現在2300行を超えて肥大化している `ConfigManager.cpp` について、人間の編集領域とAIの編集領域を完全分離し、今後の保守性・拡張性を高めるため、クラス定義（`ConfigManager.h`）は変更せず実装対象（UI等）別に実装ファイルを物理分割する。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の実装物理分割
    - `ConfigManager.cpp` の各種設定値のGetter/Setter実装を、責務ごとに `ConfigManager_Window.cpp`, `ConfigManager_Playlist.cpp`, `ConfigManager_Playback.cpp`, `ConfigManager_LogoMenu.cpp`, `ConfigManager_Visualizer.cpp`, `ConfigManager_System.cpp` の6つの新しいソースファイルに移行する。
    - `ConfigManager.cpp` 本体には `DEFAULT_INI_CONTENT` の定義、およびファイルI/O関連のコア処理のみを残す。
    - `CMakeLists.txt` を更新し、新たに作成したファイルをビルド対象に追加する。
    - 分割後、`PROJECT_ARCHITECTURE.md` に ConfigManager が複数ファイルへ物理分割された旨とその役割を追記する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManager_Window.cpp の作成
    - ウィンドウ設定、Z-Order、可視性(Visibility)、背景アート等に関する実装を移行する。
[x] タスク2: ConfigManager_Playlist.cpp の作成
    - プレイリストUI全般に関する実装を移行する。
[x] タスク3: ConfigManager_Playback.cpp の作成
    - シークバー、再生コントロール、音量コントロールに関する実装を移行する。
[x] タスク4: ConfigManager_LogoMenu.cpp の作成
    - アプリアイコン、ロゴ拡張メニューに関する実装を移行する。
[x] タスク5: ConfigManager_Visualizer.cpp の作成
    - ビジュアライザ全般に関する実装を移行する。
[x] タスク6: ConfigManager_System.cpp の作成
    - グローバルホットキー、OSDなどシステム連携に関する実装を移行する。
[ ] タスク7: CMakeLists.txt の更新と ConfigManager.cpp の整理
    - 分割した6ファイルを `CMakeLists.txt` に追加し、元の `ConfigManager.cpp` には `DEFAULT_INI_CONTENT` とコアのファイルI/O処理のみを残してクリーンアップする。
[ ] タスク8: PROJECT_ARCHITECTURE.md の更新
    - ConfigManager が実装対象別に7ファイルへ物理分割された旨と各ファイルの役割を追記する。

## 4. 詳細作業内容
### タスク1: ConfigManager_Window.cpp の作成
    - `src/ConfigManager_Window.cpp` を新規作成し、`#include "ConfigManager.h"` を追加した。
    - `ConfigManager.cpp` から以下のウィンドウ設定・背景アート関連メソッドの実装を移行した。
        - `ConfigManager::SaveWindowPosition`
        - `ConfigManager::SetZOrder`
        - `ConfigManager::SetSavePositionOnExit`
        - `ConfigManager::SetEnableResize`
        - `ConfigManager::SetLockWindowPosition`
        - `ConfigManager::SetBackgroundArtMode`
    - ※ 可視性 (Visibility) フラグに関しては、設定の Getter がインライン定義のみであり Setter が存在しないため、移行処理は発生していない。
    - ※ `CMakeLists.txt` への追加はタスク7で実施するため、本タスクでのビルド検証は保留。

### タスク2: ConfigManager_Playlist.cpp の作成
    - `src/ConfigManager_Playlist.cpp` を新規作成し、`#include "ConfigManager.h"` と必要なヘッダを含めた。
    - `ConfigManager.cpp` からプレイリスト関連のメソッド（`SetIsPlaylistPinned`, `SetDefaultPlaylistPath`, `SetPlaylistPosition`, `GetAvailablePlaylists`）の実装を移行した。
    - ※ その他プレイリストのレイアウト等の設定値に関するメソッドは、ヘッダファイル内でインラインの Getter として定義されており `.cpp` ファイルに実装が存在しないため、これ以上の移行処理は発生していない。

### タスク3: ConfigManager_Playback.cpp の作成
    - `src/ConfigManager_Playback.cpp` を新規作成し、`#include "ConfigManager.h"` と `<string>` を追加した。
    - `ConfigManager.cpp` から再生コントロールや音量設定に関連する以下のメソッドの実装を移行した。
        - `ConfigManager::SetDefaultVolume`
        - `ConfigManager::SetShuffleMode`
        - `ConfigManager::SetSkipSeconds`
    - ※ シークバー関連や、その他再生コントロール、音量コントロールの一部のUI設定については、ヘッダファイル内でインラインの Getter として定義されており `.cpp` ファイルに実装が存在しないため、これ以上の移行処理は発生していない。
    - ※ `CMakeLists.txt` への追加はタスク7で実施するため、本タスクでのビルド検証は保留。

### タスク4: ConfigManager_LogoMenu.cpp の作成
    - `src/ConfigManager_LogoMenu.cpp` を新規作成し、`#include "ConfigManager.h"` を追加した。
    - ※ アプリアイコン (AppLogo) および ロゴ拡張メニュー (LogoMenu) に関する設定メソッドは、すべてヘッダファイル内でインラインの Getter として定義されており、`.cpp` 側に実装が分離されているメソッドが一つも存在しなかったため、実装の移行処理は発生していない。
    - ※ `CMakeLists.txt` への追加はタスク7で実施するため、本タスクでのビルド検証は保留。

### タスク5: ConfigManager_Visualizer.cpp の作成
    - `src/ConfigManager_Visualizer.cpp` を新規作成し、`#include "ConfigManager.h"` と `<string>` を追加した。
    - `ConfigManager.cpp` から以下のビジュアライザ関連の Setter メソッドの実装を移行した。
        - `SetVisualizerMode`, `SetHighFreqNoiseThreshold`, `SetBandGains`
        - PrismBeat 関連: `SetPrismBeatMaxHeightRatio`
        - HaloDust 関連: `SetHaloDustBaseRadiusRatio` 等の多数のパラメータ
    - ※ `CMakeLists.txt` への追加はタスク7で実施するため、本タスクでのビルド検証は保留。

### タスク6: ConfigManager_System.cpp の作成
    - `src/ConfigManager_System.cpp` を新規作成し、`#include "ConfigManager.h"` を記述した。
    - ※ グローバルホットキーおよびOSDに関するメソッド（`SetShowHotkeys`, `SetEnableOSD`, `SetNextTrackHotkey` 等）は、ヘッダに宣言のみで実装が存在しないか、インライン定義のGetterのみであったため、`.cpp` 側に移行すべき実装は一つも存在しなかった。そのため実装の移行処理は発生していない。
    - ※ `CMakeLists.txt` への追加はタスク7で実施するため、本タスクでのビルド検証は保留。

### タスク7: CMakeLists.txt の更新と ConfigManager.cpp の整理
    - （未実施）

### タスク8: PROJECT_ARCHITECTURE.md の更新
    - （未実施）

## 5. HOTFIX1
### 原因・理由: 該当なし
    - 

### 対応: 該当なし
    - 
