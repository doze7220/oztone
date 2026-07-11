# RES:実装計画・作業レポート Phase 18-12: 曲ごとの背景アート・フレーミング（位置・拡大率の保存）

## 1. 実装目的
ウィンドウサイズが可変であるOZtoneにおいて、曲の背景アルバムアートの「最高のアングル（画角）」をユーザー自身がフレーミング（位置・拡大率の調整）し、曲ごとの設定値としてプレイリストのTSVキャッシュへ永続化して保存できるようにする。

## 2. アーキテクチャ設計
### 要件1: バックエンドの拡張（メタデータとTSVキャッシュ）
    - `PlaylistManager.h` の `TrackMetadata` 構造体に、フレーミング情報を保持する `artOffsetX`, `artOffsetY`, `artScale` (float型, 初期値: オフセット0.0f, スケール1.0f) を追加する。
    - `PlaylistManager::SaveToFile` および `LoadFromFile` メソッドにおける TSV の読み書きロジックを拡張し、既存の4項目の後ろに `\t artOffsetX \t artOffsetY \t artScale` を追加して永続化する。要素が不足している過去フォーマットのファイルに対しても初期値を適用する安全なフォールバック処理を実装する。

### 要件2: 描画・レイアウト計算への反映
    - `LayoutCalculator::CalculateBackgroundLayout` に対して `artOffsetX`, `artOffsetY`, `artScale` のパラメータを渡し、画像の拡大率および描画矩形の計算式へ適用する。
    - `Renderer::DrawBackground` にて、現在の再生曲の `TrackMetadata` から取得したオフセットとスケールを `LayoutCalculator` に渡して背景を描画する。

### 要件3: 「余白を作らない」入力ハンドリングとクランプ計算
    - 背景領域（他のUI判定がない領域）での操作、およびウィンドウアクティブ時のキーボード操作を追加する。
        - SHIFT + 左ドラッグ または SHIFT + カーソルキー: 背景アートの移動（`artOffsetX`, `artOffsetY` の増減）
        - SHIFT + マウスホイール または SHIFT + PAGEUP/PAGEDOWN: 背景アートの拡大縮小（`artScale` の増減）
        - SHIFT + HOME: リセット（オフセット0, スケール1.0）
    - スケールの最小値は `1.0f`（ウィンドウに余白なくフィットするサイズ）にクランプする。
    - 移動の範囲は、現在のスケールにおいてはみ出している画像領域（Overflow）の半分の範囲に上下・左右それぞれクランプし、黒帯（非表示領域）が見えないようにする。

### 要件4: リアルタイムな保存（セーブ）
    - ドラッグやホイール、キーボードによる操作が終了した（値が変動した）タイミングで、`PlaylistManager::UpdateMetadata` および `PlaylistManager::SaveToFile` を呼び出し、TSVキャッシュへ変更を即座に上書き保存する。

## 3. 実装タスクリスト
[x] タスク1: PlaylistManagerの拡張とTSVキャッシュ対応
    - `PlaylistManager.h` の `TrackMetadata` に `artOffsetX`, `artOffsetY`, `artScale` を追加。
    - `PlaylistManager.cpp` の `LoadFromFile`, `SaveToFile` を改修し、後方互換性を保ちながらTSVの入出力に対応。

[x] タスク2: LayoutCalculator と Renderer の描画適用
    - `LayoutCalculator.h/cpp` の `CalculateBackgroundLayout` を改修し、オフセットとスケールを考慮した矩形計算を実装。
    - クランプ用のヘルパーロジック（スケールと移動範囲の制限）を用意。
    - `Renderer.cpp` の `DrawBackground` から引数を渡すよう改修。

[x] タスク3: Window と Application での入力ハンドリングとリアルタイム保存
    - `Window.h/cpp` にフレーミング操作用のマウス・キーボード入力イベントをフックし、コールバック経由で `Application` に伝達する仕組みを追加。
    - `Application.h/cpp` でコールバックを受け取り、操作に応じて `TrackMetadata` を更新、クランプ処理を適用し、`PlaylistManager::UpdateMetadata` と `PlaylistManager::SaveToFile` で保存するロジックを実装。
    - 描画への即時反映のため `ForceRender` などを呼び出す。

