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

### 作業指示書 REQ: Phase 21-10 Task 2 : 既存変数の移植 (Renderer -> TrackDrum)
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはRendererからTrackDrumへのメンバ変数の移植である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク2(Phase 21-10)が完了しました。タスク3の指示をお願いします」と報告すること。

#### 【実装要件】
現在 `Renderer` クラスが持っているドラム関連の変数をすべて `TrackDrum` クラスの private メンバへ移動する。

*   **要件1: Renderer.h からの変数削除**
    *   `src/Renderer.h` の private メンバから以下の変数を削除（パージ）する。
        *   `std::array<DrumSlot, 2> m_drumSlots;`
        *   `int m_currentDrumSlotIndex;`
        *   `float m_drumRelativePosition;`
        *   `size_t m_animatingTargetIndex;`
        *   `int m_animatingOldIndexOffset;`
        *   `std::function<TrackMetadata(size_t)> m_drumDataProvider;`
        *   `std::function<void()> m_drumOnComplete;`
        *   その他、純粋にドラムの状態管理にのみ使われている変数が残っていれば合わせて削除する。
*   **要件2: Renderer_TrackDrum.h への変数追加と初期化**
    *   `src/Renderer_TrackDrum.h` に、変数定義に必要なインクルード（`#include "WidgetContext.h"`, `<array>`, `<functional>` 等）を追加する。
    *   `class TrackDrum` の private セクションに、要件1で削除した変数群をそのままの型と名前で追加する。
    *   `TrackDrum` のコンストラクタを定義し、各変数が安全な初期値（0やnullptrなど）を持つように初期化リストまたはインライン初期化を記述する。

#### 【絶対遵守ルール (Constraints)】
*   **メソッド移植の禁止**: 本タスクでは変数の移動のみに専念すること。既存の `Renderer.cpp` や `Renderer_Update.cpp` 内で変数が未定義になることによる大量のコンパイルエラーが発生するが、次タスクでメソッドごと移動して解決するため、今は一切気にせずエラーを放置してよい。

-----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-10 Task 3 : 既存メソッドの移植 (Renderer -> TrackDrum)
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはRendererからTrackDrumへのメソッドの移植である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md）の「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク3(Phase 21-10)が完了しました。タスク4の指示をお願いします」と報告すること。

#### 【実装要件】
現在 `Renderer` クラス内に存在するドラムアニメーション制御・更新メソッド群を、`TrackDrum` クラスへ移動する。

*   **要件1: Rendererからのメソッド削除と分離**
    *   `src/Renderer.h` から `StartDrumAnimation`, `OnSlotAnimationCompleted`, `StepDrumSlot` 等のドラム専用メソッドの宣言を削除する。
    *   `src/Renderer.cpp` から `StartDrumAnimation`, `OnSlotAnimationCompleted`, `StepDrumSlot` の実装を削除する。
    *   `src/Renderer_Update.cpp` の `UpdateAnimation` 内部から、ドラムの相対距離の計算および境界判定（`OnSlotAnimationCompleted` 呼び出し）を行っている物理演算ロジックのブロックを切り取る。
*   **要件2: TrackDrumへのメソッド定義と実装**
    *   `src/Renderer_TrackDrum.h` の `TrackDrum` クラスの public セクションに `StartAnimation` (または `StartDrumAnimation`), `Update` を定義し、private セクションに `OnSlotAnimationCompleted`, `StepDrumSlot` 等の内部制御メソッドを定義する。
    *   `src/Renderer_TrackDrum.cpp` に、要件1で切り取った各メソッドの実装を移植する（`TrackDrum::` のスコープをつける）。
*   **要件3: TrackDrum内部での完結化**
    *   移植したメソッド内において、`m_drumSlots` などのメンバ変数へのアクセスが `TrackDrum` のスコープ内で完結するように整備する。必要であれば `TrackDrum.cpp` 内に不足しているインクルード（`<algorithm>`や`<cmath>`等）を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **ロジック変更の禁止**: 移植する物理演算、状態判定、イベント呼び出しのロジック（計算式やIF文の条件など）は **1ミリも変更せず** そのまま移動させること。
*   **コンパイルエラーの放置**: メソッドを移動したことにより、`Application` などの呼び出し元で「メソッドが見つからない」というコンパイルエラーが発生するが、これは次のタスク（インターフェース調整）で解決するため、今は一切気にせずエラーを放置してよい。

-----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-10 Task 4 : RendererとTrackDrum間のインターフェース調整
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはRendererとTrackDrumのインターフェース調整である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260719_0203_RES_Phase21-10_TrackDrumRefactoring.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク4(Phase 21-10)が完了しました。タスク5の指示をお願いします」と報告すること。

#### 【実装要件】
分離した `TrackDrum` クラスを `Renderer` の部品として組み込み、外部（ApplicationやRenderer_Update）からアクセスできるように配線を繋ぐ。

*   **要件1: Renderer.h への組み込み**
    *   `src/Renderer.h` に `#include "Renderer_TrackDrum.h"` を追加する。
    *   `Renderer` クラスの private メンバに `TrackDrum m_trackDrum;` を追加する。
    *   public メンバに `TrackDrum& GetTrackDrum() { return m_trackDrum; }` などのゲッターを追加し、外部からドラムエンジンへアクセスできるようにする。
*   **要件2: Renderer_Update.cpp の修正**
    *   `src/Renderer_Update.cpp` の `UpdateAnimation` メソッド内部にて、旧来ドラムの物理演算ロジックがあった場所で `m_trackDrum.Update();` を呼び出すように配線する。
*   **要件3: Application層からの呼び出し修正**
    *   `src/Application_Playback.cpp` 等で曲を切り替える際、`m_renderer.StartDrumAnimation(...)` と呼び出していた箇所を、要件1で作成したゲッターを経由して `m_renderer.GetTrackDrum().StartAnimation(...)` （またはTrackDrum側のメソッド名）を呼び出すように修正する。

#### 【絶対遵守ルール (Constraints)】
*   **WidgetContextの修正は次タスク**: 本タスクでは `WidgetContext` を組み立てる `Renderer_Context.cpp` などの修正は行わない。`WidgetContext` に関連するコンパイルエラーが残るはずだが、それは次タスクで解決するため今は放置してよい。

-----------------------------------------------------------------------------------
