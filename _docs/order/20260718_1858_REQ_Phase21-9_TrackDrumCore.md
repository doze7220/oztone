### 作業指示書 REQ: Phase 21-9: 相対座標ドラムエンジンとオンデマンド描画
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-9_RelativeDrumCore.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
ドラムエンジンの座標管理を「絶対インデックス」から「目標を 0.0f とした相対距離」へ完全に移行し、描画側が必要なメタデータをプレイリストへ都度問い合わせる（オンデマンド取得）最強のデカップリングアーキテクチャを構築する。

*   **要件1: Rendererの相対座標化とインターフェース改修**
    *   `src/Renderer.h` および `.cpp` の `SetTrackInfo` を廃止（または内部ロジックを全面改修）し、新たに `SetDrumTarget(int relativeDistance)` を受け取るようにする。
    *   ドラムの現在位置を示すメンバ変数を `float m_drumRelativePosition;` とし、`SetDrumTarget` 呼び出し時に `m_drumRelativePosition += static_cast<float>(relativeDistance);` のように相対距離を加算（ワープ）させる。
*   **要件2: アニメーション目標の 0.0f 固定化**
    *   `src/Renderer_Update.cpp` の `UpdateAnimation` にて、`m_drumRelativePosition` が常に `0.0f` に向かって減衰（S字カーブ等）するように物理演算を書き換える。
*   **要件3: Widget_TrackInfo のオンデマンド描画 (都度聞く)**
    *   仮想スロットの描画ループ変数 `i` は「目標地点からの相対インデックス」となる。
    *   描画時、`int absIndex = ctx.currentTrackIndex + i;` を計算し、これを `ctx.totalTracks` でモジュロ演算（ループ）して正規化する。
    *   正規化されたインデックスを用いて `ctx.shuffleMetadataList` から直接「曲名、アーティスト名、画像」を取得して描画する。
    *   ※Phase 21-8 タスク4・5で実装されたガラス板（非同期ロード）や背景同期の演出ロジックは維持しつつ、データの取得元を `OLD/NOW` スナップショットから上記のオンデマンド取得へ完全に繋ぎ直すこと。
*   **要件4: Application層からの相対操作の伝達**
    *   `Application` 層から `Renderer` へ指示を出す際、純粋な「相対距離」だけを伝える。
        *   `Next` (曲終了, メディアキー, リスト切替): `relativeDistance = -1`
        *   `Prev`: `relativeDistance = 1`
        *   `Jump` (プレイリスト内ジャンプ): `relativeDistance = oldIndex - newIndex` (例: 5から15へのジャンプなら -10)
        *   `Reset` (UIクリア等): `relativeDistance = 0`
*   **要件5: 負の遺産（バケツリレー）の完全パージ**
    *   `m_oldDrumSlot`, `m_nowDrumSlot` 等のバケツリレー用変数はアーキテクチャから完全に削除すること。

#### 【絶対遵守ルール (Constraints)】
*   **OLD/NEWバケツリレーの禁止**: Renderer内に過去データを退避させる変数は絶対に作成・維持しないこと。常に相対インデックスから逆算してプレイリスト情報を直接覗き見ること。

----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 1 : Renderer層の相対座標化とインターフェース変更
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2038_RES_Phase21-9_RelativeDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはドラムエンジン相対座標化の第1ステップ（Renderer側のインターフェースと変数整理）である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_2038_RES_Phase21-9_RelativeDrumCore.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「Renderer層のインターフェース変更(Task 1)が完了しました。一時的なビルドエラーは想定内ですので、そのままタスク2へ進みます」と報告すること。

#### 【実装要件】
ドラムエンジンのバケツリレー設計を完全に破棄し、相対距離のみを受け取る純粋な物理インターフェースに作り替える。