## 4. 詳細作業内容
### タスク1: PlaylistManagerの拡張とTSVキャッシュ対応
    - `TrackMetadata` 構造体に `artOffsetX`, `artOffsetY`, `artScale` (デフォルト値: 0.0f, 0.0f, 1.0f) を追加した。
    - `LoadFromFile` 時のTSVパース処理において、カラム数が少ない過去フォーマットの場合はデフォルト値で埋める安全なフォールバックを実装した。
    - `SaveToFile` 時にはこれらの変数を追加出力するように変更した。
    - `UpdateArtFraming` メソッドと `GetArtFraming` メソッドを追加し、外部からフレーミング情報を読み書きできるようにした。

### タスク2: LayoutCalculator と Renderer の描画適用
    - `LayoutCalculator::CalculateBackgroundLayout` で画像の論理矩形計算時にオフセットとスケールを加算するよう変更した。
    - `LayoutCalculator::CalculateArtFramingBounds` を新設し、黒帯が見えないための最大移動可能範囲（クランプ境界）を計算できるようにした。
    - `Renderer` クラスに `SetBackgroundFraming` および `ClampArtFraming` メソッドを追加し、オフセットとスケールを保持、描画時に適用するようにした。

### タスク3: Window と Application での入力ハンドリングとリアルタイム保存
    - `Window` クラスにて、Shiftキー押下中のマウスドラッグ、ホイールスクロールの操作を検知し、`m_onArtFramingMove`, `m_onArtFramingScroll`, `m_onArtFramingReset`, `m_onArtFramingSave` の各コールバックを発火するよう実装した。
    - ドラッグ中およびホイール操作中は `Renderer::SetBackgroundFraming` を通じて即座に描画に反映（クランプ適用）し、マウスボタンを離したタイミングやホイール操作後に `PlaylistManager::SaveToFile` でTSVに永続化するよう `Application` 側にコールバック処理を実装した。
    - `Application::HandleMediaCommand` や `SwitchPlaylist` などの曲切り替え時に、現在の曲のフレーミング情報を取得し描画へ適用する処理を各所に追加した。

## HOTFIX: ウィンドウリサイズ時の背景アスペクト比崩れ修正
- **原因**: フレーミング設定後にウィンドウをリサイズすると、背景画像を描画する際の「切り抜き領域（`srcRect`）」と「描画先領域（`destRect`）」のアスペクト比がズレてしまい、画像が歪んでしまう問題が発生していた。
- **対応内容**: `LayoutCalculator::CalculateBackgroundLayout` の計算ロジックを変更し、`srcRect` は画像全体で固定し、`destRect` 側をウィンドウからはみ出させる形で描画するよう修正。ウィンドウと画像の比率から `baseScale` を計算し、動的クランプでオフセットを制限するようにした。これにより、リサイズされても常に正しいアスペクト比が維持されるようになった。

## HOTFIX 2: フレーミング設定のTSV保存（曲終了時セーブ）追加
- **原因と要件**: フレーミング設定のTSV保存が各操作時に行われており、I/O負荷がかかるため、「曲が切り替わる瞬間」および「アプリ終了時」に一括保存するように最適化する要件。
- **対応内容**: `Window` からの保存用コールバックの内容を空にし、代わりに `Application.cpp` の各所（デストラクタ、`Run` 内での自動進行時、`HandleMediaCommand` のNEXT/PREV時、`JumpToIndex` によるジャンプ再生時、および `SwitchPlaylist` や `ClearPlaylist` でリストが破棄される直前）に `m_playlistManager.SaveToFile()` を呼び出すように変更。これにより、ユーザーがどのように曲から離れても直前の画角設定が確実に永続化されるようになった。
