# 作業レポート: Phase 11-2 Hotfix (プレイリスト切り替え時のバックグラウンド解析停止バグ修正)

## 作業日時
2026-07-05 12:23

## 作業内容
`Application::SwitchPlaylist` において、プレイリスト切り替え時にタグ情報のバックグラウンド解析がトリガーされない不具合を修正しました。

## 原因
`SwitchPlaylist` 内で新しいプレイリストのロード (`m_playlistManager.LoadFromFile`) を行った後、未解析のトラックをバックグラウンド解析キューに追加し、スレッド (`m_parseCV.notify_one()`) を起床させる処理が抜けていました。このため、プレイリストを切り替えても新しいトラック群のタグ読み込みが開始されず、ファイル名がそのまま表示されてしまう状態になっていました。

## 対応内容
* **Application.cpp**
  * `Application::SwitchPlaylist` の末尾に、`Application::Initialize` や `OnFilesDropped` と同様の解析トリガー処理を追加しました。
  * `m_playlistManager.GetUnparsedTracks()` で取得した未解析トラックを `m_parseQueue` に追加し、`m_parseCV.notify_one()` を呼び出して解析スレッドを起床させるロジックを実装しました。

## 次のステップ
* ビルドおよび動作確認（プレイリスト切り替え後にバックグラウンド解析が正常に動作するか）を実施。
