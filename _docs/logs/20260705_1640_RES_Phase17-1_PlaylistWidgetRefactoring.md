# RES:実装計画・作業レポート Phase 17-1: PlaylistWidgetの分離・独立化

## 1. 実装目的
現在 `Widgets.h` および `Widgets.cpp` に実装されている巨大な `PlaylistWidget` クラスを完全に独立したファイル（`PlaylistWidget.h`, `PlaylistWidget.cpp`）へと切り出すリファクタリングを実行し、コードの可読性とメンテナンス性を向上させる。機能の追加や既存ロジックの変更は一切行わない。

## 2. アーキテクチャ設計
### 要件1: 新規ファイルの作成とコードの移行
    - `src/PlaylistWidget.h` および `src/PlaylistWidget.cpp` を新規作成する。
    - `Widgets.h` 内の `PlaylistWidget` クラス宣言を `PlaylistWidget.h` へ移動する。
    - `Widgets.cpp` 内の `PlaylistWidget` クラスの実装（`CreateResources`, `UpdateAnimation`, `UpdateLayout`, `Draw` 等）を `PlaylistWidget.cpp` へ移動する。

### 要件2: 依存関係の整理
    - `PlaylistWidget.h` に `Widget.h` や必要なインクルード（`<string>`, `<vector>`, `<d2d1_1.h>`, `<dwrite_1.h>` 等）を過不足なく追加する。
    - `Renderer.cpp` 等の呼び出し元に `#include "PlaylistWidget.h"` を追加し、コンパイルエラーを防ぐ。

### 要件3: ビルド設定の更新
    - `CMakeLists.txt` のソースリストに新規作成した `src/PlaylistWidget.h` および `src/PlaylistWidget.cpp` を追加する。

## 3. 実装タスクリスト
[ ] タスク1: 新規ファイルの作成とコードの移行
    - `src/PlaylistWidget.h` を作成し、`Widgets.h` からクラス宣言を移植する。
    - `src/PlaylistWidget.cpp` を作成し、`Widgets.cpp` からメソッド実装を移植する。
[ ] タスク2: 依存関係の解決
    - `PlaylistWidget.h` / `PlaylistWidget.cpp` のインクルードパスを整備する。
    - `Renderer.cpp` および必要に応じて他の関連ファイルに `#include "PlaylistWidget.h"` を追加する。
[ ] タスク3: ビルド設定の更新
    - `CMakeLists.txt` を修正し、新しいソースファイルをビルドターゲットに追加する。
[ ] タスク4: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` の「各具象 Widget 実装 (`Widgets.h/cpp`)」等の記載を修正し、`PlaylistWidget` が独立したファイルにあることを明記する。

## 4. 詳細作業内容
### タスク1: 新規ファイルの作成とコードの移行
    - 未実施

### タスク2: 依存関係の解決
    - 未実施

### タスク3: ビルド設定の更新
    - 未実施

### タスク4: ドキュメントの更新
    - 未実施
