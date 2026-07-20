# RES:HOTFIX作業レポート: マウスによるフレーミング操作の完全復活

## 1. 実装目的
Phase 23-1の大解体に伴って一時的にコメントアウト・削除されていた背景アートのフレーミング操作機能（マウスによるドラッグ移動、ホイールによる拡大縮小、リセット）を復活させ、正常に動作するように修正する。

## 2. 調査内容
`Renderer`クラスに`ClampArtFraming`メソッドが不足しており、また`Application_Initialize.cpp`内の各種コールバック設定において、関連処理がコメントアウトされている状態であった。
WIC画像からD2Dビットマップを生成して保持している`m_currentBgBitmap`から画像サイズを取得し、描画画面領域（`m_dpiScale`加味済みの論理ピクセル幅・高さ）と比較してオフセットをクランプするロジックを再実装する方針とした。

## 3. 対象ファイル
* `D:\ozlab\oztone\src\Renderer.h`
* `D:\ozlab\oztone\src\Renderer.cpp`
* `D:\ozlab\oztone\src\Application_Initialize.cpp`

## 4. 実装タスクリスト
[x] タスク1: `Renderer`クラスへの `ClampArtFraming` の復活
[x] タスク2: `Application_Initialize.cpp` の再結線と残骸パージ
[x] タスク3: 操作のリアルタイム描画保証

## 5. 詳細作業内容
* タスク1: `Renderer`クラスへの `ClampArtFraming` の復活
    - `src/Renderer.h` および `src/Renderer.cpp` に `void ClampArtFraming(float& scale, float& offsetX, float& offsetY)` を再実装。
    - `m_currentBgBitmap` から元画像サイズを取得し、アスペクト比を維持したベーススケール（Cover描画）から現在スケールを計算してクランプ。
    - 画像が未ロード(`m_currentBgBitmap` が `nullptr` またはサイズが0以下)の場合はクラッシュを避けるためにそのまま return して何もしないフォールバックを入れた。

* タスク2: `Application_Initialize.cpp` の再結線と残骸パージ
    - `src/Application_Initialize.cpp` 内の `SetArtFramingMoveCallback`、`SetArtFramingScrollCallback`、`SetArtFramingResetCallback` にあった `// [Phase23-1]...` コメントと該当コードのコメントアウトを完全に解除。
    - `m_renderer.ClampArtFraming` と `m_renderer.SetBackgroundFraming` を再有効化した。

* タスク3: 操作のリアルタイム描画保証
    - マウスでのドラッグ・ホイール操作時等に即座に画面へ反映させるため、上記3つのコールバック処理の末尾に `this->ForceRender();` を追加し、毎フレームの自律更新を待たずにUI描画が更新されるよう保証した。
