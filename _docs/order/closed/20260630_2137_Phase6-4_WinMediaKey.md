【AIへの指示（Phase 6 - Step 4 : Windowsメディアキー対応）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 4】**
Windowsのメディアキー（再生/一時停止、停止、次の曲、前の曲）からの入力を受け付け、アプリケーションをコントロールできるようにします。そのための事前準備として、AudioPlayerおよびPlaylistManagerに不足しているコントロール系ロジックを拡張します。

**【実装要件】**
1. **AudioPlayer クラスの拡張**:
   - `TogglePlayPause()` メソッドを追加: 現在再生中 (`ma_sound_is_playing` が true) の場合は `ma_sound_stop` を呼び出して一時停止し、そうでない場合は `ma_sound_start` で再開するロジックを実装してください。
   - `Stop()` メソッドを追加: `ma_sound_stop` を呼び出した後、`ma_sound_seek_to_pcm_frame(&m_sound, 0)` 等を用いて再生位置を先頭に巻き戻すロジックを実装してください。

2. **PlaylistManager クラスの拡張**:
   - `Previous()` メソッドを追加: 現在のシャッフルインデックス (`m_currentIndex`) を1つ戻してください。もし `m_currentIndex` が 0 の場合（先頭の場合）は、現在のリストの末尾（要素数 - 1）にループするように処理してください。

3. **Window クラスの拡張**:
   - `Window.h` に、メディアキーコマンドを受け取るためのコールバック関数オブジェクト（例: `std::function<void(int)> m_onMediaCommand;`）を追加し、それをセットするためのメソッド（`SetMediaCommandCallback`）を実装してください。
   - `WindowProc` にて `WM_APPCOMMAND` メッセージを捕捉し、`GET_APPCOMMAND_LPARAM(lParam)` を用いてコマンドを取得してください。
   - コマンドが `APPCOMMAND_MEDIA_PLAY_PAUSE`, `APPCOMMAND_MEDIA_STOP`, `APPCOMMAND_MEDIA_NEXTTRACK`, `APPCOMMAND_MEDIA_PREVIOUSTRACK` のいずれかであった場合、登録されたコールバック関数へそのコマンド値を渡して発火させてください。

4. **Application クラスの連携 (メディアキー処理)**:
   - `Application::Initialize` 内にて、`m_window.SetMediaCommandCallback` を用いてラムダ式等のコールバックを登録してください。
   - コールバック内で以下の制御を行ってください。
     - `APPCOMMAND_MEDIA_PLAY_PAUSE` : `m_audioPlayer.TogglePlayPause()` を呼び出す。
     - `APPCOMMAND_MEDIA_STOP` : `m_audioPlayer.Stop()` を呼び出す。
     - `APPCOMMAND_MEDIA_NEXTTRACK` : `m_playlistManager.Advance()` を呼び出した後、現在再生中の音声を停止し、次の曲の情報をセットして再ロード＆再生する（`Run` ループ内の曲移行処理と同様のロジック）を実行する。
     - `APPCOMMAND_MEDIA_PREVIOUSTRACK` : `m_playlistManager.Previous()` を呼び出した後、現在再生中の音声を停止し、前の曲の情報をセットして再ロード＆再生する。

**【事後処理】**
実装完了後、ビルドを行ってください。楽曲再生中にキーボードのメディアキーを押し、再生/一時停止、停止、次の曲へのスキップ、前の曲へのスキップが正しく機能することを確認できたら、`PROJECT_ARCHITECTURE.md` を更新し、実装レポートを出力してください。
