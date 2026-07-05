# RES:実装計画・作業レポート Phase 16-4: プレイリスト表示調整（モード別最適化）

## 1. 実装目的
プレイリストUIにおいて、曲一覧モードおよびプレイリスト一覧モードの表示をそれぞれ最適化する。具体的には、曲一覧モード時のツールバー非ホバー状態でのプレイリスト名表示、およびプレイリスト一覧モード時のメタデータ（曲数・総再生時間）の非同期解析・キャッシュ機能の実装と2行スタイルでのリッチな描画を実現する。メインスレッド（描画ループ）をブロックさせないUIスレッド保護を考慮する。

## 2. アーキテクチャ設計
### 要件1: 曲一覧モード時のツールバー表示 (PlaylistWidget)
    - `PlaylistWidget::Draw` 内のツールバー説明テキスト描画において、`isPlaylistListViewMode == false` 且つ非ホバー状態（`hoveredToolbarIndex == -1`）の際に、現在ロードされているプレイリスト名を1行で表示する。
    - プレイリスト名は拡張子を除外したベース名とする。`ConfigManager`等から現在のプレイリストパスを取得し、描画用に整形する処理を追加。

### 要件2: プレイリスト一覧のメタデータ取得・キャッシュ (Application / ConfigManager 等)
    - プレイリストのメタデータ（曲数、総時間文字列）を保持する構造体（例: `PlaylistSummary`）を定義。
    - `ConfigManager` または `PlaylistManager` で利用可能なプレイリスト一覧を取得する際に、各ファイル（TSV形式）を非同期または軽量にパースしキャッシュ化する。
    - 行数を曲数とし、TSVの第4カラム（`timeString`）を秒数に変換して合算。要素が足りない未解析の行がある場合は合算を中断し、総時間を `---` とする。
    - パース結果はキャッシュとして保持し、`WidgetContext` を介して `Renderer` 及び `PlaylistWidget` へ渡し、毎フレームのファイルI/Oを防ぐ。

### 要件3: プレイリスト一覧モード時の2行スタイル描画 (PlaylistWidget)
    - `PlaylistWidget::Draw` 内のプレイリスト一覧表示ループ（`isPlaylistListViewMode == true`）において、既存の単一行描画から2行スタイル描画へ改修。
    - 上段：`m_playlistTitleTextFormat` を用い、左寄せでプレイリスト名を描画。
    - 下段：`m_playlistArtistTextFormat` （または専用フォーマット）を用いて左寄せで「xx track」、右寄せ（または `m_playlistTimeTextFormat` 利用）で総再生時間（例: `1:12:45` または `---`）を描画。

## 3. 実装タスクリスト
[x] タスク1: PlaylistSummary キャッシュ構造とパースロジックの実装
    - プレイリストメタデータ（ファイルパス、表示名、曲数、総時間文字列）を保持する構造体の追加
    - TSVを読み込みメタデータを解析する処理の実装（未解析行検知で時間を`---`とする処理含む）
[x] タスク2: WidgetContext へのプレイリストメタデータ連携
    - `Application` のメインループ等から、利用可能なプレイリストのメタデータ一覧（キャッシュ）を `Renderer` 経由で `WidgetContext` に渡す仕組みの構築
[x] タスク3: ツールバーへのプレイリスト名表示（要件1）
    - `PlaylistWidget` のツールバー描画にて、曲一覧モード・非ホバー時にプレイリスト名を描画する処理の追加
[x] タスク4: プレイリスト一覧モードの2行スタイル描画（要件3）
    - `PlaylistWidget` のリストアイテム描画を2行スタイルに改修し、キャッシュされたメタデータを適用
[x] タスク5: PROJECT_ARCHITECTURE.md の更新
    - プレイリストUIの2行スタイルおよびメタデータ解析キャッシュの設計仕様をドキュメントに追記

