# RES:HOTFIX作業レポート: トラックドラムの初回起動時即時ロード対応

## 1. 実装目的
アプリ起動直後の初回再生時において、ドラムアニメーションをスキップし、即座に1曲目の情報をロード・表示させることで、UIの初期状態を正しく構築する。

## 2. 調査内容
`TrackDrum` クラスにおいて初回ロード時の状態を識別するフラグが存在せず、常にアニメーションの計算ルートを通ってしまっていた。そのため、`m_isDrumInitialized` フラグを新設し、初回呼び出し時には座標を 0 にリセットした上で即座にスロットへメタデータを注入してコールバックを返すよう修正する。

## 3. 対象ファイル
* `src/Renderer_TrackDrum.h`
* `src/Renderer_TrackDrum.cpp`

## 4. 実装タスクリスト
[x] タスク1: TrackDrumの初回起動状態管理フラグの追加
[x] タスク2: 初回起動時の即時ロード処理の実装

## 5. 詳細作業内容
* タスク1: TrackDrumの初回起動状態管理フラグの追加
    - `src/Renderer_TrackDrum.h` のメンバ変数に `bool m_isDrumInitialized = false;` を追加。
* タスク2: 初回起動時の即時ロード処理の実装
    - `src/Renderer_TrackDrum.cpp` の `StartDrumAnimation` メソッドの冒頭に、`!m_isDrumInitialized` 時の処理ブロックを追加。
    - 座標を初期化（`m_drumTargetPosition = 0;`, `m_drumAbsolutePosition = 0.0f;`）し、現在の曲情報をカレントスロットに直接セット。
    - `m_isDrumInitialized = true;` に設定後、引数の `onComplete()` を即時実行してリターンする早期リターン機構を実装した。
