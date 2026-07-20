# RES:実装計画・作業レポート Phase 23-6: BackgroundManagerの新設

## 1. 実装目的
Phase 23-1にて解体された背景画像描画機能を、独立した美しい描画パイプラインとして復活させるため、「空間の司令塔」となる `BackgroundManager` クラスを新設する。FileManagerを用いた非同期画像抽出、WICを用いたデコード、およびクロスフェードアニメーションの状態管理をカプセル化し、UIスレッドをブロックせずにRendererに対して完全に分離された状態で背景画象の描画情報を提供する。

## 2. アーキテクチャ設計
### 要件1: BackgroundManager クラスの新設
    - `src/BackgroundManager.h` および `src/BackgroundManager.cpp` を新規作成する。
    - 背景画像（現在・過去）のデータ保持、ロード要求キューの管理、スレッドのライフサイクル管理を行う基盤を構築する。

### 要件2: FileManager との非同期連携による背景ロード
    - バックグラウンドスレッド(WorkerLoop)を設け、`FileManager::ExtractAlbumArtBinary` を呼び出して画像バイナリを非同期抽出する。
    - 抽出したバイナリをWICを用いてデコードし、`IWICBitmapSource` などの形式で保持する。これによりUIスレッドをブロックしない。

### 要件3: クロスフェードアニメーションの管理
    - ロード完了時に過去の画像を保持しつつ、新しい画像をカレントとする。
    - タイマーやデルタタイムを用いてブレンド率（フェード率）を計算し、状態として保持・更新するメソッド（例: `Update()`）を提供する。

### 要件4: 司令塔 (Application) および 描画層 (Renderer) との結線
    - `Application` で `BackgroundManager` のインスタンスを保持・初期化し、曲の切り替え時などに画像のロード要求を発行する。
    - `Renderer` は背景描画時に `BackgroundManager` から現在の画像・過去の画像・フェード率を取得し、Renderer内のWIC/D2Dコンテキストに沿って描画を行う（Renderer側は状態を持たない完全な受動態とする）。

## 3. 実装タスクリスト
- [x] タスク1: BackgroundManagerクラスの基盤作成
    - `BackgroundManager.h / cpp` の新規作成およびCMakeLists.txtへの追加。非同期ワーカーの骨組み実装。
- [x] タスク2: FileManager連携とWICデコードの実装
    - `BackgroundManager` 内のワーカー処理にて、`FileManager` を用いたバイナリ抽出とWICデコードの実装。
- [x] タスク3: クロスフェード状態管理の実装
    - 画像切り替え時の過去画像保持およびフェード進行状態（ブレンド率）を計算するロジックの実装。
- [x] タスク4: Applicationクラスとの結線
    - `Application` での `BackgroundManager` 初期化、破棄、およびイベント時（曲変更など）のロード指示の発行。
- [x] タスク5: Rendererクラスとの結線
    - `Renderer` から `BackgroundManager` の状態を取得し、クロスフェード描画を行う処理の実装。
