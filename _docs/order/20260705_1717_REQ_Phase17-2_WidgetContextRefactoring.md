### 作業指示書 REQ: Phase 17-2: ユーティリティの分離 (計画立案)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase17-2_WidgetContextRefactoring.md`）として新規作成すること。
3. レポートのフォーマットは、`PROJECT_CONSTITUTION.md` の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
現在 `src/Widget.h` に同居している `IWidget` インターフェースと `WidgetContext` 構造体を分離し、単一責任の原則に従ってユーティリティを独立させるリファクタリングを実行する。機能の追加やロジックの変更は一切行わないこと。

* **要件1: 新規ファイルの作成と構造体の移行**
  * `src/WidgetContext.h` (新規作成)
  * 現在 `src/Widget.h` に定義されている `WidgetContext` 構造体の宣言を `WidgetContext.h` へ完全に移動する。
  * `WidgetContext` が依存している標準ライブラリや Direct2D 等のヘッダ、およびメタデータ構造体(`TrackMetadata`等)のインクルードを `WidgetContext.h` 内で過不足なく行うこと。
* **要件2: 既存ファイルの依存関係の整理**
  * `src/Widget.h` の先頭で `#include "WidgetContext.h"` を行い、`IWidget` インターフェースが `WidgetContext` を引き続き参照できるようにする。
  * `src/Widgets.h` や `src/PlaylistWidget.h`、その他 `WidgetContext` を直接利用しているソースファイルにおいて、インクルードエラーが発生しないよう依存関係を整理する。
* **要件3: ビルド設定の更新 (該当する場合)**
  * `CMakeLists.txt` を確認し、ヘッダファイルの追加が必要な構成であれば `src/WidgetContext.h` を追加する。

#### 【作業終了後】
1. 作業レポート（`...RES_Phase17-2_WidgetContextRefactoring.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **純粋なリファクタリングの徹底**: 本作業はファイルの分割のみを目的とする。変数の追加、関数のシグネチャ変更、UIのレイアウトや描画ロジックの変更は **絶対に行ってはならない**。
