# RES:実装計画・作業レポート Phase 24-10: プレースホルダ背景の統合とフレーミング保存ロック

## 1. 実装目的
アートワークが存在しない楽曲の再生時において、BackgroundManager 内部で WIC 画像としてのプレースホルダ（フォールバック）を透過的に扱うことで、既存の Renderer の描画ロジックやフラグ管理に一切の変更を加えることなくクロスフェードを機能させる。同時に、司令塔である Application 層でプレースホルダ状態を判定・管理し、意図しないフレーミング情報（拡大率・オフセット）のデータベースへの保存や適用を完全にロック（防止）する。

## 2. アーキテクチャ設計
### 要件1: BackgroundManager へのプレースホルダWIC画像保持とフォールバック適用
    - `BackgroundManager` クラスにプレースホルダ用の WIC 画像を保持するメンバ `m_placeholderWic` を追加する。
    - 初期化処理 (`Initialize`) において、WIC ファクトリを生成し、リソース (`IDI_PLACEHOLDER_ART` 等) から画像をデコードして `m_placeholderWic` に保持させる。
    - ワーカースレッド (`WorkerLoop`) において、要求されたファイルパスが空の場合や、画像の抽出・デコードに失敗した場合に、保持している `m_placeholderWic` を `m_nextWicImage` に割り当てる（フォールバック）。これにより Renderer 側は通常の画像と区別なくクロスフェード処理を続行できる。

### 要件2: Applicationでの画像判定とフラグ管理
    - `Application` クラスに、現在の背景がプレースホルダであるかを示すフラグ `m_isCurrentBackgroundPlaceholder` (初期値 false) を追加する。
    - `PlayCurrentTrack` メソッドにおいて、楽曲のアートワーク有無を事前に判定する。画像が存在しない場合（`FileManager::ExtractAlbumArtBinary` が空を返す等）は `BackgroundManager::RequestLoad(L"")` のように空パスを渡し、フラグを `true` に設定する。画像が存在する場合は通常通りパスを渡し、フラグを `false` とする。

### 要件3: フレーミング保存の完全ロック
    - `Application_Initialize.cpp` 内で登録しているフレーミング関連のコールバック（`SetArtFramingMoveCallback`、`SetArtFramingScrollCallback`、`SetArtFramingResetCallback`）において、コールバック実行時に `m_isCurrentBackgroundPlaceholder` が `true` の場合は、DB への書き込み（`SetFraming`）および `BackgroundManager` への適用を即座にスキップ（return）する制御を追加する。

## 3. 実装タスクリスト
- [x] タスク1: BackgroundManager のプレースホルダWIC保持とフォールバック処理の実装
    - `BackgroundManager.h` および `BackgroundManager.cpp` を修正し、リソースからの初期化とワーカーでのフォールバックロジックを組み込む。
- [x] タスク2: Application へのフラグ追加と判定ロジックの実装
    - `Application.h` に `m_isCurrentBackgroundPlaceholder` フラグを追加する。
    - `Application_Playback.cpp` (`PlayCurrentTrack`) で画像有無の判定を行い、フラグの更新および BackgroundManager への要求パスを制御する。
- [x] タスク3: フレーミング保存と操作のロック適用
    - `Application_Initialize.cpp` 内のフレーミング操作コールバック内にフラグ判定を導入し、プレースホルダ時の操作を弾くロジックを追加する。

## 4. 詳細作業内容
### タスク1: BackgroundManager のプレースホルダWIC保持とフォールバック処理の実装
    **【対象ファイル】**
    - `src/BackgroundManager.h` : 更新
    - `src/BackgroundManager.cpp` : 更新
    **【作業内容】**
    - `BackgroundManager.h` に `m_placeholderWic` (`Microsoft::WRL::ComPtr<IWICBitmapSource>`) を追加。
    - `BackgroundManager::Initialize` 内にて、`IDI_PLACEHOLDER_ART` リソースからメモリ上に画像をロードし、自前で `IWICBitmapDecoder` および `IWICFormatConverter` を構築して `m_placeholderWic` に保持。直後に `m_oldWicImage` に格納し、起動時のスタートアップ演出となるよう実装。
    - `RequestLoad` において、`filePath` が空の場合はワーカーを介さず即座に `m_nextWicImage` へ `m_placeholderWic` をセットし、`m_hasNewImage = true` として通常のクロスフェードを開始させる即時フォールバックロジックを追加。
    - `WorkerLoop` において、画像デコードに失敗した場合に `m_placeholderWic` をフォールバック画像としてセットするロジックを追加。

### タスク2: Application へのフラグ追加と判定ロジックの実装
    **【対象ファイル】**
    - `src/Application.h` : 更新
    - `src/Application_Playback.cpp` : 更新
    **【作業内容】**
    - `Application.h` に `m_isCurrentBackgroundPlaceholder` フラグを追加した。
    - `Application_Playback.cpp` (`PlayCurrentTrack`) で `FileManager::ExtractAlbumArtBinary` を呼び出して画像有無を判定し、フラグの更新および BackgroundManager へ空パス（`L""`）またはファイルパスを渡すよう実装した。

### タスク3: フレーミング保存と操作のロック適用
    **【対象ファイル】**
    - `src/Application_Initialize.cpp` : 更新
    **【作業内容】**
    - `Application_Initialize.cpp` のフレーミング操作コールバック (`SetArtFramingMoveCallback`, `SetArtFramingScrollCallback`, `SetArtFramingResetCallback`) の冒頭に `m_isCurrentBackgroundPlaceholder` の判定を追加し、プレースホルダ表示中の操作（DB保存など）を即座に弾くよう実装した。
