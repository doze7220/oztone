# RES:実装計画・作業レポート Phase 21-10: TrackDrumエンジンの独立クラス化とファイル分割

## 1. 実装目的
現在 `Renderer` クラス内部に混在しているドラムアニメーション関連の変数とメソッドを完全に独立した `TrackDrum` クラスとしてカプセル化し、物理ファイルとしても分離する。これにより、`Renderer` のコードをスリム化し、AIのコンテキスト節約と今後の拡張性（背景マネージャの導入等）の基盤を整える。
※直近のフェイズで完成した「コールバック駆動」「イベント駆動」「ダブルバッファリング」の計算式やデータフローのロジックは一切変更せず、純粋なリファクタリングのみを行う。

## 2. アーキテクチャ設計
### 要件1: 新規ファイルの作成とクラス定義
    - `src/Renderer_TrackDrum.h` および `src/Renderer_TrackDrum.cpp` を新規作成する。
    - `Renderer_TrackDrum.h` 内に、完全に独立した `class TrackDrum` を定義する。

### 要件2: Rendererからの変数・メソッドの完全移植
    - `Renderer` クラスが持つドラム関連変数（`m_drumSlots`, `m_currentDrumSlotIndex`, `m_drumRelativePosition`, `m_animatingTargetIndex`, `m_animatingOldIndexOffset`, `m_drumDataProvider`, `m_drumOnComplete` 等）を `TrackDrum` クラスのprivateメンバへ移動する。
    - 関連するメソッド（`StartDrumAnimation`, `UpdateAnimation` 内のドラム計算部分, `OnSlotAnimationCompleted`, `StepDrumSlot` 等）もすべて `TrackDrum` のメソッドとして移植する。

### 要件3: Renderer と外部のインターフェース調整
    - `Renderer` クラスに `#include "Renderer_TrackDrum.h"` を追加し、メンバ変数として `TrackDrum m_trackDrum;` を保持させる。
    - 外部（Application）からの呼び出しは `Renderer` の委譲メソッドを経由するか、ゲッター経由で `TrackDrum` を操作するように修正する。
    - `WidgetContext` を構築する際、`m_trackDrum` からドラムの状態を取得し、Widgetへ渡すように配線を繋ぎ直す。

### 要件4: ビルド設定の更新
    - `CMakeLists.txt` のソースリストに新規ファイル（`src/Renderer_TrackDrum.cpp`）を追加する。

## 3. 実装タスクリスト
[x] タスク1: 新規ファイル作成とTrackDrumクラスの基盤定義
    - `src/Renderer_TrackDrum.h` と `src/Renderer_TrackDrum.cpp` を作成し、インクルードガードや基本構造を記述する。
[x] タスク2: 既存変数の移植 (Renderer -> TrackDrum)
    - `Renderer` からドラム関連のメンバ変数を切り離し、`TrackDrum` クラスへ移動する。
[x] タスク3: 既存メソッドの移植 (Renderer -> TrackDrum)
    - `Renderer` からドラム計算・制御メソッド群（`StartDrumAnimation`など）を `TrackDrum` へ移動・適応させる。
[x] タスク4: RendererとTrackDrum間のインターフェース調整
    - `Renderer` に `TrackDrum` のインスタンスを持たせ、委譲メソッド・ゲッター等のアクセス口を整備する。
[x] タスク5: WidgetContextおよび周辺の配線修正
    - 描画コンテキスト構築時など、ドラム状態の参照元を `TrackDrum` へ変更する。
[ ] タスク6: ビルド設定の更新
    - `CMakeLists.txt` に `src/Renderer_TrackDrum.cpp` を追加する。
[ ] タスク7: ドキュメント更新
    - `PROJECT_ARCHITECTURE.md` の記述を更新し、TrackDrumクラスの分離について記載する。

## 4. 詳細作業内容
### タスク1: 新規ファイル作成とTrackDrumクラスの基盤定義
    - `src/Renderer_TrackDrum.h` および `src/Renderer_TrackDrum.cpp` を新規作成し、独立した `TrackDrum` クラスの枠組みを定義した。

### タスク2: 既存変数の移植 (Renderer -> TrackDrum)
    - `Renderer.h` から `m_drumSlots`, `m_currentDrumSlotIndex`, `m_drumRelativePosition`, `m_animatingTargetIndex`, `m_animatingOldIndexOffset`, `m_drumDataProvider`, `m_drumOnComplete` を削除した。
    - `Renderer_TrackDrum.h` に必要なインクルード（`WidgetContext.h`, `<array>`, `<functional>`）を追加し、上記の変数を `TrackDrum` の private メンバとして移植し、インライン初期化を記述した。

### タスク3: 既存メソッドの移植 (Renderer -> TrackDrum)
    - `src/Renderer.h` および `src/Renderer.cpp` から `StartDrumAnimation`, `OnSlotAnimationCompleted`, `SetAlbumArt`, `UpdateCurrentDrumSlot`, `IsDrumAnimating` の宣言・実装を削除した。
    - `src/Renderer_Update.cpp` の `UpdateAnimation` 内の物理演算・境界判定ロジックを切り取り、`TrackDrum::Update` の実装とした。
    - 削除・切り取ったメソッドを `src/Renderer_TrackDrum.h` および `src/Renderer_TrackDrum.cpp` に `TrackDrum` クラスのメソッドとして定義・実装し、`ConfigManager*` の渡し方などを調整した。ロジック自体は変更せずに移動を完了した。

### タスク4: RendererとTrackDrum間のインターフェース調整
    - `Renderer.h` に `TrackDrum m_trackDrum;` と `GetTrackDrum()` ゲッターを追加した。
    - `Renderer_Update.cpp` 内の `UpdateAnimation` にて `m_trackDrum.Update()` を呼び出すように配線した。
    - 各 `Application_*.cpp` から `m_renderer.StartDrumAnimation` や `IsDrumAnimating` を呼び出していた箇所を `m_renderer.GetTrackDrum().StartDrumAnimation(...)` 等へ修正した。

### タスク5: WidgetContextおよび周辺の配線修正
    - `Renderer_TrackDrum.h` の `TrackDrum` クラスに、各ドラム状態(`m_drumSlots`, `m_currentDrumSlotIndex`, `m_drumRelativePosition`, `m_animatingTargetIndex`, `m_animatingOldIndexOffset`)を取得するためのゲッターメソッドを追加した。
    - `Renderer_Context.cpp` における各種コンテキスト構築処理(`BuildAnimationContext`, `BuildLayoutContext`, `BuildRenderContext`)において、ドラムの状態を `Renderer` のメンバ変数ではなく、`m_trackDrum` からゲッターを経由して取得し、`WidgetContext` に代入するように修正した。

### タスク6: ビルド設定の更新
    - (未実施)

### タスク7: ドキュメント更新
    - (未実施)


