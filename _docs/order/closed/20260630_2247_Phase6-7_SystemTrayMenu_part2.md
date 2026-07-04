【AIへの指示（Phase 6 - Step 7 - part 2 : プレイリストの初期化）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 7 - part 2】**
システムトレイのメニュー機能拡充の第2弾として、「プレイリストの初期化」を実装します。プレイリストの内部キューを空にし、保存されているプレイリストファイルをクリアし、現在の再生を完全に停止させてUIをリセットする一連の処理を構築します。

**【実装要件】**
1. **PlaylistManager の拡張**:
   - `PlaylistManager.h` および `.cpp` に、内部のリスト（`std::vector` や `std::set`）およびシャッフルインデックスを全て空にする `Clear()` メソッドを追加してください。

2. **Window クラスの拡張 (コールバックとメニュー追加)**:
   - `Window.h` にメニューID（例: `ID_TRAY_CLEAR_PLAYLIST`）を追加し、`Window.cpp` の `TRAY_MENU_ORDER` 配列の適切な位置（区切り線を挟むなど）に追加してください。
   - 責務分離のため、`Window.h` にプレイリスト初期化を通知するコールバック `std::function<void()> m_onClearPlaylistCommand;` と、そのセッター `SetClearPlaylistCallback` を追加してください。
   - `WM_COMMAND` 内で `ID_TRAY_CLEAR_PLAYLIST` が選択された際、このコールバックを発火させてください。

3. **Application クラスの連携 (初期化処理の実行)**:
   - `Application.h` に `ClearPlaylist()` メソッドを追加してください。
   - `Initialize` 内で `m_window.SetClearPlaylistCallback` を用いて、コールバックから `ClearPlaylist()` を呼び出すように登録してください。
   - `ClearPlaylist()` の実装内で以下の処理を行ってください。
     1. `m_playlistManager.Clear()` を呼び出してキューを初期化する。
     2. `ConfigManager` から `DefaultPlaylistPath` を取得し、`m_playlistManager.SaveToFile()` を呼び出して、空になった状態をファイルへ上書き保存（クリア）する。
     3. `m_audioPlayer.Stop()` を呼び出して現在の再生を止める。
     4. `m_isPrefetchReady` を `false` にし、`Renderer` の `SetTrackInfo` と `SetAlbumArt` に初期状態（タイトル「---」等、nullptrによるデフォルト画像）をセットしてUIを "No Track" 状態にリセットする。

**【事後処理】**
実装完了後、ビルドを行ってください。
D&Dで曲を追加して再生している状態で、システムトレイのメニューからプレイリストの初期化を実行し、「音楽が停止すること」「UIが初期状態に戻ること」、および「保存されているプレイリストファイルの中身が空になっていること」を確認できたら、実装レポートを出力してください。
