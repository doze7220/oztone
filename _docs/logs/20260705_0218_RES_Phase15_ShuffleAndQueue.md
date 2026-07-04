# 実装レポート: Phase 15 - シャッフル機能とキュー管理の刷新 (計画立案)

## 実装目的
本フェーズでは、ロゴ拡張メニューのシャッフルボタンを機能させ、ストレート再生とシャッフル再生の切り替えを実装します。また、ドラッグ＆ドロップ(D&D)によるファイル追加時の挙動を見直し、SHIFTキー押下状態による挙動切り替え（BGM維持追加 vs 即時再生追加）をサポートします。これらの機能を複雑な条件分岐で実装するのではなく、`PlaylistManager`に「キューの再構築(RebuildQueue)」と「現在位置のワープ(WarpToTrack)」という共通ロジックを導入することで、エレガントなキュー管理アーキテクチャを実現します。

## アーキテクチャ設計・実装計画

### 1. ConfigManager の拡張 (シャッフル設定の永続化)
*   **変更点**: `OZtone.ini`に`ShuffleMode`（bool値、デフォルト`true`）の設定項目を追加。
*   **追加メソッド**: `GetShuffleMode()`, `SetShuffleMode(bool)` を実装し、INIファイルへの読み書き処理(`LoadSettings`, `SaveSettings`などの既存ロジック)に対応させる。

### 2. PlaylistManager の拡張 (究極のキュー管理・ワープ機能)
*   **`RebuildQueue(bool isShuffle)`**:
    *   既存のシャッフルインデックス配列（現在の周回用および次周回用）を完全にクリアする。
    *   `isShuffle`が`true`の場合は乱数を用いてランダムなインデックス配列を生成する。
    *   `isShuffle`が`false`の場合は`0`から順に連番のインデックス配列を生成する（ストレート再生）。
    *   過去のシャッフル状態や履歴は一切保持せず、完全に新しいキューを構築する設計とする。
*   **`WarpToTrack(const std::wstring& filepath)`**:
    *   現在ロードされているプレイリストの中から指定されたファイルパス（`filepath`）と一致するトラックを検索する。
    *   見つかった場合、再構築されたインデックス配列内での該当トラックの位置を探し出し、現在の再生インデックス(`m_shuffleIndex`)をその位置へ強制移動（ワープ）させる。

### 3. Application / Window の拡張 (シャッフル切り替え時の連動)
*   **シャッフルボタン操作検知**: `Window`側のクリック判定でシャッフルボタン（ロゴ拡張メニュー内）のクリックを検知し、`Application`へ通知する処理を実装します。
*   **状態の反転と適用**:
    *   `ConfigManager::GetShuffleMode()`を取得して反転させ、`SetShuffleMode()`でINIへ保存する。
    *   反転後のモードに基づき、`PlaylistManager::RebuildQueue`を実行。
    *   現在再生中の曲のパスを取得し、`PlaylistManager::WarpToTrack`を実行する。
    *   UI（プレイリスト等）は状態の変更(`m_shuffleIndices`の更新)を検知して自動的に再描画されるため、音声再生を中断することなくプレイリストの並び順だけが即座に更新される。

### 4. Application の拡張 (D&Dファイル追加時のSHIFTキー対応)
*   **`OnFilesDropped` メソッドの改修**:
    *   メソッドの先頭付近で `GetAsyncKeyState(VK_SHIFT) & 0x8000` を用いて、ドロップ時のSHIFTキー押下状態を判定する。
*   **SHIFTキー押下時（BGM維持モード）**:
    *   ファイルをプレイリストに追加（既存ロジック）するのみで、キューの再構築や現在再生位置の強制変更は行わない。
*   **SHIFTキー未押下時（通常追加・即時再生モード）**:
    *   ファイルをプレイリストに追加後、現在のシャッフル設定（`ConfigManager::GetShuffleMode()`）に従って `RebuildQueue` を実行。
    *   ドロップされたファイル群の最初のファイルパスをターゲットとして `WarpToTrack` を実行し、該当曲へ移動。
    *   即座にワープした曲から再生を開始する。
    *   ※プレイリストが空の状態からの初回追加時も、このロジックに統合することで処理を一本化する。

## 懸念点
*   `GetAsyncKeyState`によるキー状態の取得は、D&Dのメッセージ処理タイミングとユーザーのキーリリースがずれた場合に稀に判定漏れが発生する可能性があるが、実用上は許容範囲内と想定する。
*   `RebuildQueue`によるUIの動的更新がスムーズに行われるか。既存アーキテクチャでは`m_shuffleIndices`を参照してプレイリストを描画しているためUIの追随には問題ないはずだが、再構築中に他スレッド（描画等）からのアクセスが発生しないようにデータの更新順序に留意する。

---

## タスクリスト

* [x] `ConfigManager.h/cpp` に `ShuffleMode` プロパティのGetter/Setterを追加し、INIの入出力を実装する。
* [x] `PlaylistManager.h/cpp` に `RebuildQueue(bool isShuffle)` メソッドを実装する。
* [x] `PlaylistManager.h/cpp` に `WarpToTrack(const std::wstring& filepath)` メソッドを実装する。
* [x] `Window.h/cpp` にてロゴ拡張メニューのシャッフルボタン（トグル）のUI表示状態を `ConfigManager` の値と連動させ、クリック時に `Application` に通知するコールバックを実装する。
* [x] `Application.cpp` にて、シャッフル切り替え時に設定反転、`RebuildQueue`、現在曲での`WarpToTrack`を実行する処理を実装する。
* [x] `Application.cpp` の `OnFilesDropped` にて、`GetAsyncKeyState(VK_SHIFT)` を使用した分岐ロジックを追加し、SHIFT未押下時は追加＋`RebuildQueue`＋`WarpToTrack`＋再生開始となるよう改修する。

---

## 4. 詳細作業内容
* **ConfigManager**: `m_shuffleMode`を追加し、`[Audio]`セクションに`ShuffleMode`として読み書きする処理を実装。
* **PlaylistManager**:
  * `RebuildQueue(bool isShuffle)` を実装し、`m_shuffleIndices`および`m_nextShuffleIndices`を現在の全プレイリスト曲に対して再構築（シャッフルの有無に応じてランダム化するか連番にするか）するようにした。
  * `WarpToTrack` を実装し、指定されたファイルパスのトラックへ`m_shuffleIndex`を移動させるようにした。
* **Window**: `m_onShuffleCommand` のコールバックを新設し、ロゴ拡張メニューのシャッフルボタン（`ID_LOGO_SHUFFLE`）クリック時に通知するよう修正。
* **Application**: 
  * `Initialize`にて、シャッフル切り替えのコールバックを受け取り、`SetShuffleMode`、UI上のトグル状態の同期、`RebuildQueue`、そして現在のトラックへの`WarpToTrack`を実行するロジックを実装。
  * `OnFilesDropped` にて、`GetAsyncKeyState(VK_SHIFT)` による分岐を追加。SHIFT押下時（BGM維持）は従来通り`ShuffleNextLoop`を呼ぶのみとし、未押下時（即時再生）は`RebuildQueue`と最初に追加された曲への`WarpToTrack`を行い、即座に停止して対象トラックから再生を再開するよう改修。
