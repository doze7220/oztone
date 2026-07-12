# RES: Hotfix - 終了時のプレイリストパスINI保存漏れ修正

## 1. 実装目的
アプリケーション終了時に現在アクティブなプレイリストのパスが INI ファイル (`OZtone.ini` の `DefaultPlaylistPath`) に保存されず、次回起動時に復元されない不具合を修正する。

## 2. HOTFIX1
### 原因・理由: 終了時のパス保存処理の欠落
- `Application::~Application()` 内でプレイリスト内容をファイルへ保存する処理は存在していたが、現在使用中のプレイリストのパス自体を `ConfigManager` を通じて INI ファイルへ書き戻す処理が欠落していた。
- これにより、アプリ起動中にプレイリストを切り替えたり新規作成した場合、終了時に最新のパスが INI に反映されず、次回起動時には以前のパスが読み込まれてしまっていた。

### 対応: パス保存処理の追加
- `Application::~Application()` において、プレイリストの破棄・保存が行われる直前に、`m_config.SetDefaultPlaylistPath(m_config.GetDefaultPlaylistPath());` を呼び出すよう追加した。
- `SetDefaultPlaylistPath` 内部には既に `WritePrivateProfileStringW` による INI ファイルへの保存処理が実装されているため、この呼び出しにより終了時の最新パスが確実に INI の `[Playlist]` セクションへ保存されるようになった。
