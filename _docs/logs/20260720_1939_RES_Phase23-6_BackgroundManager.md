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
- [ ] タスク1: BackgroundManagerクラスの基盤作成
    - `BackgroundManager.h / cpp` の新規作成およびCMakeLists.txtへの追加。非同期ワーカーの骨組み実装。
- [ ] タスク2: FileManager連携とWICデコードの実装
    - `BackgroundManager` 内のワーカー処理にて、`FileManager` を用いたバイナリ抽出とWICデコードの実装。
- [ ] タスク3: クロスフェード状態管理の実装
    - 画像切り替え時の過去画像保持およびフェード進行状態（ブレンド率）を計算するロジックの実装。
- [ ] タスク4: Applicationクラスとの結線
    - `Application` での `BackgroundManager` 初期化、破棄、およびイベント時（曲変更など）のロード指示の発行。
- [ ] タスク5: Rendererクラスとの結線
    - `Renderer` から `BackgroundManager` の状態を取得し、クロスフェード描画を行う処理の実装。
- [ ] タスク6: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` に `BackgroundManager` の記述を追加し、プロジェクト仕様を最新化する。

## 4. 詳細作業内容
### タスク1: BackgroundManagerクラスの基盤作成
    **【対象ファイル】**
    - `src/BackgroundManager.h` : 新規作成
    - `src/BackgroundManager.cpp` : 新規作成
    - `CMakeLists.txt` : 更新
    **【作業内容】**
    - 

### タスク2: FileManager連携とWICデコードの実装
    **【対象ファイル】**
    - `src/BackgroundManager.h` : 更新
    - `src/BackgroundManager.cpp` : 更新
    **【作業内容】**
    - 

### タスク3: クロスフェード状態管理の実装
    **【対象ファイル】**
    - `src/BackgroundManager.h` : 更新
    - `src/BackgroundManager.cpp` : 更新
    **【作業内容】**
    - 

### タスク4: Applicationクラスとの結線
    **【対象ファイル】**
    - `src/Application.h` : 更新
    - `src/Application_Initialize.cpp` : 更新
    - `src/Application_Playback.cpp` : 更新
    - `src/Application.cpp` : 更新
    **【作業内容】**
    - 

### タスク5: Rendererクラスとの結線
    **【対象ファイル】**
    - `src/Renderer.h` : 更新
    - `src/Renderer_Draw.cpp` : 更新
    **【作業内容】**
    - 

### タスク6: ドキュメントの更新
    **【対象ファイル】**
    - `PROJECT_ARCHITECTURE.md` : 更新
    **【作業内容】**
    - 
