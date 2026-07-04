# REQ: Phase 10-4: プレイリスト情報のバックロードとキャッシュ戦略 実装計画レポート

## 1. 実装目的と基本方針
本フェーズでは、プレイリスト（`PlaylistManager`）が単なるファイルパスのリストから、曲名、アーティスト名、再生時間などのメタデータを保持する構造へと進化します。また、大量の曲が追加された際にもメインスレッド（UIや再生）をブロックさせないため、専用のバックグラウンドスレッドでタグ解析を行い、結果をタブ区切り（TSV）形式でローカルファイルにキャッシュする機構を導入します。
さらに、再生時（先読み時）に最新のタグ情報とキャッシュを比較し、齟齬があれば自動的にプレイリストを修復・更新保存するフェイルセーフ機構を実装します。

## 2. アーキテクチャと設計方針

### 要件1: メタデータ構造体の導入
`PlaylistManager` 内の管理単位を `std::wstring` から、メタデータを内包する構造体 `TrackMetadata` に変更します。

```cpp
struct TrackMetadata {
    std::wstring filepath;
    std::wstring title;
    std::wstring artist;
    std::wstring timeString; // 例: "03:45"
    bool isLoaded = false;   // 解析済みかどうか
};
```
これにより、`std::vector<std::wstring> m_playlist` は `std::vector<TrackMetadata> m_playlist` へと変更され、関連するすべてのインターフェース（`Add`, `GetCurrentTrack`, `GetNextTrack` 等）は後方互換性を保ちつつ内部で `TrackMetadata` を扱うように改修します。

### 要件2: バックグラウンド解析とスキップ機構
`Application` クラス内に、プレイリスト解析専用のバックグラウンドスレッド (`m_parseThread`) と、解析待ちのインデックスを保持するキュー (`m_parseQueue`) を導入します。
- **TagManagerの拡張**: `TagLib::AudioProperties` を利用して曲の長さ（秒数）を取得し、`MM:SS` 形式の文字列へ変換する機能を追加します。
- **処理フロー**: `Application::OnFilesDropped` などで曲が追加された直後、未解析 (`isLoaded == false`) のインデックスをキューに追加し、専用スレッドを起床させます。スレッドはバックグラウンドで `TagManager` を用いて解析し、`PlaylistManager` の該当インデックスのメタデータを更新します。
- **スキップ処理**: キャッシュファイルから読み込まれ、すでに `isLoaded == true` となっている曲はキューから除外（または処理時にスキップ）されます。

### 要件3: 再生時（先読み時）の自己修復と上書き更新
現在別スレッドで動いている `Application::PrefetchNextTrack` 内に、自己修復ロジックを追加します。
- 先読み時に対象曲の `TagManager` による最新情報を取得後、`PlaylistManager` から対象曲の `TrackMetadata` を取得して比較します。
- キャッシュが空、あるいは `title` や `artist` などの情報が食い違っている場合は、`PlaylistManager` のメタデータを最新情報で上書き（`UpdateMetadata` メソッド等を新設）し、直後に `PlaylistManager::SaveToFile` を呼び出してファイル上のキャッシュを自動修復します。

### 要件4: プレイリストへのキャッシュ保存・読み込み機構
`PlaylistManager::SaveToFile` と `LoadFromFile` を、TSV形式に対応させます。
- **データ形式**: `filepath \t title \t artist \t timeString`
- **保存処理**: `isLoaded == true` の曲は全項目を出力し、未解析の曲は `filepath` のみ（または空のプレースホルダー）を出力します。
- **読込処理**: タブ文字（`\t`）をデリミタとしてシンプルに分割します。分割後の要素が4つ揃っていればメタデータを復元し `isLoaded = true` とします。要素が足りない場合は未解析として扱います。

### 要件5: スレッドセーフな状態同期
メインスレッド（毎フレームのUI描画用アクセス）、先読みスレッド（`PrefetchNextTrack`）、および新設する解析スレッドの3つのスレッドから `PlaylistManager` が同時にアクセスされるようになります。
- `PlaylistManager` 内部に `std::mutex`（または `std::shared_mutex`）を導入し、配列の要素追加、メタデータの更新、および配列の読み取りに対する排他制御を厳格に行います。
- 特に、UI描画用のアクセス（`GetCurrentTrack` や `GetShuffleList`）は毎フレーム呼ばれるため、ロックの粒度を最小限に抑え、UIのフレームレート低下を防ぎます。

## 3. タスクリスト

以下の順序で実装を進めることを提案します。

*   **[x] Task 1: 基礎構造と排他制御の導入**
    *   `PlaylistManager.h/cpp` に `TrackMetadata` 構造体を定義し、内部リスト `m_playlist` を `std::wstring` から構造体へ置き換える。
    *   `std::mutex` を導入し、プレイリストへのアクセスをスレッドセーフ化する。
    *   `TagManager.h/cpp` を拡張し、`TagLib` から曲の長さ（秒数）を取得・文字列化する処理を追加する。
*   **[x] Task 2: TSVキャッシュ機構の実装**
    *   `PlaylistManager::SaveToFile` を改修し、メタデータをタブ区切りで出力する。
    *   `PlaylistManager::LoadFromFile` を改修し、タブ区切りの行をパースして `TrackMetadata` を復元する処理を実装する。
