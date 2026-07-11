# RES:実装計画・作業レポート Phase 19-3: ビジュアライザFFT基盤の独立化と解析エンジンの分離

## 1. 実装目的
PlaylistManagerとApplicationクラスが抱え込んでいるメタデータ管理とFFT解析の責務を分離し、堅牢なアーキテクチャへとリファクタリングする。TrackDatabaseによるデータ層の独立と、TrackAnalyzerによる解析スレッドの独立を行い、即時再生時のFFT初期値上書きバグ等の不具合を根絶する。

## 2. アーキテクチャ設計
### 要件1: TrackDatabase クラスの新設 (データ層の独立)
    - 楽曲固有の解析データ（曲名、アーティスト名、再生時間、peakAmplitude, maxFrequency等）を管理。
    - ファイルパス（絶対パス）をキーとする連想配列（std::unordered_map等）でメモリ上に保持。
    - データを `oztone_track.odb` (TSV形式) として保存・読み込みするメソッドを実装。

### 要件2: TrackAnalyzer クラスの新設 (解析スレッドの独立)
    - Applicationクラスからタグ解析、FFT事前スキャンロジック、キュー管理（m_parseQueue）、スレッド管理を抽出。
    - Applicationは楽曲追加時にTrackAnalyzerにパスを渡すだけになり、非同期処理の管理をTrackAnalyzerにカプセル化する。

### 要件3: PlaylistManager と Application の責務見直し (部分的分離)
    - PlaylistManagerはプレイリストの曲順（ファイルパス）と、アルバムアートのフレーミング情報のみを管理するようスリム化する。プレイリストファイル（.ozl）にはこれらの情報のみを保存する。
    - メインスレッド（再生や描画）は、必要な時にTrackDatabaseからRead Onlyで最新のFFT解析結果やメタデータを取得する構成とし、データソースを一元化する。

## 3. 実装タスクリスト
[x] タスク1: TrackDatabaseの構築
    - TrackDatabaseクラスの新設とデータ構造の定義。ファイルI/O（`oztone_track.odb`）の実装。
[x] タスク2: TrackAnalyzerの独立
    - TrackAnalyzerクラスの新設。Applicationからバックグラウンド解析スレッドやキュー処理を分離・移植。
[x] タスク3: PlaylistManagerの純化
    - PlaylistManagerからメタデータ・FFT解析データの管理を削除し、純粋なリスト・フレーミング管理クラスへリファクタリング。
[x] タスク4: Applicationの連携
    - ApplicationからTrackDatabaseとTrackAnalyzerを初期化・連携させるフローを構築。UIやAudioPlayerへのデータ受け渡しの修正、全体ビルド確認。

## 4. 詳細作業内容
### タスク1: TrackDatabaseの構築
    - `src/TrackDatabase.h` および `src/TrackDatabase.cpp` を作成。
    - `TrackMetadata` 構造体を `PlaylistManager.h` から `TrackDatabase.h` へ移動し、インクルードを修正。
    - TSV形式での読み書き(`LoadFromFile`, `SaveToFile`)およびスレッドセーフなアクセサ(`GetMetadata`, `SetMetadata`)を実装。
    - `CMakeLists.txt` にビルド対象として追加。

### タスク2: TrackAnalyzerの独立
    - `src/TrackAnalyzer.h` および `src/TrackAnalyzer.cpp` を新規作成。
    - `Application::ParseThreadFunc` にベタ書きされていたタグ解析や波形スキャン（FFT事前スキャン）を `TrackAnalyzer::ParseThreadFunc` へ移行。
    - `Application` の解析スレッド・キュー関連メンバ（`m_parseThread`, `m_parseMutex`, `m_parseQueue`等）を `TrackAnalyzer m_trackAnalyzer;` に置き換え。
    - `CMakeLists.txt` にビルド対象として `TrackAnalyzer.cpp` / `.h` を追加。

### タスク3: PlaylistManagerの純化
    - `PlaylistManager.h` に構造体 `PlaylistItem` を新設し、メタデータ管理を排除。内部キューを `std::vector<PlaylistItem>` へ移行。
    - `PlaylistManager.cpp` のファイルI/Oをスリム化し、曲パスとフレーミング情報（X, Y, スケール）のみを読み書きするよう変更。
    - `Application.cpp` および `TrackAnalyzer.cpp` で発生した、一時的な連携不足によるコンパイルエラーに対し、一時的にモック値を渡す・コメントアウトする等の応急処置を行い、ビルドが通る状態に修正した。

### タスク4: Applicationの連携
    - `Application` のメンバに `TrackDatabase m_trackDatabase` を追加。
    - `Application::Initialize` およびデストラクタにて、実行ファイルと同階層の `oztone_track.odb` に対してセーブ・ロードを行うように実装。
    - `TrackAnalyzer` の初期化引数を `TrackDatabase` へのポインタに変更し、解析スレッドから直接 DB へメタデータ・波形スキャン結果を書き込むよう実装を変更。
    - タスク3で応急処置としていた `Renderer` や再生処理へのデータ供給部分を修正。 `TrackDatabase::GetMetadata` 経由で取得したメタデータ (`peakAmplitude`, `maxFrequency` 含む) を渡すパイプラインを確立。
    - プレイリストファイル (`.ozl`) のロード・解析 (`UpdatePlaylistSummaries`) 時に、総再生時間を `TrackDatabase` から参照するようロジックを修正。

### HOTFIX: TrackDatabaseの冗長データパージ
    - `TrackDatabase` が管理する `TrackMetadata` 構造体および `oztone_track.odb` 保存フォーマットから、`PlaylistManager` 側の責務であるフレーミング情報 (`artOffsetX`, `artOffsetY`, `artScale`) を削除し、データ層の純化を行った。
    - `.odb` ファイル読込時は後方互換性を持たせ、旧フォーマット（9要素）からフレーミング情報を読み飛ばしつつピーク情報を復元できるように修正した。

### HOTFIX: TrackDatabaseの安全なマージ更新ロジック実装
    - `TrackDatabase::UpdateMetadata` を新設し、タグ解析データ（`isMetaLoaded`）とFFT解析データ（`isFFTLoaded`）の安全なマージ更新を可能にした。
    - `TrackAnalyzer` および `Application` で `SetMetadata` を呼び出していた箇所を `UpdateMetadata` に置換し、非同期スレッドが互いの完了済みデータを上書きして破壊する競合問題を解決した。
