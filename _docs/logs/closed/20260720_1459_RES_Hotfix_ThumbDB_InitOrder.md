# RES:HOTFIX作業レポート: サムネイルDBの初期化順序の適正化

## 1. 実装目的
サムネイルDBの初期化がプレイリストの読み込み・ドラム初期化（データ注入）の後に行われており、アプリ起動直後にサムネイルが正常に表示されないバグを修正する。

## 2. 調査内容
`Application::Initialize` 内にて、`m_thumbnailDatabase.Initialize()` と `m_thumbCacher.Initialize()` の呼び出し箇所が `m_playlistManager.LoadFromFile` 等の後ろに配置されていた。これらを前倒しし、プレイリスト読み込みや自動再生開始時にはデータベースが復元済みであることを保証する。

## 3. 対象ファイル
* `src/Application_Initialize.cpp`

## 4. 実装タスクリスト
[x] タスク1:初期化順序の適正化 - `Application::Initialize` 内のサムネイルDB初期化処理を `m_audioPlayer.Initialize()` の前に移動。

## 5. 詳細作業内容
* タスク1:初期化順序の適正化
    - `src/Application_Initialize.cpp` を修正し、`m_thumbnailDatabase.Initialize();` と `m_thumbCacher.Initialize();` の呼び出しを、`m_renderer.Initialize()` の直後かつ `m_audioPlayer.Initialize()`（およびプレイリストロード）の直前に移動した。
    - これにより、トラックドラムの初期データ注入処理が走る時点でサムネイルDBが利用可能となり、起動時の表示不良が解消されることを確認。
