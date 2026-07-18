# RES:HOTFIX作業レポート: プレイリスト切り替え時のシャッフル状態依存化

## 1. 実装目的
プレイリストの切り替え時やアプリ起動時にロードされたプレイリストの再生キューが、現在のシャッフルモード（ON/OFF）の設定を無視して常にシャッフル状態で初期化されてしまう不具合を修正する。

## 2. 調査内容
`PlaylistManager::LoadFromFile` の内部で、プレイリスト読み込み直後に無条件で `InitializeShuffle()` が呼び出されており、これによってキューが強制的にシャッフルされていた。この強制シャッフルロジックを取り除き、ロード後の適切なタイミングで現在の `ConfigManager` のシャッフル状態を参照し、`RebuildQueue` を明示的に呼び出すようにアーキテクチャを整理する必要があった。

## 3. 対象ファイル
* `src/PlaylistManager.cpp`
* `src/Application_Playlist.cpp`
* `src/Application_Initialize.cpp`

## 4. 実装タスクリスト
[x] タスク1: `PlaylistManager::LoadFromFile` 内の強制シャッフルロジック削除 - 無条件に `InitializeShuffle()` を呼ぶ処理を削除し、呼び出し側に責任を委譲する設計に変更。
[x] タスク2: `Application::SwitchPlaylist` 内での `RebuildQueue` 呼び出し追加 - プレイリストファイル読み込み直後に `m_config.GetShuffleMode()` を渡してキューを再構築する処理を追加。
[x] タスク3: `Application::Initialize` 内での `RebuildQueue` 呼び出し追加 - 初期起動時のプレイリスト読み込み後にも同様にシャッフルモードを参照してキューを再構築する処理を追加。

## 5. 詳細作業内容
* タスク1: `PlaylistManager::LoadFromFile` 内の強制シャッフルロジック削除
    - プレイリストの読み込みが成功した場合に強制的に `InitializeShuffle()` を呼んでいたブロックを削除し、呼び出し側で `RebuildQueue` を呼ぶようにコメントを残す設計へ純化した。
* タスク2: `Application::SwitchPlaylist` 内での `RebuildQueue` 呼び出し追加
    - `SwitchPlaylist` において `m_playlistManager.LoadFromFile(filepath)` を実行した直後、`m_playlistManager.RebuildQueue(m_config.GetShuffleMode())` を呼び出す処理を挿入。これにより、シャッフルOFF時は連番、ON時はランダムなキューが構築されるようになった。
* タスク3: `Application::Initialize` 内での `RebuildQueue` 呼び出し追加
    - `Initialize` での初回プレイリスト読み込み直後にも、`m_playlistManager.RebuildQueue(m_config.GetShuffleMode())` を実行する処理を追加し、起動直後の再生順序の設定整合性を確保した。
