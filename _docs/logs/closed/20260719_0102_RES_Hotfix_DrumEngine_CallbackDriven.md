# RES_Hotfix_DrumEngine_CallbackDriven

## 1. 概要
* **日時**: 2026-07-19
* **タスク**: Drumエンジンのコールバック駆動化とシーケンス分離 (Hotfix実装実行)
* **目的**: 
  * Renderer内のDrumエンジンから、データ層（PlaylistManager、TrackDatabase）への依存を完全排除し、デカップリングを実現する。
  * Drumのアニメーション中のメタデータ取得と、アニメーション完了後の画像ロード・再生状態確定のシーケンスを `Application` 側でコントロールできるようにする。

## 2. 対応内容詳細
* **`Renderer` のインターフェース刷新:**
  * `SetDrumTarget` を廃止し、`StartDrumAnimation` を新設。引数に `relativeDistance`、`dataProvider`（中間データ取得用）、`onComplete`（完了通知用）を受け取るように変更。
  * `m_animatingTargetIndex` を導入し、アニメーション中の「現在表示すべき論理インデックス」と「古いインデックス」を動的に管理する構造へと改修。
* **`Renderer::UpdateAnimation` の間引き・フリップ処理:**
  * `m_drumRelativePosition` の減衰計算中に整数境界を跨いだことを検知した場合、`dataProvider` をコールバックしてその時点の `relativeIndex` に対応するメタデータのみを取得。
  * 取得したデータを `DrumSlot` にセットし、画像を `nullptr` (ガラス板状態) にしたうえでフリップ処理を実行する。
  * `m_drumRelativePosition` が 0.0f に収束した時点で `onComplete` コールバックを発火。
* **`Widget_TrackInfo` の動的インデックス対応:**
  * 常に `relativeIndex == 0` を最終ターゲットとするのではなく、`ctx.animatingTargetIndex` および `ctx.animatingOldIndexOffset` を参照して描画するスロットを決定するように改修。
* **`Application` 側のシーケンス管理:**
  * `PlayCurrentTrack` にて、オーディオの再生（`m_audioPlayer.Play`）を即時開始しつつ、ビジュアル面は `dataProvider` によるメタデータ提供ロジックと、`onComplete` による正規アルバムアート適用・フレーミング反映ロジックをラムダ式で構築し `StartDrumAnimation` へ渡すように改修。
  * 再生・スキップ時の距離計算ロジックを最適化。

## 3. 関連ファイル
* `src/Renderer.h` / `src/Renderer.cpp` / `src/Renderer_Update.cpp`
* `src/WidgetContext.h` / `src/Renderer_Context.cpp`
* `src/Widget_TrackInfo.cpp`
* `src/Application_Playback.cpp` / `src/Application_Playlist.cpp`
* `PROJECT_ARCHITECTURE.md` (アーキテクチャの更新を追記)

## 4. 特記事項・次への引き継ぎ
* RendererからPlaylistManagerへの依存（#include 等）は全て排除可能な状態となった。必要であれば将来的にヘッダ依存のクリーンアップが可能。
* 連続スキップ時もシームレスにガラス板遷移とメタデータ書き換えが実行される設計。