*   **要件1: Renderer.h の純化とインターフェース変更**
    *   `src/Renderer.h` に存在している `SetTrackInfo` メソッドの宣言を削除する。
    *   代わりに `void SetDrumTarget(int relativeDistance);` の宣言を追加する。
    *   旧アーキテクチャの負の遺産である `m_oldDrumSlot`, `m_nowDrumSlot` などの過去データを退避させるバケツリレー用変数をすべて削除する。
    *   ドラムの現在位置を示す変数として `float m_drumRelativePosition = 0.0f;` を定義する。

*   **要件2: Renderer.cpp の SetDrumTarget 実装**
    *   `src/Renderer.cpp` の旧 `SetTrackInfo` の実装を削除し、代わりに `SetDrumTarget(int relativeDistance)` の実装を追加する。
    *   内部処理は純粋な物理座標のワープのみとし、`m_drumRelativePosition += static_cast<float>(relativeDistance);` を実行する。

#### 【絶対遵守ルール (Constraints)】
*   **バケツリレーの禁止**: Renderer内に過去の曲データを退避させる処理や変数は絶対に復元しないこと。
*   **ビルドエラーの許容**: 本タスク実行により `Application` 層などから呼び出している `SetTrackInfo` が見つからなくなりビルドエラーとなるが、これはタスク4で改修するため、本タスク内でのエラー解消を目的とした `Application` 層のコード修正は絶対に行わないこと。

----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 2 : 物理演算のアニメーション固定化
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2038_RES_Phase21-9_RelativeDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはドラムエンジン相対座標化の第2ステップ（物理演算の修正）である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_2038_RES_Phase21-9_RelativeDrumCore.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「物理演算のアニメーション固定化(Task 2)が完了しました。引き続きビルドエラーは無視して、タスク3(Widget層の描画改修)へ進みます」と報告すること。

#### 【実装要件】
ドラムの物理演算を絶対座標ベースから、常に `0.0f`（目標地点）に向かって収束する相対座標ベースへと書き換える。

*   **要件1: Renderer_Update.cpp のアニメーション修正**
    *   `src/Renderer_Update.cpp` の `UpdateAnimation` メソッドにおけるトラックドラムの位置更新処理を改修する。
    *   `currentTrackIndex` 等を用いた絶対座標での目標位置計算ロジックをすべて削除する。
    *   代わりに、`m_drumRelativePosition` が毎フレーム `0.0f` に向かってイージング（減衰）する物理計算を実装する。
        （例: `m_drumRelativePosition += (0.0f - m_drumRelativePosition) * 減衰係数;` 既存のアニメーションカーブ・速度を踏襲すること）
    *   計算後、`m_drumRelativePosition` が十分に `0.0f` に近づいた場合（例: 絶対値が `0.001f` 未満）は、微小振動を防ぐため完全に `0.0f` にスナップさせる処理を含めること。

#### 【絶対遵守ルール (Constraints)】
*   **絶対インデックスの完全排除**: アニメーションの目標値計算において、プレイリストのインデックスなど外部の情報を一切参照してはならない。ドラムの帰るべき場所は常に `0.0f` である。
*   **ビルドエラーの許容**: 本タスクにおいても `Application` 層等でのビルドエラーは継続するが、エラー解消を目的とした他ファイルへの修正は絶対に行わないこと。

----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 3 : Widget_TrackInfo のオンデマンド描画対応
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2038_RES_Phase21-9_RelativeDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはドラムエンジン相対座標化の第3ステップ（描画層のオンデマンド参照化）である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_2038_RES_Phase21-9_RelativeDrumCore.md）の「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「Widget層の描画改修(Task 3)が完了しました。引き続きApplication層の連携修正(Task 4)へ進みます」と報告すること。

#### 【実装要件】
描画ループを相対座標ベースに書き換え、プレイリストのメタデータリストをモジュロ演算を用いて直接参照する「オンデマンド描画（都度聞く）」アーキテクチャへと改修する。

