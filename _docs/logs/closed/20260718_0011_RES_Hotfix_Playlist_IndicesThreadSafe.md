# RES:HOTFIX作業レポート: プレイリストインデックスのスレッドセーフ化

## 1. 実装目的
マルチスレッド環境下におけるスレッド競合とヒープ破壊によるハングを防ぐため、`PlaylistManager` からシャッフルインデックスを取得するメソッドを「参照返し」から「値返し（コピー）」へと修正する。

## 2. 調査内容
マルチスレッド環境下において、`const std::vector<size_t>& GetShuffleIndices() const;` のように参照返しでインデックス配列を返すと、UI描画スレッドが配列を読み取っている最中に、別のスレッドが `PlaylistManager` 内の同配列を変更してしまう可能性があり、スレッド競合によるクラッシュやヒープ破壊が発生するリスクがある。
そのため、`GetShuffleIndices` を値返しに変更し、ミューテックスで保護された状態でコピーを生成して返す方針とする。

## 3. 対象ファイル
* `src/PlaylistManager.h`
* `src/PlaylistManager.cpp`

## 4. 実装タスクリスト
[x] タスク1: PlaylistManager のゲッターのシグネチャ変更とスレッドセーフ化 - `GetShuffleIndices` を値返しに変更し、ロック取得下でコピーを返すように実装。

## 5. 詳細作業内容
* タスク1: PlaylistManager のゲッターのシグネチャ変更とスレッドセーフ化
    - `src/PlaylistManager.h` に `std::vector<size_t> GetShuffleIndices() const;` を追加。
    - `src/PlaylistManager.cpp` に `std::lock_guard<std::mutex> lock(m_mutex); return m_shuffleIndices;` を実装。
    - 呼び出し元のシグネチャ（`const std::vector<size_t>&`）は変更せず、値返しされた一時オブジェクトがconst参照として安全にバインドされる仕組みを維持した。