## 4. 詳細作業内容
### タスク1: PlaylistSummary キャッシュ構造とパースロジックの実装
    - `Widget.h`に`PlaylistSummary`構造体を定義。
    - `Application::UpdatePlaylistSummaries()`を実装し、軽量なTSVパースおよび未解析判定、秒数フォーマット変換機能を追加した。
### タスク2: WidgetContext へのプレイリストメタデータ連携
    - `WidgetContext`に`const std::vector<PlaylistSummary>* availablePlaylistsCache`を追加。
    - `Renderer::Render`の引数を変更し、`Application::ForceRender()`から`&m_playlistSummaries`を渡すように修正。
    - 各プレイリスト変更時(`SaveToFile`呼び出し時)および初期化時にキャッシュを更新。
### タスク3: ツールバーへのプレイリスト名表示（要件1）
    - `PlaylistWidget::Draw`にて、非ホバー時および曲一覧モード時に`availablePlaylistsCache`から現在のプレイリスト名を取得し、ツールバー領域へ描画するよう変更。
### タスク4: プレイリスト一覧モードの2行スタイル描画（要件3）
    - `PlaylistWidget::Draw`のプレイリスト一覧表示ループを改修。キャッシュから曲数や総時間を取得し、それぞれ下段に表示するよう2行スタイルを実装した。
### タスク5: PROJECT_ARCHITECTURE.md の更新
    - プレイリストUIの2行スタイルおよびメタデータ解析キャッシュによる描画仕様の変更を `PlaylistWidget` の説明項に追記した。

## 5. HOTFIX1
### 原因・理由: 
    - `Application.cpp`から `Renderer::Render` 呼び出し時に `&m_playlistSummaries` キャッシュが渡されておらず、PlaylistWidget側でキャッシュが常にnullptrになっていた。これにより描画内容を判定できず、プレイリスト一覧モードでもリスト表示が曲一覧になってしまう問題が発生していた。
    - また、曲一覧モード時のツールバー名表示において、キャッシュ非存在時には表示自体がスキップされるロジックになっており、非ホバー時に確実に表示する要件が満たされていなかった。

### 対応: 
    - `Application.cpp` の `m_renderer.Render` 呼び出し引数の末尾に `&m_playlistSummaries` を追加した。
    - `Widgets.cpp` (`PlaylistWidget::Draw` および `UpdateAnimation`) にて、`ctx.isPlaylistListViewMode` による描画ループとデータ参照の分岐を完全に分離する構造に修正した。
    - ツールバーテキスト決定時、キャッシュに依存せず `std::filesystem::path` を用いて現在のプレイリストパスから直接名前（拡張子抜き）を取得するロジックに修正し、常時表示されるよう改善した。

## 5. HOTFIX 2
### 原因・理由: プレイリスト削除時の挙動と解析キャッシュ保存バグ
    - プレイリスト一覧モードにおいて、削除対象が常に「現在再生中のプレイリスト（`GetDefaultPlaylistPath`）」になっており、フォーカス中の対象が削除されていなかった。
    - また、バックグラウンドでのタグ解析処理（`ParseThreadFunc`）において、メモリ上のキャッシュは更新されていたが、ファイル（TSV）への永続化処理が抜けていたため、次回起動時などに再解析が必要になっていた。

### 対応: 削除ロジックの修正と解析スレッドの保存ロジック追加
    - `Application::OnPlaylistToolbarClick` 内のリスト削除処理を修正し、`m_focusedPlaylistIndex` から算出したファイルパスを削除するように変更。
    - 削除後直ちに `UpdatePlaylistSummaries()` を呼び出して一覧の表示キャッシュを更新。現在再生中のリストを削除した場合のみ `SwitchPlaylist` または `CreateNewPlaylist` によるフォールバックを実行するように分岐。
    - `Application::ParseThreadFunc` のループ内にて、未解析のトラックの処理が終わりキュー (`m_parseQueue`) が空になったタイミングで `m_playlistManager.SaveToFile()` を呼び出し、ファイルへの保存を一括で行う処理を追加。
