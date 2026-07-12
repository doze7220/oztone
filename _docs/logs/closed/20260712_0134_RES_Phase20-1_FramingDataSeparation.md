# RES:実装計画・作業レポート Phase 20-1: 背景アートフレーミング設定とプレイリストの分離

## 1. 実装目的
PlaylistManager によってプレイリストファイル（`.ozl`）内に保存されていた背景アートのフレーミング設定（X, Y, Scale）を分離し、独立した `ArtFramingDatabase` で管理する。これにより、データ層の責務分離を進め、プレイリスト管理クラスを純化するとともに、複数プレイリスト間で同一楽曲のフレーミング設定を共有可能にする。

## 2. アーキテクチャ設計
### 要件1: ArtFramingDatabase クラスの新設
    - 楽曲のファイルパスをキーとしてフレーミング設定 (`artOffsetX`, `artOffsetY`, `artScale`) を保持・管理する独立したクラス `ArtFramingDatabase` を新設する。
    - 設定は `oztone_framing.odb` という専用ファイルへ永続化（Save/Load）する。

### 要件2: PlaylistManager の純化と後方互換性（マイグレーション）
    - `PlaylistManager` および内部の `TrackMetadata` （または利用しているデータ構造）からフレーミング設定変数と、`.ozl` への保存ロジックを削除し、リスト管理専用クラスに戻す。
    - `PlaylistManager::LoadFromFile` において、フレーミング情報が含まれる古い `.ozl` フォーマットを読み込んだ際は、そのデータを抽出し `ArtFramingDatabase` へ移譲するマイグレーション処理を実装する。

### 要件3: Application 等のルーティング修正
    - 右ドラッグ等でのフレーミング操作時の値の保存先を `PlaylistManager` から `ArtFramingDatabase` へ変更する。
    - 楽曲再生時および描画の更新時に、`Renderer` や `LayoutCalculator` へ渡すフレーミング情報を `ArtFramingDatabase` から直接、もしくは `Application` 経由で取得するようロジックを修正する。

## 3. 実装タスクリスト
[x] タスク1: ArtFramingDatabase クラスの作成
    - `src/ArtFramingDatabase.h` および `src/ArtFramingDatabase.cpp` を作成する。
    - データの保持と、`SaveToFile`, `LoadFromFile`, `GetFraming`, `SetFraming` メソッドを実装する。
    - `CMakeLists.txt` にソースファイルを追加する。

[x] タスク2: PlaylistManager の修正とマイグレーション処理の追加
    - `TrackMetadata` (または該当構造体) から `artOffsetX`, `artOffsetY`, `artScale` を削除する。
    - `.ozl` 保存時 (`SaveToFile`) のフォーマットからフレーミング出力を削除する。
    - `LoadFromFile` にて古いフォーマットを検知した際、読み取ったフレーミングデータを `ArtFramingDatabase` へ渡し、後方互換性を担保する。

[x] タスク3: Application と Renderer の連携修正
    - `Application` の初期化処理で `ArtFramingDatabase` のインスタンス化と `LoadFromFile` を呼び出す。
    - 右ドラッグやマウスホイールによるフレーミング操作のコールバックにおいて、`PlaylistManager` ではなく `ArtFramingDatabase` へ変更を適用するよう修正する。
    - 描画処理 (`ForceRender` や `Renderer` の呼び出し) において、現在の再生曲のパスをキーとして `ArtFramingDatabase` からフレーミング設定を取得し適用する。
    - 必要なタイミング（アプリ終了時や曲切り替え時など）で `ArtFramingDatabase::SaveToFile` を呼び出すよう修正する。

[x] タスク4: PROJECT_ARCHITECTURE.md と task.md の更新
    - `PROJECT_ARCHITECTURE.md` のデータ構造仕様（PlaylistManagerの記述削除、ArtFramingDatabaseの追記等）を修正する。
    - `task.md` に記載の「保存データの責務分離」タスクを完了状態または最新状態に更新する。

## 4. 詳細作業内容
### タスク1: ArtFramingDatabase クラスの作成
    - `src/ArtFramingDatabase.h` および `src/ArtFramingDatabase.cpp` を新規作成し、ファイルパスをキーとしたフレーミング設定(X, Y, Scale)の管理と、TSV形式での保存・読み込み処理を実装した。
    - `std::mutex` を用いてスレッドセーフなアクセスを担保した。
    - `CMakeLists.txt` のビルド対象に上記ソースを追加した。

