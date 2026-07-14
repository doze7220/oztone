# RES:実装計画・作業レポート Phase 21-2: 音量ホバーハイライト対応

## 1. 実装目的
再生コントロールやロゴメニューで実装されている「ホバー時の非対称カラーフェードアニメーション」を、音量コントロールのスピーカーアイコンにも適用し、UIの操作感（アフォーダンス）を統一する。

## 2. アーキテクチャ設計
### 要件1: アニメーション状態の追加 (Widget_VolumeControl)
    - `VolumeControlWidget` クラスにホバーフェード状態を保持するメンバ変数 `m_hoverAlpha` を追加し、ホバーアニメーションの進捗を管理する。

### 要件2: 非対称フェードの更新 (UpdateAnimation)
    - `UpdateAnimation` にて、`ctx.isVolumeHovered` を判定する。
    - ホバー時は即座に `m_hoverAlpha` を1.0fに近づける（または設定速度で高速加算）。
    - 非ホバー時は `ConfigManager::GetHoverFadeOutSpeed()` を利用して減衰させる非対称フェードロジックを実装し、UI離脱ディレイの統一感を保つ。

### 要件3: カラーブレンド（Lerp）の適用 (Draw)
    - `Draw` にて、ベースカラー（白等の基本色）と `ConfigManager::GetHoverIconColor()` の色を `m_hoverAlpha` を用いて線形補間（Lerp）する。
    - ブレンドした色を `m_controlBrush->SetColor` で適用し、スピーカーアイコン等の描画色を動的に変更する。既存のツールチップ等の機能には影響を与えないように独立して管理する。

## 3. 実装タスクリスト
[x] タスク1: VolumeControlWidgetへの変数追加
    - `src/Widget_VolumeControl.h` に `float m_hoverAlpha = 0.0f;` を追加する。
[x] タスク2: UpdateAnimationの改修
    - `src/Widget_VolumeControl.cpp` の `UpdateAnimation` メソッドに非対称フェード処理を追加する。
[x] タスク3: Drawメソッドの改修
    - `src/Widget_VolumeControl.cpp` の `Draw` メソッド内で色をLerpで補間し、`m_controlBrush` にセットして描画するロジックを追加する。
[x] タスク4: アーキテクチャ資料の更新
    - `PROJECT_ARCHITECTURE.md` にホバー色合成対応について追記する。

## 4. 詳細作業内容
### タスク1: VolumeControlWidgetへの変数追加
    - `Widget_VolumeControl.h`の`VolumeControlWidget`クラスのprivateメンバに`float m_hoverAlpha = 0.0f;`を追加し、ホバーアニメーション状態を保持できるようにした。
### タスク2: UpdateAnimationの改修
    - `UpdateAnimation`において、ホバー時は`fadeInSpeed`（10.0f）を利用して高速に加算し、非ホバー時は`ctx.config->GetHoverFadeOutSpeed()`に基づいた速度で減算する非対称フェードロジックを実装した。
### タスク3: Drawメソッドの改修
    - `Draw`において、描画直前に基本色と`ConfigManager::GetHoverIconColor()`を`m_hoverAlpha`の比率で線形補間（Lerp）する処理を記述した。
    - 補間した色を`m_controlBrush->SetColor`で適用し、スピーカーアイコンやボリュームゲージ、パーセンテージテキストが動的に色変化するようにした。既存のツールチップには影響しないカプセル化を保っている。
### タスク4: アーキテクチャ資料の更新
    - `PROJECT_ARCHITECTURE.md` の `VolumeControlWidget` の記述を更新し、スピーカーアイコンに対しても再生コントロール等と同様にホバー時の非対称カラーフェードアニメーションによる色合成（`HoverIconColor` を用いた Lerp 補間）が適用され、UI全体の手触り（アフォーダンス）が統一された旨を追記した。
