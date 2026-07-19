# RES:HOTFIX作業レポート: Drumエンジンのカプセル化とフリップ更新の純化

## 1. 実装目的
Drumエンジンの入力インターフェースを純化し、バケツリレーやフルコピーバッファなどの冗長な外部操作を廃止する。曲切り替え時の状態管理を、Drumエンジン内部での「ダブルバッファリング（0と1のインデックスフリップ）」のみに完全集約させ、外部からのDrumSlot直接操作や不要なデータ取得をパージすることで、最もストイックで堅牢なアーキテクチャを実現する。

## 2. 調査内容
これまで `Application::PlayCurrentTrack` 等において、相対的に前後数曲分の `DrumSlot` をマップとして外部構築し、`SetDrumTarget` に渡して丸ごとコピーする処理が存在していた。また、`Widget_TrackInfo` はそれらのバッファから現在や過去の位置に応じてデータを抽出していた。
この設計は外部層に過剰な状態管理の責務を負わせていたため、これを廃止し、`SetDrumTarget` の引数を新しいカレント曲の情報（単一のDrumSlot）のみに限定する。内部でインデックスをフリップ（0⇔1）させ、新しいスロットにのみテキスト情報をディープコピーするダブルバッファリング方式へ差し戻し・純化する方針とした。

## 3. 対象ファイル
* `D:\ozlab\oztone\src\WidgetContext.h`
* `D:\ozlab\oztone\src\Renderer.h`
* `D:\ozlab\oztone\src\Renderer.cpp`
* `D:\ozlab\oztone\src\Application_Playback.cpp`
* `D:\ozlab\oztone\src\Renderer_Context.cpp`
* `D:\ozlab\oztone\src\Widget_TrackInfo.cpp`
* `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md`

## 4. 実装タスクリスト
[x] タスク1: Drumエンジンの入力インターフェース純化とダブルバッファリング復元 - Renderer.cppのSetDrumTarget等の引数と内部フリップ処理への置き換え
[x] タスク2: 外部バッファ構築処理のパージ - Application_Playback.cpp等における外部でのバッファ構築処理の完全削除
[x] タスク3: 描画層(Widget)の参照切り替え - Widget_TrackInfo.cppからの外部オンデマンド抽出処理の削除と、2つのスロットのみを参照する受動態化への純化

## 5. 詳細作業内容
* タスク1: Drumエンジンの入力インターフェース純化とダブルバッファリング復元
    - `Renderer.h` および `WidgetContext.h` にて、`std::map<int, DrumSlot> drumBuffer` を削除し、`std::array<DrumSlot, 2> drumSlots` と `currentDrumSlotIndex`、`animatingOldIndexOffset` に置き換えた。
    - `Renderer.cpp` の `SetDrumTarget` にて、インデックスのフリップ（1 - current）を行い、フリップ後のスロットの画像をクリアし、引数で渡された新曲のメタ情報をディープコピーする3ステップの処理へ純化した。
* タスク2: 外部バッファ構築処理のパージ
    - `Application_Playback.cpp` 内の `PlayCurrentTrack` にて、`-2` ～ `+2` の5スロット分のメタデータを抽出してマップを構築していた処理を削除した。
    - カレント曲のメタデータ（タイトル、アーティスト、トラック番号）のみを取得し、単一の `DrumSlot` に格納して `SetDrumTarget` へ渡すように改修した。
* タスク3: 描画層(Widget)の参照切り替え
    - `Widget_TrackInfo.cpp` の描画処理（`UpdateLayout` および `Draw`）にて、相対インデックス `0` の場合は `drumSlots[currentDrumSlotIndex]` を、過去のインデックス（`animatingOldIndexOffset`）の場合は `drumSlots[1 - currentDrumSlotIndex]` を参照するように描画ロジックを修正。
    - それ以外の中間スロットについてはメタデータを持たない空のガラス板としてフォールバック描画されるように純化した。
    - `PROJECT_ARCHITECTURE.md` にて、フリップサイクルがダブルバッファリングに集約された旨を追記修正した。
