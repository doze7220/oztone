# RES:実装計画・作業レポート Phase 22-1: ThumbnailDatabaseとThumbCacherの基盤構築

## 1. 実装目的
ユーザーが任意にVRAM使用量やキャッシュサイズを制御できる設定値を追加し、画像バイナリをパックファイル（idx + img）として永続化・管理する `ThumbnailDatabase` クラス、および非同期でキャッシュを生成する専用ルーチン `ThumbCacher` クラスの骨組みを新規構築する。WICを用いた実際のリサイズ処理や描画連携は次タスクとするため、今回は基盤となるファイル・クラスの定義とスレッド/キャッシュの枠組み構築に専念する。

## 2. アーキテクチャ設計
### 要件1: ConfigManager の拡張
    - `[System]` セクションに `MaxThumbnailCache` と `ThumbnailSize` を追加。
    - ゲッター、セッターを `ConfigManager.h` および `ConfigManager.cpp` に追加し、INIへの読み書き処理 (`ConfigManager_System.cpp` などを想定) を実装する。
### 要件2: ThumbnailDatabase - パックファイルI/Oと自己修復ロジック
    - `src/ThumbnailDatabase.h` / `.cpp` を新規作成。
    - 管理ファイル `oztone_track_thumb_idx.odb` と `oztone_track_thumb_img.odb` のストリーム管理を実装。
    - 初期化時の破損チェックと自己修復ロジック（マジックナンバー `OZTHUMB_V1` および現在の `ThumbnailSize` のヘッダ比較検証と、不一致時の物理フォーマット（Truncate / ファイルサイズ0での上書き）処理）を実装する。
### 要件3: ThumbnailDatabase - LRUキャッシュとインターフェース構築
    - 内部に `std::list` と `std::unordered_map` を用いた `MaxThumbnailCache` 制御のLRUキャッシュ枠組みを用意する。
    - `GetThumbnailId` (パスからハンドル取得) と `DrawThumbnail` (ハンドルベースでの描画ディスパッチ) のメソッド宣言（インターフェース構築）を実装する。
### 要件4: ThumbCacher の新設
    - `src/ThumbCacher.h` / `.cpp` を新規作成。
    - `TrackAnalyzer` とは完全に分離した、サムネイル専用のバックグラウンド生成用スレッドの起動、終了、キュー管理の骨組みを作成する。
### 要件5: Application クラスへの配線とライフサイクル管理
    - `Application.h` に `ThumbnailDatabase` と `ThumbCacher` のインスタンス（または `std::unique_ptr`）を追加。
    - `Application::Initialize` 内での初期化処理を実装。
    - `Application` のデストラクタにて、`ThumbCacher` に停止フラグを立てて安全にスレッドを join して終了するライフサイクル管理を実装する（例外終了の防止）。
### 要件6: ビルド環境の更新
    - `CMakeLists.txt` に `src/ThumbnailDatabase.cpp` と `src/ThumbCacher.cpp` を追加する。
### 要件7: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` に追加されたクラス群の概要を追記する。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManager の拡張
    - `MaxThumbnailCache` (デフォルト: 100) と `ThumbnailSize` (デフォルト: 120) のプロパティ追加とINI連動。
- [x] タスク2: ThumbnailDatabase - パックファイルI/Oと自己修復ロジック
    - idx と img のストリーム管理、マジックナンバーとヘッダの比較、自己修復(Truncate)処理の実装。
- [x] タスク3: ThumbnailDatabase - LRUキャッシュとインターフェース構築
    - `std::list` と `std::unordered_map` を用いたLRU枠組みの構築。インターフェースメソッドの定義。
- [x] タスク4: ThumbCacher クラスの構築
    - サムネイル専用ワーカースレッドとキュー管理の構築。
- [ ] タスク5: Application クラスへの配線とライフサイクル管理
    - `Application` 内でのインスタンス化、初期化、および安全なスレッド終了（join）処理の追加。
- [ ] タスク6: CMakeLists.txt の更新
    - 新規ファイル2つをビルド対象に追加。
- [ ] タスク7: PROJECT_ARCHITECTURE.md の更新
    - `ThumbnailDatabase` と `ThumbCacher` の説明を追記。

## 4. 詳細作業内容
### タスク1: ConfigManager の拡張
    - `ConfigManager.h`, `ConfigManager_System.cpp`, `ConfigManager_DefaultIni.h` を修正し、`[System]` セクションに `MaxThumbnailCache` と `ThumbnailSize` の設定を追加・保存・読み込み処理を実装した。
### タスク2: ThumbnailDatabase - パックファイルI/Oと自己修復ロジック
    - `src/ThumbnailDatabase.h` と `src/ThumbnailDatabase.cpp` を新規作成。
    - `Initialize()` メソッドを実装し、マジックナンバー (`OZTHUMB_V1`) と `ThumbnailSize` によるヘッダ検証機能を追加。
    - 破損時・不一致時に idx/img の両ファイルを Truncate して初期化する自己修復ロジックを実装。
### タスク3: ThumbnailDatabase - LRUキャッシュとインターフェース構築
    - `ThumbnailDatabase.h` に `std::list` と `std::unordered_map` を用いたLRUデータ構造 (`m_pathToId`, `m_lruList`, `m_cache`) を追加。
    - `GetThumbnailId` と `DrawThumbnail` を実装し、キャッシュ有無とパックファイル存在有無（TODO部分）による3分岐の描画ディスパッチロジックを構築。
    - キャッシュヒット時はリストの先頭にIDを移動してLRUを更新し、画像を描画。未キャッシュ時は0.1の半透明白ブラシでプレースホルダーを描画するよう実装。
### タスク4: ThumbCacher クラスの構築
    - `src/ThumbCacher.h` と `src/ThumbCacher.cpp` を新規作成。
    - コンストラクタで `ThumbnailDatabase*` を受け取り保持。
    - `std::thread`, `std::mutex`, `std::condition_variable`, `std::queue<std::wstring>`, `std::atomic<bool>` を用いたスレッドとキュー管理基盤を実装。
    - スレッドの起動を行う `Initialize()`、安全な停止とジョインを行う `Uninitialize()` を実装。
    - `EnqueueTrack()` によるキューへのタスク追加とCV起床、ワーカースレッドループ(`WorkerLoop`)によるタスク取り出し処理の枠組みを構築（実処理はTODOとして記述）。
### タスク5: Application クラスへの配線とライフサイクル管理
    - (未実施)
### タスク6: CMakeLists.txt の更新
    - (未実施)
### タスク7: PROJECT_ARCHITECTURE.md の更新
    - (未実施)
