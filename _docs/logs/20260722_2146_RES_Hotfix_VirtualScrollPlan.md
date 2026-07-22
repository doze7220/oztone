# RES:実装計画・作業レポート Phase Hotfix: 仮想スクロール（Prev/Nextマウスホイール対応）

## 1. 実装目的
TrackInfo領域（左下のアルバムアート・曲名・アーティスト名）における、マウスホイールを使用した仮想スクロール機能（パラパラめくりと再生確定）を実装する。ホバー時のアフォーダンス（ツールチップ）を追加し、ホイール回転中は楽曲再生を継続したままドラムアニメーションのみを進行させ、一定時間入力が停止した段階で対象曲の再生を確定させる「高級オーディオのダイヤルのような手触り」を実現する。

## 2. アーキテクチャ設計
### 要件1: ホバー判定とアフォーダンス (Window / Widget層)
    - `Window` クラスに TrackInfo 領域（画面左端〜プレイリスト領域の境界手前）のホバー判定 (`IsInTrackInfoRegion`) を追加し、`m_isTrackInfoHovered` 状態を管理する。
    - `TrackInfoWidget` に非対称カラーフェード（フェードイン・フェードアウト）を実装し、ツールチップを描画する。
    - DRY原則に基づき、`VolumeControlWidget` で実装されていたツールチップ（吹き出し）のジオメトリ生成と描画ロジックを抽出し、`WidgetCommon` に共通のユーティリティ（例: `TooltipHelper` 等）として集約する。
    - `ConfigManager` にツールチップ表示用のオフセット設定等を追加する。
### 要件2: アキュムレータと初動の重み付け (Window層)
    - `Window::HandleMouseWheel` において TrackInfo 領域上のホイール操作を捕捉する。
    - `Window` クラス内部にアキュムレータ（蓄積変数）を設け、初動時のみ規定のしきい値（例: ホイールデルタ120や240）を超えるまでスクロールを無視する。スクロール状態へ移行した後は蓄積を解除し、1ティックごとにダイレクトに反応させる。
### 要件3: 仮想スクロールとドラムの連動 (Application層)
    - `Window` からの仮想スクロールイベントを `Application::OnVirtualScroll` 等で受け取り、相対距離を合算してドラムの回転 (`StartDrumAnimation`) を発動する。
    - 仮想スクロール中の `dataProvider`（コールバック）では、画像ロード（`GetOrGenerateThumbId`）や重いタグ解析（`ExtractTextMetadata`）を **一切行わず**、データベース上のキャッシュまたはファイル名からのフォールバック文字のみを返却するように隔離し、ファイルロックを完全に回避する。
### 要件4: スクロール確定と自動曲送りのブロック (Application層)
    - `Application` クラス内に仮想スクロール状態フラグと、最終入力からの経過時間を計測するタイマーを設ける。
    - メインループ（`Run` 等のポーリング機構内）で、仮想スクロール中は既存の自動曲送りロジックが発動しないようにブロック処理を入れる。
    - ホイールの回転が停止してから約500ms（マジックナンバー）が経過した時点でスクロール確定とみなし、蓄積された相対距離を `PlaylistManager` に反映させ、`PlayCurrentTrack` を発行して新曲の再生とリソースロードを正式に開始する。

## 3. 実装タスクリスト
[x] タスク1: ツールチップ描画ロジックの共通化 (WidgetCommon へ抽出)
    - `VolumeControlWidget` からツールチップ描画処理を抽出し、`WidgetCommon` へ移行。
[x] タスク2: ConfigManagerへの設定追加
    - 仮想スクロール用ツールチップのオフセットや関連する設定値を追加。
[x] タスク3: Window層 - TrackInfo領域のホバー判定追加 (WM_MOUSEMOVE 等の対応)
    - `Window_Mouse.cpp` にTrackInfo領域のホバー判定(`IsInTrackInfoRegion`)と状態管理を追加。
[x] タスク4: Window層 - アキュムレータの実装 (WM_MOUSEWHEEL の初動重み付け対応)
    - ホイール入力に対する初動の重み付けと、連続スクロール時のイベント発火処理を追加。
[x] タスク5: Application層 - 仮想スクロール状態とタイマーの基礎実装
    - 変数の追加と毎フレームのタイマー減算処理を実装。
[x] タスク6: Application層 - 自動曲送りのブロック対応
    - タイマー稼働中の `IsAtEnd()` などをフックして自動曲送りをブロック。
[x] タスク7: Application層 - ドラム連動と再生確定ロジックの実装
    - ホイール入力時のドラム回転発注(`StartDrumAnimation`)と、タイマー0時の `PlayCurrentTrack` 発動処理を実装（ファイルロック回避の専用dataProviderを含む）。
[ ] タスク8: Widget層 - アフォーダンス描画
    - `TrackInfoWidget` にて、ホバーアニメーションおよび共通化されたツールチップを表示。
[ ] タスク9: ドキュメントの更新
    - 仮想スクロール機構の実装に伴うアーキテクチャへの影響を仕様書に追記。

## 4. 詳細作業内容
### タスク1: ツールチップ描画ロジックの共通化 (WidgetCommon へ抽出)
    **【対象ファイル】**
    - `src/WidgetCommon.h`
    - `src/WidgetCommon.cpp`
    - `src/Widget_VolumeControl.cpp`
    **【作業内容】**
    - `WidgetCommon::DrawMouseScrollTooltip` を追加し、ツールチップの背景・アイコン・テキストを汎用的に描画できるように抽出した。
    - `VolumeControlWidget::Draw` 内の既存描画ロジックを、共通化した関数の呼び出しに置き換えてリファクタリングを完了した。
