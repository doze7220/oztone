# RES:HOTFIX作業レポート: プレイリストの自動更新（スナップショット監視）と .lst サポートの完全廃止

## 1. 実装目的
プレイリストファイル（.ozl）が外部操作によりリネーム・削除・追加された際、アプリケーションを再起動することなく、自動的にプレイリスト一覧UIに反映させるホットリロード機構（スナップショット監視）を実装する。また、旧来の `.lst` フォーマットのサポートをシステム全体から完全にパージし、`.ozl` フォーマットのみに統一する。

## 2. 調査内容
*   現状、プレイリストの再走査は `ConfigManager::GetAvailablePlaylists()` 経由で行われており、この中で `.lst` もリストアップされているため、この判定を除去して `.ozl` 専用に純化する。
*   ファイル変更の検知については、ディレクトリのファイルパスと最終更新日時 (`last_write_time`) をペアにしたスナップショット比較方式を用いる。この監視は `Application::Run()` のメインループ内で1秒（1000ms）ごとに軽量なポーリングを行うことで、UIのフレームレート（スレッドブロッキング）への影響を最小限に抑える。
*   再生中プレイリストがファイル単位で消失した場合に備え、フェイルセーフとして、ポーリング時にアクティブなパスが存在しない場合は即座に再生を停止し、安全な空状態へ移行させる機構を実装する。

## 3. 対象ファイル
*   `src/ConfigManager.h`
*   `src/ConfigManager_Playlist.cpp`
*   `src/Application.h`
*   `src/Application_Render.cpp`
*   `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md`

## 4. 実装タスクリスト
[x] タスク1: 旧プレイリスト(.lst)サポートの完全パージ
[x] タスク2: ConfigManager へのスナップショット監視機構の追加
[ ] タスク3: Application メインループへの毎秒ポーリングフックとキャッシュ再構築・フェイルセーフ機構の実装
[ ] タスク4: PROJECT_ARCHITECTURE.md の更新

## 5. 詳細作業内容
* タスク1: 旧プレイリスト(.lst)サポートの完全パージ
    - `src/ConfigManager_Playlist.cpp` の `ConfigManager::GetAvailablePlaylists()` における拡張子判定から `|| ext == L".lst"` を完全に削除し、`.ozl` 専用に修正する。
    - その他 `.lst` を特別扱いしている箇所が存在すれば完全に除去する。

    ### HOTFIX1
    #### 原因・理由: プレイリストファイルフォーマットの統一
        - .lst サポートをシステム全体から完全にパージし、.ozl 専用にするため。
    #### 対象ファイル:
        - src/ConfigManager_Playlist.cpp
    #### 対応: 旧プレイリスト(.lst)サポートの完全パージ
        - ConfigManager::GetAvailablePlaylists() における拡張子判定から `|| ext == L".lst"` を削除し、`.ozl` 専用に純化した。
* タスク2: ConfigManager へのスナップショット監視機構の追加
    - `src/ConfigManager.h` に、前回走査時の状態を保持するメンバ変数 `std::vector<std::pair<std::wstring, std::filesystem::file_time_type>> m_playlistSnapshot;` と、監視メソッド `bool CheckPlaylistSnapshotChanged();` を追加する。
    - `src/ConfigManager_Playlist.cpp` に `CheckPlaylistSnapshotChanged()` の実装を追加。対象ディレクトリ内の `.ozl` ファイルに限定して走査し、パスと更新日時のリストを取得。前回の `m_playlistSnapshot` と比較して差異（ファイルの増減、または日時の変化）があればリストを更新し `true` を返す。

    ### HOTFIX2
    #### 原因・理由: プレイリストファイルのスナップショット監視機構の実装
        - .ozl ファイルの増減や変更を検知し、アプリケーション再起動なしに反映させるホットリロード機構の基盤となるため。
    #### 対象ファイル:
        - src/ConfigManager.h
        - src/ConfigManager_Playlist.cpp
    #### 対応: ConfigManager へのスナップショット監視機構の追加
        - `ConfigManager.h` に `m_playlistSnapshot` と `CheckPlaylistSnapshotChanged()` メソッドの宣言を追加した。
        - `ConfigManager_Playlist.cpp` に `CheckPlaylistSnapshotChanged()` の実装を追加し、ディレクトリ内の .ozl ファイルのパスと更新日時を走査して前回との差異を検出するロジックを実装した。ファイルI/Oの例外発生時にはフェイルセーフとして false を返すように安全なエラーハンドリングを導入した。
* タスク3: Application メインループへの毎秒ポーリングフックとキャッシュ再構築・フェイルセーフ機構の実装
    - `src/Application.h` に、前回監視時刻を保持する `ULONGLONG m_lastPlaylistSnapshotTime = 0;` を追加。
    - `src/Application_Render.cpp` の `Application::Run()` 内メインループにて、`GetTickCount64()` を用いて1秒ごとに `m_config.CheckPlaylistSnapshotChanged()` を呼び出すノンブロッキングなフック処理を追加する。
    - 変更が検知された場合（`true` が返された場合）、`UpdatePlaylistSummaries()` を呼び出してUIのメタデータキャッシュを最新状態に同期させる（要件2）。
    - **フェイルセーフ（要件3）**: 変更検知時、現在アクティブなプレイリストパス（`m_config.GetDefaultPlaylistPath()`）がディスク上に存在しない場合、`m_playlistManager.Clear()` や `m_audioPlayer.Stop()` などを呼び出し、安全に再生を停止して空のプレイリスト状態（No Track）へフォールバックさせる。
* タスク4: PROJECT_ARCHITECTURE.md の更新
    - `PROJECT_ARCHITECTURE.md` の記述を見直し、`ConfigManager` によるプレイリストのホットリロード機能（スナップショット監視）が追加されたこと、および旧 `.lst` フォーマットが完全に廃止されたことを追記・修正する。
