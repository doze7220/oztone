# RES:HOTFIX作業レポート: 背景描画パイプラインの完全開通とフレーミング再結線

## 1. 実装目的
Phase 23-6 にて新設された `BackgroundManager` 稼働に伴い発生した「背景画像およびデフォルト背景が表示されない不具合」を解消するため、切断されたパイプライン（スレッド初期化、フレーミングデータ取得・受け渡し、デフォルト背景フォールバック描画）の再結線および修復を行う。

## 2. 調査内容
1. `BackgroundManager.cpp` 内の `WorkerLoop` にて、`CoInitializeEx` の戻り値 `hr` が、後続の WIC ファクトリ生成処理などによって上書きされ、スレッド終了時に正しい判定で `CoUninitialize()` が呼ばれないバグが発覚した。
2. 背景画像表示位置などのフレーミング情報 (`ArtFramingDatabase` からの取得) が、旧来のレンダラ統合アーキテクチャ解体に伴い完全に途絶え、描画時に利用できなくなっていた。
3. `Renderer_Draw.cpp` の `DrawBackground` において、WIC画像が無い（`nullptr` の）場合における `IDI_PLACEHOLDER_ART` （デフォルト背景）へのフォールバック描画ロジックが欠落していた。
4. キャッシュ機構（WICからD2Dビットマップへの変換）は、ポインタの一致判定に加えて初期起動時（非null判定）のチェックが必要だった。

## 3. 対象ファイル
*   `src/BackgroundManager.cpp`
*   `src/Application_Playback.cpp`
*   `src/Renderer.h`
*   `src/Renderer.cpp`
*   `src/Renderer_Draw.cpp`

## 4. 実装タスクリスト
- [x] タスク1: ワーカースレッドのCOM初期化（デコード失敗の修復）
- [x] タスク2: ArtFramingDatabase の再結線（フレーミング情報の復活）
- [x] タスク3: デフォルト背景（プレースホルダー）描画の復活とキャッシュ機構の適正化

## 5. 詳細作業内容
* タスク1: ワーカースレッドのCOM初期化（デコード失敗の修復）
    - `src/BackgroundManager.cpp` にて、`CoInitializeEx` の戻り値を `hrInit` という別変数で保持するように修正し、ループ内で `hr` が上書きされてもスレッド終了時に確実に `CoUninitialize()` が呼ばれるように保護を掛けた。
* タスク2: ArtFramingDatabase の再結線（フレーミング情報の復活）
    - `src/Application_Playback.cpp` の `PlayCurrentTrack` にて、`m_framingDb.GetFraming` で曲ごとのフレーミング（X, Y, Scale）を取得し、`Renderer` に伝達する処理を復活させた。
    - `src/Renderer.h` / `.cpp` に `SetBackgroundFraming` メソッドとメンバ変数群 (`m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale`) を追加し、情報を受け取れるようにした。
    - `src/Renderer_Draw.cpp` 内で `LayoutCalculator::CalculateBackgroundLayout` を呼び出す際、このフレーミング情報を引数として正しく渡すように修正した。
* タスク3: デフォルト背景（プレースホルダー）描画の復活とキャッシュ機構の適正化
    - `src/Renderer_Draw.cpp` の `DrawBackground` において、`BackgroundManager` から取得した `currentWic` または `oldWic` が `nullptr` であった場合、`m_placeholderArtBitmap` （デフォルト背景）をフォールバックとしてD2Dビットマップキャッシュ (`m_currentBgBitmap`, `m_oldBgBitmap`) に割り当てるロジックを実装した。
    - また、毎フレームの無駄なD2Dビットマップ生成を防ぐため、「WICポインタが変化した時」または「現在のD2Dビットマップが未初期化の時 (`!m_currentBgBitmap`)」にのみキャッシュ更新を行う最適化判定を導入した。
