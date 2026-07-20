# RES: 実装計画・作業レポート Phase 23-1: TagManagerの純化と既存背景アート機構の完全解体

## 1. 実装目的
今後の「背景マネージャ」新設および「背景クロスフェード」実装に向けた事前準備として、現在の `TagManager` による重いフル解像度アルバムアート（APIC）の抽出機能をシステムから完全にパージ（削除）する。
中途半端な互換性維持やサムネイル画像での代用は一切行わず、「どこに背景に関連する神経（コード）が通っているか」を1つ残らずリストアップ・解体し、純化することを目的とする。本フェーズ中は関連する依存コードの切断に伴う一時的なビルド破損を許容する。

## 2. 関連するフレーミング操作・背景関連処理（移譲・結線の準備カタログ）
本解体作業に伴い、将来「背景マネージャ」へ移譲・再結線すべき関連コード（接点）は以下の通り。

* **ConfigManager / 設定関連**:
  * `m_backgroundArtMode`, `GetBackgroundArtMode`, `SetBackgroundArtMode`。背景の描画モード設定。
* **ArtFramingDatabase**: 
  * 背景画像ごとのフレーミング（X, Y, Scale）を保持するデータベース。現在は `Application_Playback.cpp` で曲決定時に読み込み、`Renderer` へ渡されている。
* **LayoutCalculator**: 
  * `CalculateBackgroundLayout` (背景の描画領域・クリップ計算)
  * `CalculateArtFramingBounds`
  * `BackgroundLayout` 構造体
* **Window_Mouse / イベント関連**: 
  * 右クリックドラッグによるフレーミング移動 (`m_isArtFramingDragging`, `m_onArtFramingMove`)
  * Shift+ホイールによる拡縮 (`m_onArtFramingScroll`)
  * 中クリックによるリセット (`m_onArtFramingReset`)
  * ロゴメニューの背景モード切替（`ID_LOGO_BG_MODE`）

## 3. 実装タスクリスト
- [x] タスク1: TagManagerの純化（画像抽出のパージ）
- [x] タスク2: Application層のロード処理のパージ
- [x] タスク3: Renderer層の背景アート機構の完全パージ
- [x] タスク4: 潜在的な背景読み込みロジックのパージ
- [x] タスク5: ThumbCacher内のサムネイル生成時画像抽出処理のパージ

## 4. 詳細作業内容

### タスク1: TagManagerの純化（画像抽出のパージ）
#### 対象ファイル: `src/TagManager.h`, `src/TagManager.cpp`
#### 対応:
- `Load` メソッドの `skipImage` 引数を削除。常にテキスト情報のみを抽出するように純化する。
- `GetAlbumArtBytes()` メソッドおよび戻り値となる `m_albumArtBytes` メンバ変数を削除。
- `ExtractAlbumArtBinary()` メソッドを削除。
- TagLib を用いた APIC (Picture) フレームの探索・抽出ロジック（`Load` および `ExtractAlbumArtBinary` 内）を全て削除。テキスト情報（曲名・アーティスト名）の抽出のみを行う純粋なクラスへと純化する。

    #### HOTFIX1 (タスク1完了報告)
    ##### 原因・理由: TagManagerの純化と画像抽出ロジックの完全解体
        - 将来の背景マネージャ新設とクロスフェード実装に向け、重いフル解像度アルバムアート（APIC）抽出機能をTagManagerから完全にパージするため。

    ##### 対象ファイル: 
        - `src/TagManager.h`
        - `src/TagManager.cpp`

    ##### 対応: 画像抽出関連のメンバ変数およびメソッドを完全削除
        - `TagManager.h`: `Load` メソッドから `skipImage` 引数を削除。`GetAlbumArtBytes` と `ExtractAlbumArtBinary` の定義、および `m_albumArtBytes` メンバ変数を削除。
        - `TagManager.cpp`: TagLibの不要なインクルードを削除。`Load` メソッド内のAPICフレーム取得ロジックと `m_albumArtBytes` へのアサイン処理を削除し、テキスト情報（曲名、アーティスト名、再生時間）のみを取得するよう純化。`GetAlbumArtBytes` と `ExtractAlbumArtBinary` の実装を削除。

### タスク2: Application層のロード処理のパージ
#### 対象ファイル: `src/Application_Playback.cpp`, `src/Application_Initialize.cpp`
#### 対応:
- `Application_Playback.cpp` の `PlayCurrentTrack` 内において、`TagManager::GetAlbumArtBytes()` を呼び出し、得られたバイナリを用いて `m_renderer.LoadBitmapFromMemory` で画像をデコードする一連の処理を完全削除する。
- 同メソッド内での `m_renderer.SetBackgroundArt(artBitmap.Get());` の呼び出しを削除する。
- 同メソッド内での `m_framingDb.GetFraming` および `m_renderer.SetBackgroundFraming` の呼び出しについて、Renderer側の口が塞がれるため呼び出しを削除または無効化する。
- `Application_Initialize.cpp` 内の `SetupCallbacks` において、`m_window.SetArtFramingMoveCallback`、`SetArtFramingScrollCallback`、`SetArtFramingResetCallback` 内で呼ばれている `m_renderer.SetBackgroundFraming` への伝達処理（および `ClampArtFraming`）を削除または無効化する。

    #### HOTFIX2 (タスク2完了報告)
    ##### 原因・理由: Application層の背景画像ロードと設定伝達処理の完全パージ
        - Renderer側の機能撤去に向け、Application層からRendererに対する背景アートの描画・フレーミング設定の伝達に関するパイプを切断するため。

    ##### 対象ファイル: 
        - `src/Application_Playback.cpp`
        - `src/Application_Initialize.cpp`

    ##### 対応: ロードとデコード、およびRendererへの伝達処理の削除・無効化
        - `Application_Playback.cpp`: `PlayCurrentTrack`において、`TagManager`からの画像抽出と`m_renderer.LoadBitmapFromMemory`によるデコード、`SetBackgroundArt`の呼び出しを完全削除。また、フレーミング設定の取得と`SetBackgroundFraming`呼び出しを削除。
        - `Application_Initialize.cpp`: `SetupCallbacks`における各種フレーミング操作コールバック内で、`m_renderer.ClampArtFraming`および`m_renderer.SetBackgroundFraming`への伝達処理をコメントアウトし無効化。

