# RES:実装計画・作業レポート Phase 23-4: ThumbnailManagerの新設

## 1. 実装目的
「サムネイルに関すること」をすべて統括する視覚の司令塔である `ThumbnailManager` クラスを新設し、現在 `Application` クラス等が直接管理・操作している `ThumbCacher` と `ThumbnailDatabase` をその配下にカプセル化・再編する。これにより、サムネイル処理の責務を単一のFacade（窓口）に集約し、外部クラスの依存関係を整理してアーキテクチャの純化を行う。

## 2. アーキテクチャ設計
### 要件1: ThumbnailManager クラスの新設
- `src/ThumbnailManager.h` および `src/ThumbnailManager.cpp` を新規作成する。
- 内部に `ThumbnailDatabase` と `ThumbCacher` のインスタンスを保持する構成とする。
- 外部からのサムネイル操作（ID取得、画像のオンデマンド読み出し要求、バックグラウンド生成のエンキューなど）を単一の窓口として引き受けるAPIを設計する。

### 要件2: ThumbCacher と ThumbnailDatabase のカプセル化
- `ThumbnailManager` のコンストラクタで、内部コンポーネント（特に `ThumbCacher` への `ThumbnailDatabase` 参照渡しなど）の依存解決を行う。
- `ThumbnailManager.h` 側でのみこれらをインクルードし、外部からは極力 `ThumbCacher` 等の存在を意識させない設計とする。

### 要件3: 外部クラスの配線付け替え
- `Application` のメンバ変数を `m_thumbnailManager` 1つに統合する。
- `Renderer` クラスへ渡すポインタを `ThumbnailManager` に変更する。
- `Application_*` 系および `Renderer_*` 系のソースコードに散在する `ThumbnailDatabase` や `ThumbCacher` への直接呼び出しを、すべて `ThumbnailManager` 経由に置き換える。
- `CMakeLists.txt` に新規ファイルを追加しビルドを通す。

## 3. 実装タスクリスト
- [x] タスク1: ThumbnailManagerのクラス定義と実装作成
    - `src/ThumbnailManager.h` および `src/ThumbnailManager.cpp` の新規作成。
    - `CMakeLists.txt` への追加。
- [ ] タスク2: Applicationクラスの配線付け替え (ヘッダ・コンストラクタ・初期化)
    - `src/Application.h`, `src/Application.cpp`, `src/Application_Initialize.cpp` のメンバ変数統合と初期化ルーチン修正。
- [ ] タスク3: Applicationクラスの配線付け替え (再生・プレイリスト・ファイル・レンダリング)
    - `src/Application_Playback.cpp`, `src/Application_Playlist.cpp`, `src/Application_FileDrop.cpp`, `src/Application_Render.cpp` の呼び出しを修正。
- [ ] タスク4: Rendererクラスの配線付け替え
    - `src/Renderer.h`, `src/Renderer.cpp`, `src/Renderer_Context.cpp`, `src/Renderer_Update.cpp` のポインタおよび呼び出しを修正。
- [ ] タスク5: アーキテクチャ資料の更新
    - `PROJECT_ARCHITECTURE.md` に `ThumbnailManager` の説明を追記し、カプセル化の事実を明記する。

## 4. 詳細作業内容
### タスク1: ThumbnailManagerのクラス定義と実装作成
    **【作業ファイル】**
    - `src/ThumbnailManager.h` (新規作成)
    - `src/ThumbnailManager.cpp` (新規作成)
    - `CMakeLists.txt` (編集)
    **【作業内容】**
    - `ThumbnailManager` クラスを新規作成し、内部に `ThumbnailDatabase` および `ThumbCacher` を `std::unique_ptr` で保持する構造を実装しました。
    - `GetOrGenerateThumbId`、`HasCookedData`、`GetThumbnailBitmap` などのAPIを定義し、各内部コンポーネントへ処理を委譲（ルーティング）するよう実装しました。
    - `CMakeLists.txt` の `SOURCES` および `HEADERS` に作成したファイルを追加し、ビルドが正常に通ることを確認しました。

### タスク2: Applicationクラスの配線付け替え (ヘッダ・コンストラクタ・初期化)
    **【対象ファイル】**
    - `src/Application.h` : 更新
    - `src/Application.cpp` : 更新
    - `src/Application_Initialize.cpp` : 更新
    **【作業内容】**
    - 未実行

### タスク3: Applicationクラスの配線付け替え (再生・プレイリスト・ファイル・レンダリング)
    **【対象ファイル】**
    - `src/Application_Playback.cpp` : 更新
    - `src/Application_Playlist.cpp` : 更新
    - `src/Application_FileDrop.cpp` : 更新
    - `src/Application_Render.cpp` : 更新
    **【作業内容】**
    - 未実行

### タスク4: Rendererクラスの配線付け替え
    **【対象ファイル】**
    - `src/Renderer.h` : 更新
    - `src/Renderer.cpp` : 更新
    - `src/Renderer_Context.cpp` : 更新
    - `src/Renderer_Update.cpp` : 更新
    **【作業内容】**
    - 未実行

### タスク5: アーキテクチャ資料の更新
    **【対象ファイル】**
    - `PROJECT_ARCHITECTURE.md` : 更新
    **【作業内容】**
    - 未実行
