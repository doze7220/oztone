# RES:実装計画・作業レポート Phase 21-9: 相対座標ドラムエンジンとオンデマンド描画

## 1. 実装目的
ドラムエンジンの座標管理を「絶対インデックス」から「目標を 0.0f とした相対距離」へ完全に移行し、描画側が必要なメタデータをプレイリストへ都度問い合わせる（オンデマンド取得）デカップリングアーキテクチャを構築する。過去データを退避させるバケツリレー変数をパージし、シンプルかつ堅牢な設計を目指す。

## 2. アーキテクチャ設計
### 要件1: Rendererの相対座標化とインターフェース改修
    - `src/Renderer.h` および `src/Renderer.cpp` のインターフェースを変更する。
    - 従来の `SetTrackInfo` (またはそれに類するメソッド) を廃止し、新たに `void SetDrumTarget(int relativeDistance)` を定義する。
    - メンバ変数として `float m_drumRelativePosition;` を追加。
    - `SetDrumTarget` メソッド内で、`m_drumRelativePosition += static_cast<float>(relativeDistance);` のように相対距離を加算（目標地点から現在位置を遠ざける・ワープさせる）処理を実装する。

### 要件2: アニメーション目標の 0.0f 固定化
    - `src/Renderer_Update.cpp` の `UpdateAnimation` にて物理演算を修正する。
    - 目標値が常に `0.0f` となるようにし、`m_drumRelativePosition` を `0.0f` に向かって減衰（S字カーブや指数減衰など）させる。

### 要件3: Widget_TrackInfo のオンデマンド描画 (都度聞く)
    - 仮想スロットの描画ループ変数 `i` を「目標地点からの相対インデックス」として扱う。
    - 描画時に `int absIndex = ctx.currentTrackIndex + i;` を計算。
    - これを `ctx.totalTracks` でモジュロ演算して正規化（正の値になるよう調整）。
    - 取得した正規化インデックスを用いて、`ctx.shuffleMetadataList` （またはそれに類するリスト）から直接曲名・アーティスト名・アルバムアート画像を取得する。
    - 以前実装したガラス板描画（ロード中など画像がない場合のフォールバック）や背景同期等の演出ロジックは維持した上で、データ取得元をスナップショットからオンデマンド取得へ差し替える。

### 要件4: Application層からの相対操作の伝達
    - `Application` (例: `Application_Playback.cpp` や関連部分) から `Renderer::SetDrumTarget` を呼び出す際、純粋な「相対距離」を渡すように変更。
    - `Next` 操作時は `relativeDistance = -1`。
    - `Prev` 操作時は `relativeDistance = 1`。
    - `Jump` 操作時は `relativeDistance = oldIndex - newIndex`。
    - `Reset` 時は `relativeDistance = 0`。

### 要件5: 負の遺産の完全パージ
    - `Renderer.h` や `Renderer.cpp` に残存している `m_oldDrumSlot`, `m_nowDrumSlot` などのバケツリレー用状態変数を完全に削除する。

## 3. 実装タスクリスト
[x] タスク1: `Renderer` のインターフェース改修と状態変数整理
    - `Renderer.h` の `SetTrackInfo` 等を `SetDrumTarget(int relativeDistance)` に置換。
    - `m_drumRelativePosition` 追加、`m_oldDrumSlot`/`m_nowDrumSlot` 削除。
    - `Renderer.cpp` の `SetDrumTarget` 実装。
[x] タスク2: `Renderer_Update.cpp` アニメーション演算の修正
    - `m_drumRelativePosition` が常に `0.0f` へ向かうように物理演算を更新。
[x] タスク3: `Widget_TrackInfo` のオンデマンド描画移行
    - スナップショット参照から相対インデックスを用いた `ctx` 経由のメタデータ直接参照への切り替え。インデックス正規化ロジックの実装。
[x] タスク4: `Application` 層の連携修正
    - 再生変更・ジャンプ等に伴う `SetDrumTarget` の呼び出し処理実装。相対距離計算ロジックへの置き換え。

## 4. 詳細作業内容
### タスク1: `Renderer` のインターフェース改修と状態変数整理
    - `Renderer.h` から `SetTrackInfo` を削除し、`SetDrumTarget(int relativeDistance)` を追加。
    - バケツリレー用変数（`m_oldTrackTitle` 等の Track Drum States 関連）を削除。
    - `float m_drumRelativePosition = 0.0f;` を追加。
    - `Renderer.cpp` で `SetDrumTarget` を実装し、`m_drumRelativePosition += static_cast<float>(relativeDistance);` を記述。

### タスク2: `Renderer_Update.cpp` アニメーション演算の修正
    - `UpdateAnimation` にて、`currentTrackIndex` 等を用いた絶対座標ロジックをすべて削除。
    - `m_drumRelativePosition` に対して、毎フレーム `0.0f` に向かって `dampingFactor` (設定から取得した速度ベース) を用いてイージングする処理を実装。
    - 絶対値が `0.001f` 未満になった場合は `0.0f` にスナップさせる処理を追加。

### タスク3: `Widget_TrackInfo` のオンデマンド描画移行
    - `WidgetContext.h` からバケツリレー用の変数群(`oldTrackTitle`, `drumPosition` 等)をパージし、`drumRelativePosition` を追加。
    - `Renderer_Context.cpp` の代入ロジックを修正。
    - `Widget_TrackInfo.h` および `Widget_TrackInfo.cpp` にて、古いキャッシュ変数と生成ロジックを削除。
    - `Widget_TrackInfo.cpp` の `Draw` メソッド内で、相対インデックスを用いた `ctx.shuffleMetadataList` へのオンデマンド参照（モジュロ演算による正規化含む）を実装。
    - クロスフェードやガラス板演出の参照先を相対インデックスベースに置き換え。

### タスク4: `Application` 層の連携修正
    - `Application::PlayCurrentTrack` のシグネチャを `bool PlayCurrentTrack(int relativeDistance = -1);` に変更し、内部で `m_renderer.SetDrumTarget(relativeDistance);` を呼び出すよう修正。
    - 各種操作（Next, Prev, JumpToIndex, SwitchPlaylistなど）に伴う `PlayCurrentTrack` 呼び出し時に、適切な相対距離(`-1`, `1`, `oldIndex - newIndex`) を伝達するよう修正。
    - `ClearPlaylist` や初期起動時のUI空状態対応について、古い `SetTrackInfo` の「"NO TRACK"」文字列表現を廃止し、`m_renderer.SetDrumTarget(0);` を呼び出すよう統一。
    - タスク1から発生していた `SetTrackInfo` 起因のビルドエラーを完全に解消し、Application層に残っていたバケツリレー用の古い変数をパージ。
