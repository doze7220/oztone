# RES:実装計画・作業レポート Phase 19-4: ヘイローダスト(Halo Dust)の究極ブラッシュアップ

## 1. 実装目的
円形パーティクル・ビジュアライザ（`Visualizer_HaloDust.cpp`）をスケールフリー化＆物理シミュレーション化し、究極のブラッシュアップを行う。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の拡張 (INIによるスケール・グロー制御)
    - `HaloLaserLengthRatio`, `HaloParticleSizeRatio`, `HaloGlowOpacity`, `HaloGlowThickness` を追加。

### 要件2: パーティクル構造体の物理エンジン化
    - `Particle` および `LaserRay` 構造体に `lifeTime`, `maxLifeTime`, `acceleration` を追加。

### 要件3: 命の軌跡（VFX）ロジックの組み込み
    - 加速度による成長と減速、およびサイン波を用いたフェードイン/アウトを実装。

### 要件4: テーマカラーの「青・緑」シフト
    - 生成されたベースカラーのRGB成分を入れ替える（桁ずらし）ことで、サイバーテイストな青・緑系の発色へとシフト。

### 要件5: スケールフリー化の適用
    - `ConfigManager` の各比率プロパティとウィンドウ矩形から描画サイズを動的に計算する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManager の拡張
[x] タスク2: パーティクル構造体の物理エンジン化
[x] タスク3: 命の軌跡（VFX）ロジックの組み込み
[x] タスク4: テーマカラーの「青・緑」シフト
[x] タスク5: スケールフリー化の適用

## 4. 詳細作業内容
### タスク1: ConfigManager の拡張
    - `src/ConfigManager.h` および `src/ConfigManager.cpp` を修正。ゲッター・セッター、初期値設定、INIファイル入出力を実装。

### タスク2: パーティクル構造体の物理エンジン化
    - `src/Visualizer_HaloDust.h` 内の構造体定義を更新。新規生成時にランダムな加速度・寿命を設定するように変更。

### タスク3: 命の軌跡（VFX）ロジックの組み込み
    - `src/Visualizer_HaloDust.cpp` にて、`lifeTime`の増分に基づくイージングロジック（`progress`）を導入。
    - progress > 0.5f 以降のマイナス加速度適用や、`sin()` 計算による透明度およびサイズの遷移を実装。

### タスク4: テーマカラーの「青・緑」シフト
    - `src/Visualizer_HaloDust.cpp` の色算出部で、元々算出された R, G, B 値を B, R, G へマッピングし直すことで、色相分布を青・緑寄りへシフト。

### タスク5: スケールフリー化の適用
    - レーザー長やパーティクルサイズに `m_config` の値を使用し、リサイズ時にも美しいレイアウトが保持されるようスケーリング。
    - グローの太さと透明度もコンフィグと動的に連動。

## 5. Hotfix: 追加のVFXパラメータと2パスレーザー描画の完全対応
本タスクの追加要件として、さらなるパラメータのINI対応とレーザー描画処理の改修を実施。

### Hotfixタスクリスト
[x] タスク6: ConfigManagerへの追加パラメータ対応（Thickness, Speed, SpawnRate, LifeTime）
[x] タスク7: レーザーおよびパーティクルの生成ロジックへINI設定値の適用
[x] タスク8: レーザー描画の2パス（コア・グロー）描画ロジックの組み込み

### Hotfix詳細作業内容
#### タスク6: ConfigManagerへの追加パラメータ対応
    - `HaloLaserThickness`, `HaloLaserSpeed`, `HaloLaserSpawnRate`, `HaloLaserLifeTime`, `HaloParticleSpeed`, `HaloParticleSpawnRate`, `HaloParticleLifeTime` を追加。
    - 各パラメータのデフォルト設定とINIファイル入出力ロジックを実装。

#### タスク7: 生成ロジックへのINI設定値の適用
    - レーザーおよびパーティクルの `SpawnRate` を発生判定に直接適用。
    - 生成時の `lifeTime`, `maxLifeTime`, `acceleration` 算出にINIの `Speed`, `LifeTime` プロパティを反映。

#### タスク8: レーザーの2パス描画
    - 矩形の塗りつぶし（FillRectangle）から、`DrawLine` を用いたコア（白/テーマカラー）とグロー（太く半透明な線）の2パス描画ロジックへと改修し、円本体と同等のネオングロー発光エフェクトを適用。
