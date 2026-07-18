### 作業指示書 REQ: Phase 21-10: TrackDrumエンジンの独立クラス化とファイル分割
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-10_TrackDrumRefactoring.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
現在 `Renderer` クラス内部に混在しているドラムアニメーション関連の変数とメソッドを完全に独立した `TrackDrum` クラスとしてカプセル化し、物理ファイルとしても分離する。これにより、`Renderer` のコードをスリム化し、AIのコンテキスト節約と今後の拡張性（背景マネージャの導入等）の基盤を整える。

*   **要件1: 新規ファイルの作成とクラス定義**
    *   `src/Renderer_TrackDrum.h` および `src/Renderer_TrackDrum.cpp` を新規作成する。
    *   `Renderer_TrackDrum.h` 内に、完全に独立した `class TrackDrum` を定義する。
*   **要件2: Rendererからの変数・メソッドの完全移植**
    *   現在 `Renderer` が持っているドラム関連の変数（`m_drumSlots`, `m_currentDrumSlotIndex`, `m_drumRelativePosition`, `m_animatingTargetIndex`, `m_animatingOldIndexOffset`, `m_drumDataProvider`, `m_drumOnComplete` 等）をすべて `TrackDrum` クラスの private メンバへ移動する。
    *   関連するメソッド（`StartDrumAnimation`, `UpdateAnimation` 内のドラム計算部分, `OnSlotAnimationCompleted`, `StepDrumSlot` 等）もすべて `TrackDrum` のメソッドとして移植する。
*   **要件3: Renderer と外部のインターフェース調整**
    *   `Renderer` クラスは `#include "Renderer_TrackDrum.h"` を行い、メンバ変数として `TrackDrum m_trackDrum;` を保持するように修正する。
    *   外部（Application）からの呼び出しは `Renderer` の委譲メソッドを経由するか、ゲッター経由で `TrackDrum` を操作するように修正する。
    *   `WidgetContext` を構築する際、`m_trackDrum` からドラムの状態を取得し、Widgetへ渡すように配線を繋ぎ直す。
*   **要件4: ビルド設定の更新**
    *   `CMakeLists.txt` のソースリストに新規ファイルを追加する。

#### 【作業終了後】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-10_TrackDrumRefactoring.md）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
*   **ロジック変更の禁止**: 本タスクは純粋なリファクタリング（設計変更）である。直近のフェイズで完成した「コールバック駆動」「イベント駆動」「ダブルバッファリング」の計算式やデータフローのロジックは **1ミリも変更せず**、そのままクラス化すること。
*   **スコープの厳守**: 本フェイズでは「背景アートの分離」等は行わない。純粋にドラムエンジンのファイル切り出しとクラス化にのみ専念すること。

-----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-10 Task 1 : 新規ファイル作成とTrackDrumクラスの基盤定義
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは新規ファイルの作成とクラス定義である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク1(Phase 21-10)が完了しました。タスク2の指示をお願いします」と報告すること。

#### 【実装要件】
ドラムアニメーション機構をカプセル化するための独立したクラスとファイルを新規作成する。本タスクでは枠組みの作成のみを行い、Rendererからの移植はまだ行わない。

*   **要件1: 新規ファイルの作成**
    *   `src/Renderer_TrackDrum.h` および `src/Renderer_TrackDrum.cpp` を新規作成する。
*   **要件2: クラスの基盤定義**
    *   `Renderer_TrackDrum.h` 内に `#pragma once` を記述し、完全に独立した `class TrackDrum` の定義（空のクラス枠）を作成する。
    *   `Renderer_TrackDrum.cpp` 内に `#include "Renderer_TrackDrum.h"` を記述する。

#### 【絶対遵守ルール (Constraints)】
*   **移植の禁止**: 本タスクではまだ `Renderer` クラスからの変数やメソッドの移動を行わない。純粋にファイルと空のクラス枠を作成することのみに専念すること。

-----------------------------------------------------------------------------------