### タスク2: ConfigManagerへの設定追加 (および Task 1 Hotfix)
    **【対象ファイル】**
    - `src/ConfigManager.h`
    - `src/ConfigManager.cpp`
    - `src/ConfigManager_Playback.cpp`
    - `src/ConfigManager_System.cpp`
    - `src/ConfigManager_DefaultIni.h`
    - `src/Widget_VolumeControl.cpp`
    **【作業内容】**
    - [Task 1 Hotfix] `src/Widget_VolumeControl.cpp` に `#include "WidgetCommon.h"` を追加し、`WidgetCommon::DrawMouseScrollTooltip` に関連するビルドエラーを修正した。
    - `[Layout_VolumeControl]` に混入していたツールチップのデザイン設定（`TooltipIconSize` 等）を抽出し、新セクション `[Layout_Tooltip]` へ移行した。
    - `src/ConfigManager.h`, `src/ConfigManager.cpp`, `src/ConfigManager_Playback.cpp` を修正し、共通設定として読み込むようにリファクタリング。`VolumeControlWidget` 側も新しい共通ゲッターを使用するように対応した。
    - 以前の作業で `[Layout_NowPlaying]` に `TooltipOffsetX=-10.0`, `TooltipOffsetY=-20.0` が追加されていることを確認したため、元の設定を維持・拡張した。
### タスク3: Window層 - TrackInfo領域のホバー判定追加 (WM_MOUSEMOVE 等の対応)
    **【対象ファイル】**
    - `src/Window.h`
    - `src/Window_Mouse.cpp`
    **【作業内容】**
    - `Window.h` にホバー状態を管理する `m_isTrackInfoHovered` と `IsInTrackInfoRegion` メソッドの宣言を追加した。
    - `Window_Mouse.cpp` に `IsInTrackInfoRegion` の実装を追加し、X座標は画面左端からプレイリストのホバー判定領域手前まで、Y座標は再生コントロール領域より上でアルバムアートを含む高さを判定するようにした。
    - `HandleMouseMove` にて、既存のUI判定の優先順位を保ちつつ、他のUIがホバーされていない場合にのみ `m_isTrackInfoHovered` を更新するように排他制御を実装した。
    - `HandleMouseLeave` 発生時に状態をリセットする処理を追加した。
### タスク4: Window層 - アキュムレータの実装 (WM_MOUSEWHEEL の初動重み付け対応)
    **【対象ファイル】**
    - `src/Window.h`
    - `src/Window_Mouse.cpp`
    **【作業内容】**
    - `Window.h` に仮想スクロール用の状態 `m_virtualScrollAccumulator`, `m_isVirtualScrolling` およびコールバック `m_onVirtualScrollCallback` を追加し、リセット用の `ResetVirtualScrollState` を実装した。
    - `Window_Mouse.cpp` の `HandleMouseWheel` 内に、`m_isTrackInfoHovered` が有効な場合のホイール入力捕捉処理を追加した。
    - アキュムレータを用いて、初動時は `WHEEL_DELTA * 2`、スクロール中は `WHEEL_DELTA` を閾値とし、閾値を超過した際に回転方向のコールバック通知と蓄積値の減算を行うよう実装を完了した。
### タスク5: Application層 - 仮想スクロール状態とタイマーの基礎実装
    **【対象ファイル】**
    - `src/Application.h`
    - `src/Application_Render.cpp`
    **【作業内容】**
    - `src/Application.h` に仮想スクロールの状態を管理する `m_virtualScrollTimer` と `m_virtualScrollTargetIndex` を追加した。
    - `src/Application_Render.cpp` のメインループ(`Run`)内に、`deltaTime` を算出して `m_virtualScrollTimer` を減算する基礎的なタイマー処理を実装した。
### タスク6: Application層 - 自動曲送りのブロック対応
    **【対象ファイル】**
    - `src/Application_Render.cpp`
    **【作業内容】**
    - `src/Application_Render.cpp` のメインループ (`Run`) 内において、`m_audioManager.IsAtEnd()` による自動曲送り判定条件に `&& m_virtualScrollTimer <= 0.0f` を追加し、仮想スクロール中（タイマー稼働中）は自動で次の曲へ進まないようにブロックする処理を実装した。
### タスク7: Application層 - ドラム連動と再生確定ロジックの実装
    **【対象ファイル】**
    - `src/Application_Initialize.cpp`
    - `src/Application_Render.cpp`
    **【作業内容】**
    - `src/Application_Initialize.cpp` に `m_window.SetVirtualScrollCallback` を追加し、仮想スクロール時のターゲットインデックス更新と、ファイルロックを回避する（画像や重いタグ解析を行わない）専用の `dataProvider` を用いたドラム回転の発注を実装した。
    - `src/Application_Render.cpp` のメインループ (`Run`) 内において、`m_virtualScrollTimer` が0以下になった瞬間に `m_playlistManager.JumpToIndex` を呼び出してインデックスを確定させ、`PlayCurrentTrack` で再生を開始するとともに `m_window.ResetVirtualScrollState()` を呼び出して状態をリセットするロジックを実装した。
### タスク8: Widget層 - アフォーダンス描画
    **【対象ファイル】**
    - 
    **【作業内容】**
    - 
### タスク9: ドキュメントの更新
    **【対象ファイル】**
    - 
    **【作業内容】**
    - 
