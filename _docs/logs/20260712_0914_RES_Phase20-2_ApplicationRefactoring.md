# RES:実装計画・作業レポート Phase 20-2: ApplicationRefactoring

## 1. 実装目的
肥大化している `Application.cpp` のコード重複を徹底的に排除し、処理を適切なメンバ関数へ分割・集約するリファクタリングを実施する。本フェーズでは機能の追加や仕様変更は一切行わず、既存の動作とアーキテクチャの役割分担を維持したまま、コードの可読性と保守性を向上させる。

## 2. アーキテクチャ設計
### 要件1: 再生開始処理の集約 (`PlayCurrentTrack`)
    - 現在 `Initialize`, `OnFilesDropped`, `HandleMediaCommand`, `Run`, `SwitchPlaylist` など複数箇所に散在している「メタデータの自己修復(`UpdateTrackMetadataIfNeeded`)」「AudioPlayerによる再生(`Play`)」「次曲の先読み(`PrefetchNextTrack`)」といった一連の再生開始処理を、新設する `void Application::PlayCurrentTrack()` メソッドへ集約し、重複コードを排除する。

### 要件2: 巨大ラムダ式のメンバ関数化
    - `Initialize` や `OnFilesDropped` 内で直接記述されている巨大なコールバックラムダ式（プレイリストのクリック処理、ツールバーのクリック処理など）を、`OnPlaylistClicked` や `OnPlaylistToolbarClicked` などの独立したメンバ関数として抽出し、ラムダ式からはそれらを呼び出すだけのクリーンな構造に変更する。

### 要件3: `Initialize` メソッドの分割
    - 巨大化している `Initialize` メソッドから、各種コールバックの登録処理（MediaCommand, MouseWheel, PlaylistScroll 等）を `void SetupCallbacks()` という別メソッドへ分割し、見通しを良くする。

### 要件4: `ForceRender` メソッドの整理
    - `ForceRender` 内で行われている描画前の各種処理（進行度の計算、スペクトルの取得、ホバー状態の伝達など）を必要に応じて整理し、司令塔としての役割を明確化・可読性を向上させる。

## 3. 実装タスクリスト
[x] タスク1: `PlayCurrentTrack` の新設と既存処理の置き換え
    - `Application.h` に `bool PlayCurrentTrack();` を宣言する。
    - `Application.cpp` に `PlayCurrentTrack` の実装を追加し、各所（`Initialize`, `OnFilesDropped`, `HandleMediaCommand`, `Run`, `SwitchPlaylist`等）の重複処理をこのメソッド呼び出しに置き換える。

[x] タスク2: 巨大ラムダ式のメンバ関数化
    - `Application.h` に `OnPlaylistClicked`, `OnPlaylistToolbarClicked` などの必要なコールバック用メンバ関数を宣言する。
    - コールバック処理の実体をメンバ関数として実装し、元のラムダ式内から呼び出すように修正する。

[ ] タスク3: `SetupCallbacks` の新設と `Initialize` の分割
    - `Application.h` に `void SetupCallbacks();` を宣言する。
    - `Application.cpp` 内でコールバック登録処理を `SetupCallbacks` に移動し、`Initialize` から呼び出すようにする。

[ ] タスク4: `ForceRender` メソッドの整理
    - 処理ブロックごとに適切なコメントを追加し、ローカル変数のスコープを整理するなどして可読性を高める。

[ ] タスク5: PROJECT_ARCHITECTURE.md の更新確認と修正
    - 本リファクタリングにより影響を受ける記述（`Initialize`やその他のメソッドの説明）が `PROJECT_ARCHITECTURE.md` にあれば修正する。

## 4. 詳細作業内容
### タスク1: `PlayCurrentTrack` の新設と既存処理の置き換え
    - `Application.h` の private セクションに `bool PlayCurrentTrack();` を追加。
    - `Application.cpp` に `PlayCurrentTrack` を実装。内部で `m_audioPlayer.Play` の成功時に `UpdateTrackMetadataIfNeeded` と `PrefetchNextTrack` を呼び出し、成否を `bool` で返すよう構築。
    - `Application.cpp` 内で `HandleMediaCommand`, `Initialize` (2箇所), `OnFilesDropped`, `Run`, `SwitchPlaylist` の計6箇所に点在していた再生開始ブロックを `if (PlayCurrentTrack())` の判定に置き換え、元のスキップ制御（フェイルセーフ機構）を維持しつつ重複を排除。

### タスク2: 巨大ラムダ式のメンバ関数化
    - `Application.h` の private セクションに `OnPlaylistClicked`, `OnPlaylistDoubleClicked`, `OnPlaylistToolbarClicked` の3つのメンバ関数を追加。
    - `Application.cpp` の `Initialize` 内にベタ書きされていた巨大なラムダ式（`SetPlaylistClickCallback`, `SetPlaylistDoubleClickCallback`, `SetPlaylistToolbarClickCallback` に渡されていたもの）の中身を抽出し、先ほど追加したメンバ関数内に実装を移動。
    - 元のコールバック登録箇所は `[this](int x, int y) { this->OnPlaylistClicked(x, y); }` のように、抽出した関数を呼び出すだけのシンプルなラムダ式に置き換え、`Initialize` の見通しを改善した。

### タスク3: `SetupCallbacks` の新設と `Initialize` の分割
    - (未実施)

### タスク4: `ForceRender` メソッドの整理
    - (未実施)

### タスク5: PROJECT_ARCHITECTURE.md の更新確認と修正
    - (未実施)

## 5. HOTFIX1
### 原因・理由:
    - (未発生)

### 対応:
    - (未対応)
