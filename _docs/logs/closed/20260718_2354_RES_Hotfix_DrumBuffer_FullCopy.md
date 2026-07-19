# RES:HOTFIX作業レポート: トラックインフォのフルコピー・バッファリング化

## 1. 実装目的
ドラムアニメーションに登場する全てのトラック情報（OLD、中間、NOW）を値（ディープコピー）として保持する独立したバッファ配列を新設し、プレイリストの切り替え時においてもアニメーション完了までデータを保護する堅牢なアーキテクチャを確立すること。また、Widget側の描画ロジックから外部のプレイリスト依存を完全に排除し、単一の純粋なデータ受け取りフローに修正する。

## 2. 調査内容
旧実装では `std::array<DrumSlot, 2>` を利用し現在と過去のデータのみを管理しており、中間スロット（ジャンプ時のアニメーション用）は描画時に `shuffleMetadataList` を直接参照するハイブリッドな構成であった。このため、プレイリストの切り替え時に `shuffleMetadataList` 自体が初期化されると、中間やOLDのテキスト情報が意図しない新しいプレイリストのデータで上書きされたり空白になる不具合が発生しやすかった。
これを解決するため、描画に必要なメタデータ群を `std::map<int, DrumSlot>` として丸ごとコピー・保持する設計に変更した。
* アプリケーション層 (`PlayCurrentTrack`) のタイミングで必要なトラック情報を全て抽出し、Renderer にディープコピーとして渡す。
* Renderer側 (`SetDrumTarget`) では、渡された新データと既存のバッファをシフトしながらマージし、古いメタデータを安全に保持し続ける。
* Widget側 (`TrackInfoWidget`) では、渡されたバッファのみを参照し、複雑な条件分岐（現在か過去か中間か）をすべて排除したシンプルな描画フローへと純化した。

## 3. 対象ファイル
* `src/WidgetContext.h`
* `src/Renderer.h`
* `src/Renderer.cpp`
* `src/Renderer_Context.cpp`
* `src/Renderer_Update.cpp`
* `src/Application_Playback.cpp`
* `src/Widget_TrackInfo.cpp`

## 4. 実装タスクリスト
[x] タスク1: ドラムバッファのフルコピー化 - WidgetContext および Renderer の `drumSlots` を `drumBuffer (std::map)` に変更
[x] タスク2: プレイリスト切り替え時のデータ保護 - Application層でのデータ構築およびRenderer層でのシフト・マージ処理の実装
[x] タスク3: 描画の純化と外部依存の完全パージ - Widget_TrackInfo.cpp の描画ループから外部依存を排除

## 5. 詳細作業内容
* タスク1: ドラムバッファのフルコピー化
    - `WidgetContext.h` の `std::array<DrumSlot, 2> drumSlots;` を `std::map<int, DrumSlot> drumBuffer;` に置き換えた。
    - `Renderer.h` も同様に変更し、`SetDrumTarget` のシグネチャをフルコピーバッファを受け取れるように拡張した。
* タスク2: プレイリスト切り替え時のデータ保護
    - `Application_Playback.cpp` の `PlayCurrentTrack` にて、`SetDrumTarget` を呼び出す前に `startIdx` から `endIdx` までのトラック情報を抽出し、`std::map<int, DrumSlot>` に実体としてコピーするロジックを追加した。
    - `Renderer.cpp` の `SetDrumTarget` では、既存の `m_drumBuffer` のインデックスを相対距離分シフトさせ、新しいデータを上書き・結合させることで、OLD情報が上書きされずに正しく残るようにした。
* タスク3: 描画の純化と外部依存の完全パージ
    - `Widget_TrackInfo.cpp` にて、これまでは `relativeIndex == 0`, `relativeIndex == m_animatingOldIndexOffset`, `それ以外（中間スロット）` というように分岐していた抽出ロジックをすべて廃止した。
    - すべての情報を `ctx.drumBuffer.at(relativeIndex)` から直接・無条件で読み取って描画するフローに純化した。
