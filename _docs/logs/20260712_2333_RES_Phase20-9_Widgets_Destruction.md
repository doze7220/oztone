# RES:実装計画・作業レポート Phase 20-9: Widgets.cpp の完全解体とプレフィックス命名分割

## 1. 実装目的
AI-IDEのコンテキスト節約および保守性の極限向上を目指し、現在複数の小型Widgetが同居している `src/Widgets.h` および `src/Widgets.cpp` を完全に解体する。
また、ファイル名の一覧性を高めるため、分割後のすべてのファイル名に `Widget_` というプレフィックスを付与する。

## 2. アーキテクチャ設計
### 要件1: 各Widgetの個別ファイル化 (プレフィックス命名)
    - 現在 `src/Widgets.h / .cpp` に存在するすべてのWidgetクラスを、クラス名（`AppLogoWidget` 等）は維持したまま個別の物理ファイルへ分割する。
    - 分割先ファイル一覧:
        - `src/Widget_AppLogo.h / .cpp`
        - `src/Widget_TrackInfo.h / .cpp`
        - `src/Widget_NextTrack.h / .cpp`
        - `src/Widget_SeekBar.h / .cpp`
        - `src/Widget_GlobalHotkeys.h / .cpp`
        - `src/Widget_ResizeGrip.h / .cpp`
        - `src/Widget_Osd.h / .cpp`

### 要件2: 依存関係の再構築
    - 各 `Widget_*.h` は `#pragma once` を持ち、描画に必須となる共通ヘッダ (`WidgetCommon.h` や `WidgetContext.h`、必要に応じて `ConfigManager.h` や `TrackDatabase.h` 等) を適切にインクルードする。
    - 各 `Widget_*.cpp` は自身のヘッダをインクルードし、実装を配置する。
    - 呼び出し元（主に `src/Renderer.cpp` 等）で `Widgets.h` をインクルードしている箇所を、分割後の各ヘッダファイル（`Widget_AppLogo.h` 等）のインクルードに置き換える。

### 要件3: ビルド設定の更新と旧ファイルの削除
    - 移行完了後、中身が不要になった旧ファイル (`src/Widgets.h` および `src/Widgets.cpp`) を物理削除する。
    - `CMakeLists.txt` のソースファイルリストを更新し、旧ファイルを削除、新規作成した各 `.cpp` と `.h` を追加する。

## 3. 実装タスクリスト
[x] タスク1: AppLogoWidget の分離
    - `src/Widgets.h / .cpp` から `AppLogoWidget` の宣言と実装を抽出し、`src/Widget_AppLogo.h / .cpp` へ移行する。
    - 呼び出し元のインクルードを修正し、`CMakeLists.txt` に新しいファイルを追加してビルドが通ることを確認する。
[x] タスク2: TrackInfoWidget の分離
    - `src/Widgets.h / .cpp` から `TrackInfoWidget` を抽出し、`src/Widget_TrackInfo.h / .cpp` へ移行する。
    - インクルード修正と `CMakeLists.txt` への追加を行う。
[x] タスク3: NextTrackWidget の分離
    - `src/Widgets.h / .cpp` から `NextTrackWidget` を抽出し、`src/Widget_NextTrack.h / .cpp` へ移行する。
    - インクルード修正と `CMakeLists.txt` への追加を行う。
[x] タスク4: SeekBarWidget の分離
    - `src/Widgets.h / .cpp` から `SeekBarWidget` を抽出し、`src/Widget_SeekBar.h / .cpp` へ移行する。
    - インクルード修正と `CMakeLists.txt` への追加を行う。
[x] タスク5: GlobalHotkeysWidget の分離
    - `src/Widgets.h / .cpp` から `GlobalHotkeysWidget` を抽出し、`src/Widget_GlobalHotkeys.h / .cpp` へ移行する。
    - インクルード修正と `CMakeLists.txt` への追加を行う。
[x] タスク6: ResizeGripWidget の分離
    - `src/Widgets.h / .cpp` から `ResizeGripWidget` を抽出し、`src/Widget_ResizeGrip.h / .cpp` へ移行する。
    - インクルード修正と `CMakeLists.txt` への追加を行う。
[x] タスク7: OsdWidget の分離
    - `src/Widgets.h / .cpp` から `OsdWidget` を抽出し、`src/Widget_Osd.h / .cpp` へ移行する。
    - インクルード修正と `CMakeLists.txt` への追加を行う。
[ ] タスク8: 旧 Widgets.h / .cpp の削除とクリーンアップ
    - すべてのWidgetの移行が完了し、中身が空になった `src/Widgets.h / .cpp` を物理削除する。
    - `CMakeLists.txt` から旧ファイルのエントリを削除し、最終的なビルド確認を行う。
