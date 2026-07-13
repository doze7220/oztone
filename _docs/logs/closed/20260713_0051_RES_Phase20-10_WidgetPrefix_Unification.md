# RES:実装計画・作業レポート Phase 20-10: 全Widgetのプレフィックス命名統一

## 1. 実装目的
Phase 20-9 までに `Widgets.cpp` の解体が行われ、新規に分離されたファイル群は `Widget_` プレフィックスを持つようになった。しかし、過去のフェーズ（Phase 17 等）で先行して独立していた中型Widget群（`PlaylistWidget`, `LogoMenuWidget` など）は旧来のファイル名のままとなっている。
本フェーズでは、これらのファイル名を `Widget_*.h/cpp` の規則に従ってリネームし、プロジェクト内のすべてのWidgetファイルの一覧性と統一性を極限まで高めることを目的とする。

## 2. アーキテクチャ設計
### 要件1: 先行独立Widget群のファイルリネーム
    - 以下のファイルを物理的にリネームする（クラス名は変更しない）。
        - `src/LogoMenuWidget.h` → `src/Widget_LogoMenu.h`
        - `src/LogoMenuWidget.cpp` → `src/Widget_LogoMenu.cpp`
        - `src/PlaylistWidget.h` → `src/Widget_Playlist.h`
        - `src/PlaylistWidget.cpp` → `src/Widget_Playlist.cpp`
        - `src/PlaybackControlsWidget.h` → `src/Widget_PlaybackControls.h`
        - `src/PlaybackControlsWidget.cpp` → `src/Widget_PlaybackControls.cpp`
        - `src/VolumeControlWidget.h` → `src/Widget_VolumeControl.h`
        - `src/VolumeControlWidget.cpp` → `src/Widget_VolumeControl.cpp`

### 要件2: 依存関係の解決
    - 呼び出し元における旧ファイル名の `#include` を新ファイル名へ書き換える。
        - 対象ファイル: 新しい名称になった対象Widgetのcppファイル自身や、`src/Renderer.cpp`, 各マネージャなどのソースコード群。

### 要件3: ビルド設定の更新
    - `CMakeLists.txt` 内のソースおよびヘッダのリストを新しいファイル名に更新する。

### 要件4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` を更新し、すべてのWidgetが `Widget_*.h/cpp` という構成に統一された旨を反映する。

## 3. 実装タスクリスト
[x] タスク1: 先行独立Widget群のファイルリネーム
    - `LogoMenuWidget`, `PlaylistWidget`, `PlaybackControlsWidget`, `VolumeControlWidget` の `.h` および `.cpp` ファイルを `Widget_` プレフィックスの名称にリネームする。
[x] タスク2: ソースコードの依存関係（#include）の修正
    - `#include "LogoMenuWidget.h"` 等を `#include "Widget_LogoMenu.h"` 等に置換する。
[x] タスク3: CMakeLists.txt の更新
    - `CMakeLists.txt` 内の該当ファイル名を新しい名称に置換し、ビルド環境を整える。
[x] タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` を修正し、ファイル構造の記述を統一する。

## 4. 詳細作業内容
### タスク1: 先行独立Widget群のファイルリネーム
    - 以下のファイルを物理リネームした。
      - `src/LogoMenuWidget.h / .cpp` → `src/Widget_LogoMenu.h / .cpp`
      - `src/PlaylistWidget.h / .cpp` → `src/Widget_Playlist.h / .cpp`
      - `src/PlaybackControlsWidget.h / .cpp` → `src/Widget_PlaybackControls.h / .cpp`
      - `src/VolumeControlWidget.h / .cpp` → `src/Widget_VolumeControl.h / .cpp`

### タスク2: ソースコードの依存関係（#include）の修正
    - `src/Widget_LogoMenu.cpp`, `src/Widget_Playlist.cpp`, `src/Widget_PlaybackControls.cpp`, `src/Widget_VolumeControl.cpp` および `src/Renderer.cpp` にて、対象ファイルのインクルード宣言を `Widget_` プレフィックスの新しいファイル名へ一括置換した。

### タスク3: CMakeLists.txt の更新
    - `CMakeLists.txt` 内の `SOURCES` および `HEADERS` に定義されていた古いファイル名を `Widget_` プレフィックスの名称へ更新した。

### タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` 内に記載されている `LogoMenuWidget`, `PlaylistWidget`, `PlaybackControlsWidget`, `VolumeControlWidget` のファイルパス記述を、クラス名はそのままに新しい `Widget_` プレフィックスの名称（`src/Widget_*.h/cpp`）へと更新し、アーキテクチャ資料と実際のファイル構造の整合性を完全に統一した。