### タスク2: PlaylistManager の修正とマイグレーション処理の追加
    - `PlaylistManager.h` の `PlaylistItem` からフレーミング用変数 (`artOffsetX`, `artOffsetY`, `artScale`) を削除し、関連する `GetArtFraming`, `UpdateArtFraming` メソッドを削除した。
    - `SaveToFile` メソッドにて、ファイルパスのみをTSV出力するように変更した。
    - `LoadFromFile` の引数に `ArtFramingDatabase* framingDb` を追加し、古い `.ozl` ファイル（カラム数が4以上のもの）を読み込んだ際は、そこからフレーミング情報をパースして `framingDb` に登録（マイグレーション）するよう処理を追加した。

### タスク3: Application と Renderer の連携修正
    - `Application.h` に `ArtFramingDatabase m_framingDb;` と `std::wstring m_framingDbPath;` を追加した。
    - `Application::Initialize` で `m_framingDb` のロード処理を追加し、プレイリストのロード時に `m_framingDb` のポインタを渡すよう修正した。
    - `Application.cpp` 内に存在していた `m_playlistManager.GetArtFraming` および `UpdateArtFraming` の呼び出し箇所（全11箇所）を、`m_framingDb.GetFraming` および `SetFraming` に置き換えた。
    - フレーミング情報保存コールバック（`m_window.SetArtFramingSaveCallback`）および `Application` のデストラクタにて、`m_framingDb.SaveToFile` を呼び出して設定を永続化するよう対応した。

### タスク4: PROJECT_ARCHITECTURE.md と task.md の更新
    - `PROJECT_ARCHITECTURE.md` の `PlaylistManager` クラスの解説部分から、フレーミング情報の保持・保存に関する記述を削除した。
    - データ層クラスの並びに新設した `ArtFramingDatabase` クラスに関する解説を追記し、曲のファイルパスをキーとして背景アートのフレーミング設定を独立したデータベース（TSV形式）で永続化・管理する責務を持つ旨を記載した。
    - （※task.md は今回存在しないため、プロジェクトのドキュメントおよび本ログの更新をもって本タスク完了とする）

## 5. HOTFIX 1: ArtFramingDatabaseの保存タイミング修正
### 原因・理由: 
    - `ArtFramingDatabase` の保存(`SaveToFile`)がアプリケーション終了時にしか呼ばれず、再生中の曲スキップやプレイリスト切り替えなどの操作でフレーミング設定が永続化されていなかった。
    - 過去の最適化によって `SetArtFramingSaveCallback` に登録された保存処理が発火しない無効なルートとなっていた。

### 対応: 
    - `Application::Run` のオーディオ自動進行時、`Application::HandleMediaCommand` の曲スキップ時、プレイリストクリック時の曲ジャンプ、`Application::SwitchPlaylist`、`Application::ClearPlaylist` 内において、既存のプレイリスト保存処理付近に `m_framingDb.SaveToFile` の呼び出しを追加し、確実に永続化されるように修正した。
    - 無効となっていた不要な `m_window.SetArtFramingSaveCallback` の呼び出し処理を削除した。

## 6. HOTFIX 2: ArtFramingDatabaseの未初期化メモリバグ修正
### 原因・理由: 
    - ArtFramingDatabase::GetFraming 内で、指定された楽曲のフレーミングデータが見つからなかった場合に引数の参照変数へ値を代入せず alse を返していた。
    - さらに、呼び出し側の Application.cpp でも変数を初期化せずに GetFraming に渡していたため、未初期化メモリの不定値がそのまま描画処理やデータベースに渡り、アプリがクラッシュする原因となっていた。

### 対応: 
    - ArtFramingDatabase::GetFraming にて、見つからなかった場合は outX = 0.0f; outY = 0.0f; outScale = 1.0f; を代入してから alse を返すよう安全化のフォールバック処理を追加した。
    - Application.cpp 側のすべての呼び出し元で、変数の宣言時に loat artX = 0.0f, artY = 0.0f, artScale = 1.0f; と明示的に初期化するよう徹底的に修正した。
