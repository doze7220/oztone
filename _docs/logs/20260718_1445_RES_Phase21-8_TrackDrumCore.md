# RES:実装計画・作業レポート Phase 21-8: トラックドラム (Track Drum) コアエンジンの実装

## 1. 実装目的
曲の切り替え時（メディアキー等による Next / Prev 時など）に、物理的な「質量」と「慣性」を持ったドラムが回転するトランジションUI（トラックドラム）のコアエンジンを実装する。本フェーズではマウス操作等は含まず、「描画とバケツリレーの物理演算」のみにスコープを限定する。
INI設定で無効化された場合は従来通りの即時切り替えを完全に保証し、またアーキテクチャの原則に則り、背景アートとの完全同期および非同期画像ロードの責務（Application層への委譲）を厳格に守る設計とする。

## 2. アーキテクチャ設計
### 要件1: ConfigManagerの拡張
    - INIファイルに `[TrackDrum]` セクションを新設する。
    - `EnableTrackDrum` (デフォルト: 1), `MaxDuration` (3.0), `MinSpeed`/`MaxSpeed` (10.0/60.0), `Acceleration`/`Deceleration` (20.0/15.0) の設定管理を追加し、デフォルト値を定義する。

### 要件2: トラックドラムの描画とバケツリレー管理
    - `Widget_TrackInfo` 内に、現在位置・速度・目標インデックス・アニメーション進行度などの状態をもたせ、「消えゆく曲（OLD）」と「現れる曲（NOW）」の2つの情報セットを用いてバケツリレー形式で状態を更新する。
    - 速度は `Acceleration` / `Deceleration` を用いて計算し、S字カーブ等で制御する。
    - 目標までの距離が遠い場合は、`MaxDuration` 制約に基づき間のインデックスを間引き（スキップ）する。
    - 高速移動中（着地前）は正規の重い画像デコードをスキップし、半透明のガラス板（`FallbackArtOpacity` の流用など）を描画する。着地した瞬間にクロスフェードで出現させる。

### 要件3: 背景アートとの完全同期
    - トラックドラムの回転アニメーション（バケツリレーおよびクロスフェード）を、左下の `TrackInfoWidget` だけでなく背景アート（`DrawBackground`）とも完全に同期させる。
    - 背景描画の責務を `Widget_TrackInfo` に統合・吸収するか、あるいは `Renderer` のコンテキスト（`WidgetContext` 等）でトラックドラムの進行度合い（現在のアニメーション状態・ブレンド率・OLD/NOWのアート情報）を一括管理し、Renderer側で背景を同期描画する設計に修正する。

### 要件4: 非同期画像ロードの責務厳守
    - `Widget_TrackInfo` などのUIコンポーネント内で独自にスレッド（`std::thread`, `std::async`）を立てて画像ファイルI/Oを行うことは絶対に行わない。
    - 画像のロード処理は `Application` 層の先読み機構、または `TrackDatabase` / `TrackAnalyzer` へと委譲する。
    - Widget側は「Application層から画像が渡されるまでガラス板を描画して待つ」という受動的な設計とする。画像のロード指示やデータ連携が必要な場合は `Application_*.cpp` 側でハンドリングする機構を追加・調整する。

### 要件5: シャッフル時および既存操作の安全設計
    - 静止時にシャッフル状態が切り替わった場合は表示（ドラム）を動かさず、裏のリストのみ更新する。
    - アニメーション中にシャッフル状態が変更された場合は、OLD/NOWを現在のアニメーションのまま一旦着地させ、完了後に新リストの情報を引き込む安全な状態遷移を行う。
    - `EnableTrackDrum == 0` の場合、全アニメーションロジックをバイパスし、即座に次の曲を描画する従来の挙動を完全に維持する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManagerの拡張
    - `ConfigManager.h`, `ConfigManager.cpp`, `ConfigManager_DefaultIni.h` に `[TrackDrum]` セクションと関連パラメータ群を追加する。
[x] タスク2: Renderer と Widget_TrackInfo への状態変数・物理演算の追加 (同期管理設計)
    - `Widget_TrackInfo.h/cpp` または `Renderer_Context.cpp` に、ドラムの進行状態、速度、OLD/NOWのアート・テキスト情報を保持する機構を実装する。
    - バケツリレー方式での情報更新と、`EnableTrackDrum` に従うフォールバックロジック（即時切り替え）を実装する。
[x] タスク3: 遠距離ジャンプのスキップ処理の実装
    - `MaxDuration` に基づいて、遠距離移動時に中間のインデックスを間引くスキップ処理ロジックを実装する。
