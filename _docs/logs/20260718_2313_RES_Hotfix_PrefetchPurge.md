# RES:HOTFIX作業レポート: 先読み機能（Prefetch）の完全パージ

## 1. 実装目的
旧来のバケツリレー式先読み機能に関する処理を完全にパージし、フリップ後の新スロットに対するオンデマンド画像ロードへの依存へと一本化する。

## 2. 調査内容
`Application.h`, `Application_Playback.cpp`, `Application_Playlist.cpp`, `Application_Render.cpp`、および `Application.cpp` に残存する `PrefetchNextTrack()` の宣言・実装、ならびに先読み関連変数（`m_isPrefetchReady`, `m_prefetchThread`, `m_prefetchedTitle`, `m_prefetchedArtist`, `m_prefetchedAlbumArt`）について調査した。これらが各所で呼び出されていたり、待機処理に使用されていたため、跡形もなくパージする方針とした。

## 3. 対象ファイル
- `src/Application.h`
- `src/Application.cpp`
- `src/Application_Playback.cpp`
- `src/Application_Playlist.cpp`
- `src/Application_Render.cpp`

## 4. 実装タスクリスト
[x] タスク1: Application層からの先読みメソッドと変数のパージ
[x] タスク2: 呼び出し元からの先読みトリガー・待機処理の削除
[x] タスク3: 画像ロードのフリップ後オンデマンドへの一本化

## 5. 詳細作業内容
* タスク1: Application層からの先読みメソッドと変数のパージ
    - `src/Application.h` から `PrefetchNextTrack()` の宣言および `m_isPrefetchReady` などの関連変数を完全削除。
    - `src/Application_Playback.cpp` の `PrefetchNextTrack()` 実装を削除。
    - `src/Application.cpp` のデストラクタに存在した `m_prefetchThread` 終了待ち処理を削除。
* タスク2: 呼び出し元からの先読みトリガー・待機処理の削除
    - `src/Application_Playlist.cpp` の `m_isPrefetchReady.store(false);` を3箇所削除。
    - `src/Application_Playback.cpp` の `PlayCurrentTrack` の最後での `PrefetchNextTrack();` 呼び出しを削除。
    - `src/Application_Render.cpp` の `Run` 内の `if (m_isPrefetchReady.load()) { ... }` によるガード処理を削除し、直接内部ロジックを実行させるように修正。
* タスク3: 画像ロードのフリップ後オンデマンドへの一本化
    - `src/Application_Playback.cpp` の `PlayCurrentTrack` における画像セット処理について、`m_isPrefetchReady.load()` に依存する条件分岐を削除し、常に現在のトラックからタグと画像を読み込む処理のみを残した。
