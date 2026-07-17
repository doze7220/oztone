# RES:実装計画・作業レポート Phase 21-3: プレイリスト一覧復帰時のフォーカス復元

## 1. 実装目的
プレイリストのUIにおいて、曲一覧モードから「上の階層へ（プレイリスト一覧）」へ戻った際、現在再生中（アクティブ）のプレイリストが自動的に「選択状態（フォーカス）」となるように改修し、そのままリスト削除などの操作をスムーズに行えるようにする。

## 2. アーキテクチャ設計
### 要件1: フォーカス復元ロジックの追加
    - `src/Application_Playlist.cpp` 内の `OnPlaylistToolbarClick` メソッドを改修し、曲一覧モードからプレイリスト一覧へ戻る際に、フォーカス状態 (`m_focusedPlaylistIndex`) を復元するロジックを追加する。

### 要件2: 現在のプレイリストのインデックス検索
    - `m_config.GetDefaultPlaylistPath()` を用いて現在のプレイリストのパスを取得する。
    - `m_config.GetAvailablePlaylists()` を呼び出して利用可能なプレイリスト一覧を取得する。
    - 取得した一覧を走査して現在のパスと一致する要素を特定し、そのインデックスを `m_focusedPlaylistIndex` に設定する（見つからない場合は既存のデフォルト動作とする）。

## 3. 実装タスクリスト
[x] タスク1: Application_Playlist.cppのフォーカス復元ロジック追加
    - `src/Application_Playlist.cpp` の `Application::OnPlaylistToolbarClick` 内にある `!m_isPlaylistListViewMode` かつ `buttonIndex == 0` のブロックを修正。
    - プレイリスト一覧と現在のパスを取得・比較し、一致するインデックスを `m_focusedPlaylistIndex` に代入する処理を実装する。

## 4. 詳細作業内容
### タスク1: Application_Playlist.cppのフォーカス復元ロジック追加
    - `Application_Playlist.cpp`の`Application::OnPlaylistToolbarClicked`メソッドを改修。
    - プレイリスト一覧に戻る際、`m_config.GetDefaultPlaylistPath()`で取得したパスと、`m_config.GetAvailablePlaylists()`の要素を比較し、一致するインデックスを`m_focusedPlaylistIndex`に復元するように実装した。