[x] タスク4: 非同期ロードのApplication層への委譲とフォールバック描画の実装
    - `Application_Playback.cpp` (または関連ファイル) にて、トラックドラムの目標着地（または先読み）に合わせて画像のロードを指示・通知する処理を追加/連携させる。
    - `Widget_TrackInfo` および背景描画において、画像未ロード時や高速移動中は半透明のガラス板を描画し、着地時に画像が利用可能になり次第クロスフェードさせる描画処理を実装する。
    - UIコンポーネント内からの一切の独自スレッド起動を排除する。
[ ] タスク5: 背景アートとの完全同期描画の実装
    - `Renderer_Draw.cpp` などの背景描画処理にて、トラックドラムのブレンド率・OLD/NOWアート情報に完全同期して背景をフェード・スライドさせる処理を実装する（または描画責務を再編する）。
[ ] タスク6: シャッフル状態変更時の安全な状態遷移の実装
    - 静止中・アニメーション中のシャッフル状態変更イベントを適切にハンドリングし、表示の破綻を防ぐロジックを追加する。
[ ] タスク7: PROJECT_ARCHITECTURE.md の更新
    - アーキテクチャ図や説明におけるトラックドラム機能に関する概要、および背景アートの同期や非同期ロードの責務（Application層との連携）について追記する。

## 4. 詳細作業内容
### タスク1: ConfigManagerの拡張
    - `ConfigManager_DefaultIni.h` に `[TrackDrum]` セクションのデフォルト設定を追加。
    - `ConfigManager.h` に関連パラメータのゲッターとメンバ変数を追加。
    - `ConfigManager_Playback.cpp` の `LoadPlaybackSettings` メソッド内に `[TrackDrum]` 設定の読み込み処理を実装。
    - 既存の `Widget_TrackInfo` や `Renderer` には一切変更を加えずにスコープを遵守。
### タスク2: Renderer と Widget_TrackInfo への状態変数・物理演算の追加 (同期管理設計)
    - `Renderer.h` に `m_oldTrackTitle`, `m_oldTrackArtist`, `m_oldArtBitmap`, `m_isDrumAnimating`, `m_drumPosition`, `m_drumVelocity`, `m_drumTargetIndex` の状態変数を追加。
    - `WidgetContext.h` にも同様にアニメーション状態やOLD情報を伝播させるための変数を追加し、`Renderer_Context.cpp` の各種ビルド関数で引き渡すよう修正。
    - `Renderer::SetTrackInfo` にて、新しい曲がセットされた際に現在の情報をOLDに退避し、`m_isDrumAnimating` を有効にするバケツリレー処理を実装（初回起動時は除く）。
    - `Renderer::UpdateAnimation` にて、`ConfigManager` の `TrackDrumAcceleration`, `TrackDrumDeceleration`, `TrackDrumMaxSpeed` を用いた速度と位置の更新（物理演算ロジック）を実装。
    - `EnableTrackDrum` が 0（無効）の場合はアニメーション状態を強制解除し、即時にNOWのインデックスへ位置をスナップするフォールバックロジックを実装。
### タスク3: 遠距離ジャンプのスキップ処理の実装
    - `Renderer::UpdateAnimation` にて、目標となる `m_drumTargetIndex` と現在の `m_drumPosition` の距離（スロット数）を算出する処理を追加。
    - `ConfigManager` の `TrackDrumMaxSpeed` と `TrackDrumMaxDuration` を掛け合わせ、「物理的に表示可能な最大スロット数」を計算。
    - 算出した距離が最大スロット数を超える場合、目標座標から最大スロット数分離れた位置に `m_drumPosition` をワープさせることで仮想的に中間をスキップし、必ず `MaxDuration` 秒以内にアニメーションが完了する補正ロジックを実装。
### タスク4: 非同期ロードのApplication層への委譲とフォールバック描画の実装
    - `Application.h`/`Application_Playback.cpp` に `LoadCurrentTrackArtAsync` を実装し、非同期で画像をロードする機構を追加。
    - `Application_Render.cpp` の `ForceRender` 内でトラックドラムの着地（`m_isDrumAnimating` が `true` から `false` に変わった瞬間）を検知し、`LoadCurrentTrackArtAsync` をトリガーするように修正。
    - `Widget_TrackInfo.cpp` にて、アニメーション中はガラス板を描画し、画像がロードされたらクロスフェードするロジックを実装。
    - `m_drumStartIndex` の管理を `Renderer` および `WidgetContext` に追加し、OLDとNOWのオフセット座標を正確に計算するロジックを実装。
### タスク5: 背景アートとの完全同期描画の実装
    - 未着手
### タスク6: シャッフル状態変更時の安全な状態遷移の実装
    - 未着手
### タスク7: PROJECT_ARCHITECTURE.md の更新
    - 未着手