[ ] タスク9: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` を確認し、`Widgets.h` や `Widgets.cpp` に関する記述を分割後のファイル構造 (`Widget_AppLogo.cpp` 等) を反映した形へ更新する。

## 4. 詳細作業内容
### タスク1: AppLogoWidget の分離
    - `src/Widget_AppLogo.h` を新規作成し、`Widgets.h` から `AppLogoWidget` の宣言を移行。
    - `src/Widget_AppLogo.cpp` を新規作成し、`Widgets.cpp` から `AppLogoWidget` の実装を移行。
    - 移行後、`Widgets.h` / `.cpp` から `AppLogoWidget` 関連のコードを削除。
    - `Renderer.cpp` に `#include "Widget_AppLogo.h"` を追加。
    - `CMakeLists.txt` に `src/Widget_AppLogo.h` と `src/Widget_AppLogo.cpp` を追加。

### タスク2: TrackInfoWidget の分離
    - `src/Widget_TrackInfo.h` を新規作成し、`Widgets.h` から `TrackInfoWidget` の宣言を移行。
    - `src/Widget_TrackInfo.cpp` を新規作成し、`Widgets.cpp` から `TrackInfoWidget` の実装を移行。
    - 移行後、`Widgets.h` / `.cpp` から `TrackInfoWidget` 関連のコードを削除。
    - `Renderer.cpp` に `#include "Widget_TrackInfo.h"` を追加。
    - `CMakeLists.txt` に `src/Widget_TrackInfo.h` と `src/Widget_TrackInfo.cpp` を追加。

### タスク3: NextTrackWidget の分離
    - `src/Widget_NextTrack.h` を新規作成し、`Widgets.h` から `NextTrackWidget` の宣言を移行。
    - `src/Widget_NextTrack.cpp` を新規作成し、`Widgets.cpp` から `NextTrackWidget` の実装を移行。
    - 移行後、`Widgets.h` / `.cpp` から `NextTrackWidget` 関連のコードを削除。
    - `Renderer.cpp` に `#include "Widget_NextTrack.h"` を追加。
    - `CMakeLists.txt` に `src/Widget_NextTrack.h` と `src/Widget_NextTrack.cpp` を追加。

### タスク4: SeekBarWidget の分離
    - `src/Widget_SeekBar.h` を新規作成し、`Widgets.h` から `SeekBarWidget` の宣言を移行。
    - `src/Widget_SeekBar.cpp` を新規作成し、`Widgets.cpp` から `SeekBarWidget` の実装を移行。
    - 移行後、`Widgets.h` / `.cpp` から `SeekBarWidget` 関連のコードを削除。
    - `Renderer.cpp` に `#include "Widget_SeekBar.h"` を追加。
    - `CMakeLists.txt` に `src/Widget_SeekBar.h` と `src/Widget_SeekBar.cpp` を追加。
### タスク5: GlobalHotkeysWidget の分離
    - `src/Widget_GlobalHotkeys.h` を新規作成し、`Widgets.h` から `GlobalHotkeysWidget` の宣言を移行。
    - `src/Widget_GlobalHotkeys.cpp` を新規作成し、`Widgets.cpp` から `GlobalHotkeysWidget` の実装と関連する無名名前空間(列挙型 `ActionID`)を移行。
    - 移行後、`Widgets.h` / `.cpp` から `GlobalHotkeysWidget` 関連のコードを削除。
    - `Renderer.cpp` に `#include "Widget_GlobalHotkeys.h"` を追加。
    - `CMakeLists.txt` に `src/Widget_GlobalHotkeys.h` と `src/Widget_GlobalHotkeys.cpp` を追加。

### タスク6: ResizeGripWidget の分離
    - `src/Widget_ResizeGrip.h` を新規作成し、`Widgets.h` から `ResizeGripWidget` の宣言を移行。
    - `src/Widget_ResizeGrip.cpp` を新規作成し、`Widgets.cpp` から `ResizeGripWidget` の実装を移行。
    - 移行後、`Widgets.h` / `.cpp` から `ResizeGripWidget` 関連のコードを削除。
    - `Renderer.cpp` に `#include "Widget_ResizeGrip.h"` を追加。
    - `CMakeLists.txt` に `src/Widget_ResizeGrip.cpp` を追加。

### タスク7: OsdWidget の分離
    - 既存の `src/OsdWidget.h / .cpp` から `src/Widget_Osd.h / .cpp` へ名称を合わせつつ移行。
    - 移行後、不要になった `OsdWidget.h / .cpp` を削除。
    - `Renderer.cpp` の `#include "OsdWidget.h"` を `#include "Widget_Osd.h"` に置換。
    - `CMakeLists.txt` の `src/OsdWidget.cpp / .h` エントリを `src/Widget_Osd.cpp / .h` に置換。

### タスク8: 旧 Widgets.h / .cpp の削除とクリーンアップ
    - (作業実行時に追記)

### タスク9: ドキュメントの更新
    - (作業実行時に追記)

## 5. HOTFIX
### 原因・理由: (未発生)
    - 

### 対応: (未対応)
    - 