### タスク3: Renderer層の背景アート機構の完全パージ
#### 対象ファイル: `src/Renderer.h`, `src/Renderer.cpp`, `src/Renderer_Draw.cpp`
#### 対応:
- `Renderer.h` および `Renderer.cpp` から `SetBackgroundArt`, `SetBackgroundFraming`, `ClampArtFraming` メソッドの実装を削除する。
- `Renderer.h` から `m_backgroundArtBitmap` （背景画像保持用）, `m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale` メンバ変数を削除する。
- `Renderer_Draw.cpp` の `DrawBackground` メソッド内において、`ID2D1Bitmap` を用いた画像描画ロジックとそれに伴うレイアウト取得処理を完全削除する。
- 指示に従い、同メソッド内の `m_config->GetBgDarkenOpacity()` と `m_bgDarkenBrush` を用いたダークオーバーレイ（`FillRectangle`）の描画処理のみは残す。

    #### HOTFIX3 (タスク3完了報告)
    ##### 原因・理由: Renderer層からの背景アート描画・関連機能の完全パージ
        - 将来の「背景マネージャ」への移譲に備え、現行のRenderer層におけるフル解像度アルバムアートの直接保持・描画、フレーミング設定の各種メソッドおよびメンバ変数を完全に解体し、接点マーカーを残すため。

    ##### 対象ファイル: 
        - `src/Renderer.h`
        - `src/Renderer.cpp`
        - `src/Renderer_Draw.cpp`

    ##### 対応: 背景画像描画ロジックおよび関連メンバ・メソッドのパージと結線マーカー付与
        - `Renderer.h`: `SetBackgroundArt`, `SetBackgroundFraming`, `ClampArtFraming` の定義、および `m_backgroundArtBitmap`, `m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale` を削除し、後日結線用のマーカーコメントを付与。
        - `Renderer.cpp`: `SetBackgroundArt`, `SetBackgroundFraming`, `ClampArtFraming` の実装を完全に削除し、後日結線用のマーカーコメントを付与。
        - `Renderer_Draw.cpp`: `DrawBackground` メソッド内において、画像描画ロジック（`DrawBitmap` など）とレイアウト計算処理を削除し結線用のマーカーコメントを付与。ダークオーバーレイ描画の処理のみ残存させた。

### タスク4: 潜在的な背景読み込みロジックのパージ
#### 対象ファイル: `src/Renderer.h`, `src/Renderer_Image.cpp`
#### 対応:
- 全コード検索の結果、背景画像のオンメモリロード用途専用として `LoadBitmapFromMemory` が存在している。これを `Renderer.h` および `Renderer_Image.cpp` から完全に削除する。

    #### HOTFIX4 (タスク4完了報告)
    ##### 原因・理由: 潜在的な背景読み込みロジックのパージ
        - 背景画像のオンメモリロード用途としてのみ利用されていた `LoadBitmapFromMemory` を不要コードとして安全に削除するため。

    ##### 対象ファイル: 
        - `src/Renderer.h`
        - `src/Renderer_Image.cpp`

    ##### 対応: 未使用のデコードメソッドの削除
        - `Renderer.h`: `LoadBitmapFromMemory` の定義を削除。
        - `Renderer_Image.cpp`: WICを用いたメモリストリームからのデコード処理である `LoadBitmapFromMemory` の実装本体を完全に削除。

### タスク5: ThumbCacher内のサムネイル生成時画像抽出処理のパージ
#### 対象ファイル: `src/ThumbCacher.cpp`
#### 対応:
- `ThumbCacher.cpp` の処理ループ内において、サムネイル生成のために `TagManager::ExtractAlbumArtBinary(filepath)` を呼び出している箇所が存在する。
- タスク1での `ExtractAlbumArtBinary` 削除に伴い、この呼び出し処理自体もパージ（削除）する。一時的にサムネイル生成機能が完全に機能しなくなる（ビルド破損もしくは実行時空回り）が、制約事項に従い妥協的な代替処理は組み込まず、そのまま削ぎ落とす。

    #### HOTFIX5 (タスク5完了報告)
    ##### 原因・理由: ThumbCacherからの画像抽出とサムネイル生成処理のパージ
        - 将来の新しいサムネイル・背景連携パイプラインの構築に向け、一時的なサムネイル生成不能を許容してでも、現在の画像抽出およびサムネイル生成への依存を更地にするため。

    ##### 対象ファイル: 
        - `src/ThumbCacher.cpp`

    ##### 対応: サムネイル生成関連ロジックの完全パージ
        - `ThumbCacher.cpp`: `WorkerLoop`内において、`TagManager::ExtractAlbumArtBinary` による生バイナリの抽出処理、およびそれに続くWICを用いたサムネイル画像への変換（`CookThumbnailImage`）、最終的なデータベースへの保存（`StoreCookedData`）という一連の処理ブロックを完全に削除した。