*   **要件1: Widget_TrackInfo の相対スロット描画**
    *   `src/Widget_TrackInfo.cpp` の仮想スロット描画ループにおいて、変数 `i` を「0.0f を基準とした相対スロットインデックス」として扱う。
    *   ループ内で描画すべき絶対インデックスを `int absIndex = static_cast<int>(ctx.currentTrackIndex) + i;` で算出する。
    *   算出された `absIndex` をプレイリストの総曲数 (`ctx.totalTracks`) でモジュロ演算し、負の値も考慮して正規化する（例: `int normalizedIndex = (absIndex % totalTracks + totalTracks) % totalTracks;`）。
    *   正規化されたインデックスを用いて、`ctx.shuffleMetadataList[normalizedIndex]` から曲名、アーティスト名、トラック番号などのメタデータを直接取得し、描画する。
*   **要件2: 演出（ガラス板等）のデータ参照先切り替え**
    *   Phase 21-8 で実装された、非同期ロード時のガラス板描画やクロスフェード演出のロジックは維持すること。
    *   ただし、画像データの参照元を `ctx.nowDrumSlot.artBitmap` 等のバケツリレー変数から、`ctx.shuffleMetadataList` （および必要に応じて `ctx.currentArtBitmap` や、バックグラウンドロード中の場合はプレースホルダー等）から取得するように配線を繋ぎ直す。
*   **要件3: 不要変数のパージ (WidgetContext)**
    *   `src/WidgetContext.h` から、バケツリレーの残骸である `DrumSlotData` 構造体、および `oldDrumSlot`, `nowDrumSlot` を完全に削除する。

#### 【絶対遵守ルール (Constraints)】
*   **バケツリレー変数の使用禁止**: `Widget_TrackInfo.cpp` 内において、`oldDrumSlot` や `nowDrumSlot` などの過去/目標の退避データに依存した描画分岐は絶対に行わないこと。必ず相対インデックスとモジュロ演算を用いたリスト直接参照のみで描画すること。
*   **ビルドエラーの許容**: 本タスク完了時点でも、`Application` 層の `SetTrackInfo` 呼び出しに起因するビルドエラーは残存する。`Application` 層のコード修正は絶対に行わないこと。

----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 4 : Application層からの相対操作の伝達
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2038_RES_Phase21-9_RelativeDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはドラムエンジン相対座標化の最終ステップ（Application層からの指示の相対化）である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_2038_RES_Phase21-9_RelativeDrumCore.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「Application層からの相対距離伝達(Task 4)が完了しました。全てのビルドエラーが解消され、相対座標ドラムエンジンが完成しました。動作確認をお願いします！」と報告すること。

#### 【実装要件】
Application層からRendererへ曲の切り替えやジャンプを指示する部分を、すべて純粋な「相対距離（`relativeDistance`）」を用いた `SetDrumTarget` の呼び出しへ置き換える。

*   **要件1: PlayCurrentTrack のシグネチャ変更**
    *   `src/Application.h` および `src/Application_Playback.cpp` にある `PlayCurrentTrack` メソッドの引数を、相対距離を受け取るように変更する（例: `bool PlayCurrentTrack(int relativeDistance = -1);` ※デフォルトはNextとする）。
    *   `PlayCurrentTrack` の内部にて、`m_renderer.SetDrumTarget(relativeDistance);` を呼び出してドラムの回転目標を指示する。
*   **要件2: 各操作からの相対距離の算出と伝達**
    *   各ファイルからの `PlayCurrentTrack` 呼び出し箇所において、操作のコンテキストに応じた相対距離を渡すように修正する。
        *   **曲終了時の自動遷移や `Next` 操作**: `PlayCurrentTrack(-1)` を呼び出す（順方向へ1つ回す）。
        *   **`Prev` 操作**: `PlayCurrentTrack(1)` を呼び出す（逆方向へ1つ回す）。
        *   **プレイリスト内のクリックによるジャンプ**: `JumpToIndex` 実行時、移動前のインデックス (`oldIndex`) と移動後のインデックス (`newIndex`) から `relativeDistance = static_cast<int>(oldIndex) - static_cast<int>(newIndex);` を算出し、`PlayCurrentTrack(relativeDistance)` を呼び出す。
        *   **プレイリストの切り替え (SwitchPlaylist)**: 新しいリストの先頭から始まるため、`PlayCurrentTrack(-1)` とし、UI上は「新しいリストが上から降ってくる」ように見せる。
