# RES:HOTFIX作業レポート: 画像拡縮クランプとズーム中心補正

## 1. 実装目的
背景アートフレーミング機能において、「右クリック＋ホイール」による画像拡大縮小時にズームの中心がズレる問題、およびズームアウト時にスケールの限界がクランプされない（`ClampArtFraming` が `scale` を制限し呼び出し元に書き戻していない）不具合を修正する。

## 2. 調査内容
1. `Window_Mouse.cpp` からのコールバックにマウス座標（ズーム中心点）が渡されておらず、単純に原点が固定のままスケールが増減していたことがズーム中心のズレの原因である。
2. `Renderer::ClampArtFraming` 内部で `scale` のクランプ処理が欠落しており、最小スケールの制約が担保されていなかった。
これらを解決するため、コールバックシグネチャを拡張してマウス座標を渡し、ズーム中心を維持するオフセット計算ロジックを実装。併せて `ClampArtFraming` でのスケール制限と書き戻しを行う。

## 3. 対象ファイル
* `src/Window.h`
* `src/Window_Mouse.cpp`
* `src/Window_System.cpp`
* `src/Application_Initialize.cpp`
* `src/Renderer.cpp`

## 4. 実装タスクリスト
[x] タスク1: スクロールコールバックへのマウス座標の受け渡し
[x] タスク2: ズーム中心を維持するオフセット補正計算の実装
[x] タスク3: `ClampArtFraming` 内でのスケール参照の書き戻し

## 5. 詳細作業内容
* タスク1: スクロールコールバックへのマウス座標の受け渡し
    - `src/Window.h` の `SetArtFramingScrollCallback` シグネチャを `std::function<void(float, int, int)>` へ変更した。
    - `src/Window_Mouse.cpp` のホイール処理内で `pt.x`, `pt.y` をコールバック引数として渡すように修正した。
    - `src/Window_System.cpp` のページアップ/ダウンキー処理内でも、擬似的に画面中央の座標を算出（`GetClientRect` で取得）して渡すよう修正した。
* タスク2: ズーム中心を維持するオフセット補正計算の実装
    - `src/Application_Initialize.cpp` 内のスクロールコールバック処理を更新し、受け取ったマウス座標をDPIスケールで論理座標へ変換する処理を追加。
    - 拡大縮小の前後でマウスカーソル位置にある画像のピクセルが移動しないように `artX`, `artY` を再計算・補正するロジックを実装した。
* タスク3: `ClampArtFraming` 内でのスケール参照の書き戻し
    - `src/Renderer.cpp` の `ClampArtFraming` 冒頭にて `scale = (std::max)(1.0f, scale);` を追加し、スケールが1.0未満にならないようにクランプ（呼び出し元へ書き戻し）されるように修正した。
    - 既存の移動クランプ（`offsetX`, `offsetY`）ロジックは一切変更せず維持した。
