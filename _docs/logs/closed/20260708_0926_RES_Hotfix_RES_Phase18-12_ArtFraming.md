# RES:HOTFIX作業レポート: プレイリスト切り替えバグの修正

## 1. 実装目的
`Application::SwitchPlaylist` および `Application::CreateNewPlaylist` 内で、新しいプレイリストパスへ切り替わった後に `m_playlistManager.SaveToFile()` が呼ばれることで、切り替え前の情報が新しいファイルに上書き保存されてしまうバグ、および `m_playlistManager.Clear()` の呼び出し欠落によりリストの追加読み込み（追記）が発生してしまうバグの修正。

## 2. 調査内容
    - `Application::SwitchPlaylist` において、`m_config.SetDefaultPlaylistPath` によってパスが変更された後で `m_playlistManager.SaveToFile` が実行されており、保存先が誤っていたことを確認。
    - また、同じく `SwitchPlaylist` にて `m_playlistManager.Clear()` の呼び出しが抜け落ちていたことを確認。
    - `Application::CreateNewPlaylist` も同様に、新しいファイルを作成してパスを切り替えた後に `ClearPlaylist()` を経由して保存処理が走るため、古い情報が新しいファイルに保存されてしまう問題を確認。
    - `Application::ClearPlaylist` は、自身の処理の最後で `SaveToFile` を行っているが、処理の冒頭でも余分に `SaveToFile` を呼び出していたことを確認。

## 3. 詳細作業内容
    - `src/Application.cpp` の `Application::SwitchPlaylist` にて、パスを更新する前に現在のパス(`oldPath`)を取得し、`m_playlistManager.SaveToFile(oldPath)` を呼び出して現在の状態を安全に保存するように修正。
    - `Application::SwitchPlaylist` 内で、新しいパスのプレイリストを読み込む直前に `m_playlistManager.Clear()` を呼び出し、メモリ上のリストを空にする処理を追加。
    - `Application::CreateNewPlaylist` にて、新しいパスに変更（`m_config.SetDefaultPlaylistPath`）する前に、現在の状態を保存する `m_playlistManager.SaveToFile(defaultPath)` の呼び出しを追加。
    - `Application::ClearPlaylist` 内の冒頭にあった不要な `m_playlistManager.SaveToFile` の呼び出しを削除（末尾で正しく空の状態が保存されるように順序を維持）。
