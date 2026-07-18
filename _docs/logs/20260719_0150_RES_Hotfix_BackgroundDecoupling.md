# RES:HOTFIX作業レポート: 背景アートとドラムアニメーションの連動パージおよび独立描画への復元

## 1. 実装目的
ドラムアニメーションの進行（相対距離やフリップサイクル）と背景アートの描画が連動していたため生じていた問題を解消し、曲決定時に即座に背景画像が読み込まれ、ドラム状態に依存せずに独立して描画されるように復元すること。

## 2. 調査内容
- `Application::PlayCurrentTrack` にて、背景画像やフレーミング情報の読み込みがドラムアニメーション完了時のコールバック（`onComplete`）内で実行されていたため、曲が切り替わってから背景が更新されるまでに遅延が生じていた。
- `Renderer` 側で背景アートを単独で保持する変数がなく、ドラムスロット（`m_drumSlots`）にセットされたアルバムアートを背景として参照していたため、ドラムのフリップアニメーションの完了を待たなければ新しい背景画像を描画できない構造になっていた。

修正方針として、`Application_Playback.cpp` 側で曲決定時に即時タグ解析を行って背景画像を取得し、Renderer側に新設する `m_backgroundArtBitmap` へセットする仕組みへと戻す。これにより、`Renderer_Draw.cpp` での背景描画はドラムスロットから独立した純粋な描画ロジックとなる。

## 3. 対象ファイル
* `src/Application_Playback.cpp`
* `src/Renderer.h`
* `src/Renderer.cpp`
* `src/Renderer_Draw.cpp`

## 4. 実装タスクリスト
[x] タスク1: Application層での背景画像即時ロードの復元 - `Application::PlayCurrentTrack` を修正し、`StartDrumAnimation` 実行前に背景画像の即時読み込みとフレーミング情報設定を行う。
[x] タスク2: Rendererへの背景アート専用インターフェースの復元・分離 - `Renderer.h` および `Renderer.cpp` に `m_backgroundArtBitmap` と `SetBackgroundArt` メソッドを追加し、`ClampArtFraming` の参照先も変更。
[x] タスク3: 背景とドラムの同期ロジックの完全パージ - `Renderer_Draw.cpp` の `DrawBackground` で `m_drumSlots` ではなく `m_backgroundArtBitmap` を参照するように修正し、ドラム連動をパージ。

## 5. 詳細作業内容
* タスク1: Application層での背景画像即時ロードの復元
    - `Application_Playback.cpp` における `PlayCurrentTrack` メソッド内で、`m_tagManager.Load(track)` を即座に実行し、`artBitmap` を生成して `SetBackgroundArt` と `SetBackgroundFraming` を呼ぶように修正。ドラムスロット用の `SetAlbumArt` は引数として渡した `artBitmap` を用い、引き続き `onComplete` 時に実行するよう分離した。
* タスク2: Rendererへの背景アート専用インターフェースの復元・分離
    - `Renderer.h` および `Renderer.cpp` に `Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backgroundArtBitmap;` と `void SetBackgroundArt(ID2D1Bitmap* bitmap);` を追加し実装。
    - 同時に背景フレーミングの計算 (`ClampArtFraming`) で用いる画像参照を `m_backgroundArtBitmap` へ変更し、ドラムスロットとの依存を絶った。
* タスク3: 背景とドラムの同期ロジックの完全パージ
    - `Renderer_Draw.cpp` の `DrawBackground` 内で、描画用の画像として `m_drumSlots[m_currentDrumSlotIndex].artBitmap` を参照していた箇所を `m_backgroundArtBitmap` に置き換えた。これにより、ドラムアニメーションの状態によらず独立した背景アートが描画される形に純化された。