*   **要件3: UIリセット等の対応**
    *   `ClearPlaylist` 時や初期起動時など、UIをリセット・クリアしたい場合は、`m_renderer.SetDrumTarget(0);` を明示的に呼び出し、ドラム位置をリセットする。旧来の「"NO TRACK" 文字列を投げる」ような処理は廃止する。

#### 【絶対遵守ルール (Constraints)】
*   **バケツリレーの残骸の排除**: Application層に残っている「OLDの文字列」や「古いインデックス」を退避・引数として渡すような変数がもしあれば、すべてパージすること。
*   **ビルドエラーの完全解消**: 本タスク完了をもって、タスク1から続いていた `SetTrackInfo` 等に起因するすべてのビルドエラーが解消され、正常にビルドが通る状態にすること。

----------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Hotfix 1 : バケツリレー残骸の完全破壊と真のオンデマンド描画の強制適用
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2038_RES_Phase21-9_RelativeDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは、AIエージェントの指示無視によって残留した「NEW→OLDのコピー処理」を完全に破壊し、真のオンデマンド描画を強制適用する Hotfix の「実装実行」である。
2. 作業完了後、チャットにて「バケツリレーの残骸を完全にパージし、プレイリスト直参照の描画ループを実装しました！」と報告すること。

#### 【実装要件】
*   **要件1: Renderer および WidgetContext からのバケツリレー残骸の完全パージ**
    *   `src/Renderer.h`, `src/Renderer.cpp`, `src/Renderer_Context.cpp`, `src/WidgetContext.h` の中を徹底的に精査し、`oldTrackTitle`, `nowTrackTitle`, `oldDrumSlot`, `nowDrumSlot` またはそれに類する**「目標（NEW）のデータを過去（OLD）へコピーして退避させている処理・変数」を1行残らずすべて削除**すること。
*   **要件2: Widget_TrackInfo の真のオンデマンド描画ループの実装**
    *   `src/Widget_TrackInfo.cpp` の仮想スロット描画ループ（`i = -2` から `2` 等）にて、OLDやNEWといった退避変数は**絶対に参照しない**こと。
    *   描画すべき絶対インデックスを `int absIndex = static_cast<int>(ctx.currentTrackIndex) + i;` で求める。
    *   プレイリストの総曲数 (`ctx.totalTracks`) でモジュロ演算（ループ丸め）を行い、`normalizedIndex` を算出する。
    *   `ctx.shuffleIndices[normalizedIndex]` を経由して実インデックスを取得し、`ctx.shuffleMetadataList` からメタデータ（曲名、アーティスト名）を直接取得して描画する。
*   **要件3: アルバムアート画像のオンデマンド描画ロジック**
    *   現在のオンデマンドアーキテクチャでは、保持している画像は `ctx.currentArtBitmap`（現在の曲の画像）のみである。
    *   描画ループ内で `i == 0`（目標スロット）の時のみ `ctx.currentArtBitmap` を描画し、それ以外のスロット（`i != 0`）では画像が存在しないため、フォールバック（ガラス板またはプレースホルダー）を描画するよう条件分岐すること。

#### 【絶対遵守ルール (Constraints)】
*   **コピー処理の禁止**: 今後いかなる場合も、Renderer層やWidget層において「曲のメタデータを別の変数へコピーして退避させる処理（バケツリレー）」を書いてはならない。常に `ctx.shuffleMetadataList` を相対インデックスから逆算して直接覗き見ること。

----------------------------------------------------------------------------------
