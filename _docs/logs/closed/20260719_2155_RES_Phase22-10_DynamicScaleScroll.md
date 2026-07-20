# RES:実装計画・作業レポート Phase 22-10: 物理パラメータに基づく動的間引きスクロールの実装

## 1. 実装目的
TrackDrumエンジン(`Renderer_TrackDrum`)における遠距離ジャンプ時のスロット間引き処理を、INI設定の物理パラメータ(`MaxDuration`, `MaxSpeed`)を用いた動的算出式へ変更し、「限界フリップ数」を算出してインデックス進行のステップ幅をスケールさせる（早送りのようなダイナミック・間引きスクロール）機能を実装する。これにより、UI描画の物理動作と論理的なプレイリスト進行を滑らかに分離・スケーリングさせ、目標への完全同期を実現する。

## 2. アーキテクチャ設計
### 要件1: 物理パラメータからの最大フリップ数（距離）の動的算出
- `TrackDrum::StartDrumAnimation` の引数に物理パラメータ `float maxDuration, float maxSpeed` を追加する。（ConfigManager には直接依存させず、ピュアな数値として受け取る）
- 物理アニメーションの減衰式に基づく終了条件(`0.001f`)から、`maxDuration` 時間内に到達可能な最大物理距離を逆算する。
  算出式: `float maxPhysicalDistance = 0.001f * std::exp(maxSpeed * maxDuration);`
- これを整数化し `int maxFlipCount` として定義する。（最低1を保証する）

### 要件2: 物理アニメーションと論理インデックスの分離とスケーリング
- 実際のジャンプ距離（`relativeDistance`）が `maxFlipCount` を超える場合、物理的な目標位置(`m_drumTargetPosition`)への加算分を `direction * maxFlipCount` にクランプする。
- 物理的な1フリップに対する論理インデックスのステップ幅を `m_indexStep = relativeDistance / 物理移動ベクトル` として算出し、`TrackDrum` クラスのメンバ変数として保持する。

### 要件3: 動的インデックス計算によるPre-fill (0ぴったり更新時)
- `PrefillAbsolute` メソッドにおいて、現在の物理的な絶対位置から目標位置までの残り物理距離(`m_drumTargetPosition - absolutePos`)に `m_indexStep` を乗算し、現在注入すべき論理的な相対インデックスを動的に算出する。
- 着地時には `absolutePos == m_drumTargetPosition` となるため、残り物理距離が0となり、乗算結果も正確に0となる。これにより、目標曲の完全同期が数学的に保証される。

## 3. 実装タスクリスト
- [x] タスク1: `TrackDrum.h` にメンバ変数 `m_indexStep` を追加し、`StartDrumAnimation` のシグネチャを更新
- [x] タスク2: `Renderer.cpp` 側での `StartDrumAnimation` 呼び出し時に、ConfigManagerから取得した物理パラメータ(`MaxDuration`, `MaxSpeed`)を渡すように修正
- [x] タスク3: `Renderer_TrackDrum.cpp` の `StartDrumAnimation` 内部に最大物理距離の逆算ロジックとクランプ処理を追加
- [x] タスク4: `Renderer_TrackDrum.cpp` の `PrefillAbsolute` に `m_indexStep` のスケーリング乗算を適用

## 4. 詳細作業内容
### タスク1: `TrackDrum.h` の改修
- メンバ変数 `float m_indexStep = 1.0f;` の追加。
- `void StartDrumAnimation(int relativeDistance, float maxDuration, float maxSpeed, ...)` へシグネチャ変更。
- **完了:** `Renderer_TrackDrum.h` に `m_indexStep` とシグネチャの変更を適用。

### タスク2: `Renderer.cpp` の改修
- `TrackDrum` を呼び出している箇所(`Renderer_Update.cpp` または関連ファイル)で、`m_trackDrum.StartDrumAnimation(distance, m_config->GetTrackDrumMaxDuration(), m_config->GetTrackDrumMaxSpeed(), ...)` と数値を渡すように修正。
- **完了:** `Application_Playback.cpp`, `Application_Playlist.cpp`, `Application_Render.cpp`, `Application_Initialize.cpp`, `Application_FileDrop.cpp` 等で `StartDrumAnimation` を呼び出す全箇所を更新。

### タスク3: 最大フリップ計算とクランプ (`Renderer_TrackDrum.cpp`)
- `std::exp` を用いた `maxPhysicalDistance` の計算。
- `physicalDistance = std::min(std::abs(relativeDistance), maxFlipCount)` を用いた目標位置のクランプと `m_indexStep` の算出・保持。
- **完了:** `StartDrumAnimation` 内部に `maxPhysicalDistanceF` の計算を追加し、`m_indexStep` を用いたクランプとステップ計算を実装。

### タスク4: Pre-fill 動的計算 (`Renderer_TrackDrum.cpp`)
- `int relativeIndex = static_cast<int>(std::round((m_drumTargetPosition - absolutePos) * m_indexStep));` の計算式を実装し、論理インデックスの動的注入を実現する。
- **完了:** `PrefillAbsolute` メソッドに `relativeIndex` を求めるスケーリング計算式を実装。

### HOTFIX: 物理距離算出式の純化 (2026-07-19)
- `Renderer_TrackDrum.cpp` の `StartDrumAnimation` 内部における `maxFlipCount` 算出式を、`std::exp` を用いたものから `maxDuration * maxSpeed * 60.0f` に基づく純粋なリニア計算に修正。
- `std::max` のマクロ衝突を避けるため `(std::max)` を使用。