*   **[x] Task 3: バックグラウンド解析スレッドの実装**
    *   `Application.h/cpp` にメタデータ解析専用のバックグラウンドスレッド (`m_parseThread`)、キュー (`m_parseQueue`)、起床用条件変数 (`std::condition_variable`) を追加する。
    *   キューに入った未解析の曲を順次 `TagManager` で読み込み、`PlaylistManager` を更新するスレッドループを実装する。
*   **[x] Task 4: PrefetchNextTrackへの自己修復ロジック導入**
    *   `Application::PrefetchNextTrack` 内で取得したタグ情報と、プレイリスト側のキャッシュ情報を比較するロジックを追加する。
    *   差異があった場合にプレイリスト情報を更新し、`PlaylistManager::SaveToFile` を呼び出して自動保存する。
*   **[ ] Task 5: 動作確認とUI連携の調整**
    *   ビルドを行い、ドロップ時のバックグラウンド解析がUIをブロックしないか検証する。
    *   再起動時にキャッシュから情報が復元されること、およびタグ変更時に `PrefetchNextTrack` で自動修復されることを確認する。

## 4. 詳細作業内容

### Task 1: 基礎構造と排他制御の導入
* `TagManager` を拡張し、`TagLib::AudioProperties` から曲の長さ（秒数）を取得して `MM:SS` 形式の `m_timeString` として保持・取得できる処理 (`GetTimeString`) を追加。
* `PlaylistManager.h` に `TrackMetadata` 構造体を定義し、`std::vector<TrackMetadata> m_playlist` に変更。
* `PlaylistManager` に `mutable std::mutex m_mutex;` を導入。
* `PlaylistManager.cpp` の全てのパブリックメソッドに `std::lock_guard<std::mutex> lock(m_mutex);` を適用。既存の内部ロジックや `std::wstring` を返すインターフェース（`GetCurrentTrack`, `GetNextTrack`, `GetShuffleList`等）は、構造体の `filepath` プロパティを参照することで後方互換性を維持。
* ファイルへの保存 (`SaveToFile`) 実行時は、ロックの期間を最小化するためにローカルコピー (`std::vector<TrackMetadata> playlistCopy;`) を作成してからファイルI/O処理を行うように調整。
* ファイルからの読み込み (`LoadFromFile`) やシャッフル初期化時も安全にロックが行われるよう、ロック順序およびデッドロックの回避に配慮して実装。

### Task 2: TSVキャッシュ機構の実装
* `PlaylistManager::SaveToFile` を改修し、`TrackMetadata` が解析済み(`isLoaded == true`)の場合は `filepath \t title \t artist \t timeString` の形式で出力し、未解析の場合は `filepath` のみを出力する処理を実装。
* `PlaylistManager::LoadFromFile` を改修し、`std::wstringstream` と `std::getline(..., L'\t')` を用いて読み込んだ行をタブ区切りで堅牢にパースする処理を追加。
* パース結果の要素数が4つ以上揃っている場合は、追加された曲の `TrackMetadata` をスレッドセーフに検索・更新（`title`, `artist`, `timeString` をセットして `isLoaded = true`）するフェイルセーフ機構を実装。古いフォーマット（要素数が足りない）場合は従来通りファイルパスのみを追加して未解析状態とするフォールバック処理も適用。

### Task 3: バックグラウンド解析スレッドの実装
* `PlaylistManager` に `UpdateMetadata`、`IsTrackLoaded`、`GetUnparsedTracks` の3つのメソッドを追加し、パス指定によるメタデータ更新や未解析トラックの取得をサポート。
* `Application.h` にバックグラウンド解析スレッド関連のメンバ（`std::thread m_parseThread`, `std::mutex m_parseMutex`, `std::condition_variable m_parseCV`, `std::queue<std::wstring> m_parseQueue`, `std::atomic<bool> m_parseThreadRunning`）を追加。
* `Application.cpp` に `ParseThreadFunc()` を実装。キューから取得したファイルパスに対し、スレッドローカルな `TagManager` を用いてタグ情報を読み込み、`PlaylistManager::UpdateMetadata` を呼び出して反映。すでに `isLoaded == true` の場合はスキップする処理を実装。
* `Application::Initialize()` と `OnFilesDropped()` の末尾で `GetUnparsedTracks()` を呼び出し、未解析のトラックをキューに積んで `m_parseCV.notify_one()` でスレッドを起床させるロジックを実装。デストラクタでスレッドの終了と `join()` を適切に実行するよう追加。

### Task 4: PrefetchNextTrackへの自己修復ロジック導入
* `PlaylistManager.h/cpp` に `GetTrackMetadata` メソッドを追加し、特定のファイルパスに対するメタデータ情報をスレッドセーフに取得できるインターフェースを実装。
* `Application::PrefetchNextTrack` メソッド内で、`TagManager` を用いて次曲のタグ情報を取得した直後に `PlaylistManager::GetTrackMetadata` で既存のキャッシュ情報を取得して比較する処理を追加。
* 既存情報が未解析（`isLoaded == false`）であるか、あるいは取得した最新の `title` や `artist` と食い違いがある場合には、`PlaylistManager::UpdateMetadata` を呼び出して情報を上書き更新するロジックを導入。
* 上書き更新が行われた場合は、直後に `PlaylistManager::SaveToFile` を呼び出してキャッシュファイル（`.lst`）を最新状態に自動修復するように実装。これにより、メインスレッドをブロックすることなくフェイルセーフなキャッシュ更新が可能となった。