- [ ] タスク6: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` に `BackgroundManager` の記述を追加し、プロジェクト仕様を最新化する。

## 4. 詳細作業内容
### タスク1: BackgroundManagerクラスの基盤作成
    **【作業ファイル】**
    - `src/BackgroundManager.h` (新規作成)
    - `src/BackgroundManager.cpp` (新規作成)
    - `CMakeLists.txt` (編集)
    **【作業内容】**
    - `BackgroundManager` の基盤となるヘッダーと実装ファイルを新規作成した。
    - `std::thread`、`std::mutex`、`std::condition_variable` を用いたスレッドセーフな非同期ワーカーの骨組みを実装した。
    - `Initialize` および `Uninitialize` メソッドを実装し、安全なスレッド起動・終了機構（COM初期化・終了を含む）を構築した。
    - `CMakeLists.txt` の `SOURCES` と `HEADERS` に新規ファイルを追加し、ビルド可能な状態にした。

### タスク2: FileManager連携とWICデコードの実装
    **【作業ファイル】**
    - `src/BackgroundManager.h` (編集)
    - `src/BackgroundManager.cpp` (編集)
    **【作業内容】**
    - `src/BackgroundManager.h` に画像ロード要求を発注するインターフェース `RequestLoad` メソッドおよびキュー・WIC画像格納用変数を追加した。
    - `src/BackgroundManager.cpp` のワーカー処理（`WorkerLoop`）内にて、キューから取り出したファイルパスに対して `FileManager::ExtractAlbumArtBinary` を呼び出し、バイナリ抽出を行う処理を実装した。
    - 抽出したバイナリデータが存在する場合に、COMを用いて `IWICImagingFactory` および `CreateStreamOnHGlobal` を利用してWICでデコードを行い、`IWICFormatConverter` (PBGRA形式) の状態でスレッドセーフにクラス内部 (`m_currentWicImage`) へ保持する処理を実装した。

### タスク3: クロスフェード状態管理の実装
    **【作業ファイル】**
    - `src/BackgroundManager.h` (編集)
    - `src/BackgroundManager.cpp` (編集)
    **【作業内容】**
    - `src/BackgroundManager.h` にクロスフェード用の状態変数 `m_fadeProgress` (0.0f〜1.0f) と、古い画像を保持する `m_oldWicImage` を追加した。
    - メインスレッドとワーカースレッドの画像の受け渡しをスレッドセーフに行うため、新たに `m_nextWicImage` と `m_hasNewImage` フラグを追加した。
    - `src/BackgroundManager.cpp` に毎フレーム呼び出される `UpdateAnimation(float deltaTime)` メソッドを実装した。
    - `UpdateAnimation` 内で、新しい画像が準備されている場合に現在の画像を `m_oldWicImage` に退避し、`m_fadeProgress` を0.0fにリセットする処理を追加した。
    - さらに、0.5秒をフェード期間として `deltaTime` を用いてフェード進行度を加算し、1.0fに到達した際に `m_oldWicImage` を解放する状態遷移ロジックを実装した。

### タスク4: Applicationクラスとの結線
    **【作業ファイル】**
    - `src/Application.h` (編集)
    - `src/Application.cpp` (編集)
    - `src/Application_Initialize.cpp` (編集)
    - `src/Application_Playback.cpp` (編集)
    - `src/Application_Render.cpp` (編集)
    **【作業内容】**
    - `src/Application.h` に `BackgroundManager` をメンバ変数 (`m_backgroundManager`) として追加した。
    - `src/Application_Initialize.cpp` 内の初期化処理に `m_backgroundManager.Initialize()` を追加した。
    - `src/Application.cpp` のデストラクタにおいて、`m_backgroundManager.Uninitialize()` を呼び出して安全に破棄するようにした。
    - `src/Application_Render.cpp` の更新処理 (`ForceRender` メソッド内) にて毎フレーム `m_backgroundManager.UpdateAnimation(0.016f)` を呼び出すように結線した。
    - `src/Application_Playback.cpp` 内の曲決定時 (`PlayCurrentTrack` 内) で `m_backgroundManager.RequestLoad(track)` を呼び出し、背景画像の非同期ロードを発注するように結線した。

### タスク5: Rendererクラスとの結線
    **【対象ファイル】**
    - `src/BackgroundManager.h` : 更新
    - `src/Renderer.h` : 更新
    - `src/Renderer.cpp` : 更新
    - `src/Renderer_Draw.cpp` : 更新
    - `src/Application_Initialize.cpp` : 更新
    **【作業内容】**
    - `src/BackgroundManager.h` に現在画像、過去画像、フェード率を取得するためのゲッターメソッドを追加した。
    - `src/Renderer.h` と `src/Renderer.cpp` に `SetBackgroundManager` メソッドを追加し、ポインタを受け取って保持するように実装した。
    - また `Renderer.h` に `ID2D1Bitmap` 型のキャッシュ変数（現在・過去）を追加した。
    - `src/Renderer_Draw.cpp` の `DrawBackground` において、`BackgroundManager` が保持する WIC 画像をUIスレッド上で `CreateBitmapFromWicBitmap` を用いて D2D ビットマップに動的変換しキャッシュするロジックを実装した。
    - `DrawBackground` にて、古い画像が存在する場合は下地として描画し、現在の画像をその上にフェード率を不透明度としてクロスフェード描画する処理を復活させた。
    - `src/Application_Initialize.cpp` 内の初期化処理で `m_renderer.SetBackgroundManager(&m_backgroundManager)` を呼び出し、結線を完了させた。

### タスク6: ドキュメントの更新
    **【対象ファイル】**
    - `PROJECT_ARCHITECTURE.md` : 更新
    **【作業内容】**
    - 
