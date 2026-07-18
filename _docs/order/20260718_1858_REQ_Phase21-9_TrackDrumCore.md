### 作業指示書 REQ: Phase 21-9: 完全独立型トラックドラムエンジンの構築
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\order\20260718_1858_REQ_Phase21-9_TrackDrumCore.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-9_TrackDrumRebuild.md）として新規作成すること。
3. **【重要】後続の実装AIが過去の肥大化したレポート（Phase 21-8 RES）を一切読まずとも作業が完結できるよう、パージ（削除）すべき古いハック実装（具体的な変数名や処理内容）を本計画書内に詳細に明記し、この計画書単体で情報が完結する（Single Source of Truthとなる）ように作成すること。**
4. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
プレイリストのデータ構造からトラックドラムの描画データを完全に切り離し、ドラム自身が独立したスナップショットデータと操作意図を持つ「最強のバケツリレーアーキテクチャ」を構築する。
※本フェーズのベースラインはPhase 21-8のHotfix 4完了時点とする。

*   **要件1: ドラム専用データ構造と操作意図の定義**
    *   `Renderer.h` 等に、描画に必要な純粋なデータのみを持つ構造体 `DrumSlotData` (曲名、アーティスト、トラックNo文字列、画像ビットマップ等) を定義する。
    *   ドラムの回転意図を指示する列挙型 `DrumMoveType` (`Next`, `Prev`, `Jump`, `CrossPlaylist`, `Reset` 等) を定義し、`SetTrackInfo` の引数として受け取れるようインターフェースを改修する。
*   **要件2: Application層からの操作意図伝達**
    *   `Application` 層の各操作（メディアキー、ジャンプ、リスト切り替え、UIクリア等）において `SetTrackInfo` を呼び出す際、単に曲情報を渡すだけでなく、適切な `DrumMoveType` を引数として渡すように連携させる。
*   **要件3: 独立したバケツリレーと位置リセットの構築**
    *   `Renderer` 内に `m_oldDrumSlot` と `m_nowDrumSlot` を実体（コピー）として保持する。
    *   `SetTrackInfo` 呼び出し時、渡された `DrumMoveType` に応じて現在位置（`m_drumPosition`）を正確にワープさせる。
        *   `Next` または `CrossPlaylist`: `m_drumPosition = 目標 - 1.0f` (1つ手前から順方向へ進む)
        *   `Prev`: `m_drumPosition = 目標 + 1.0f` (1つ先から逆方向へ戻る)
        *   `Jump`: インデックス差分による遠距離ジャンプ（現状維持）
    *   同時に、`m_nowDrumSlot` を `m_oldDrumSlot` へ完全にコピーしてデータを退避させる、絶対的なバケツリレーを確立する。
*   **要件4: Widget_TrackInfo の描画純化**
    *   仮想スロット描画ループにおいて、OLDスロットとNOWスロットを描画する際は絶対にプレイリストデータ（`ctx.shuffleMetadataList` 等）のインデックスを参照せず、`ctx` を経由して渡された `oldDrumSlot` と `nowDrumSlot` の実体のみを使用してテキストと画像を描画するように純化する。

#### 【絶対遵守ルール (Constraints)】
*   **完全な独立**: ドラムのOLD/NOWデータは、プレイリストが破棄・再構築されても絶対に消えない完全に独立したメモリ（実体のコピー）として管理すること。

-----------------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 1 : データ構造定義とSetTrackInfoシグネチャ変更
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_1905_RES_Phase21-9_TrackDrumRebuild.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはトラックドラム再構築のためのデータ構造定義とインターフェース改修である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_1905_RES_Phase21-9_TrackDrumRebuild.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「データ構造定義とシグネチャ変更(Task 1)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
プレイリストのデータ構造からトラックドラムのデータを完全に切り離すための準備として、専用のデータ構造と、操作意図を伝達するパイプラインを構築する。

*   **要件1: ドラム専用データ構造と操作意図の定義**
    *   `src/Renderer.h` に、描画に必要な純粋なデータのみを持つ構造体 `DrumSlotData` を定義する。
        *   メンバの例: `std::wstring title`, `std::wstring artist`, `std::wstring trackNo`, `Microsoft::WRL::ComPtr<ID2D1Bitmap> artBitmap`
    *   `src/Renderer.h` に、ドラムの回転意図を指示する列挙型 `DrumMoveType` を定義する。
        *   値の例: `Next` (次へ), `Prev` (前へ), `Jump` (任意曲へのジャンプ), `CrossPlaylist` (別リスト移動/ループ), `Reset` (UIクリア等の空打ち)
*   **要件2: SetTrackInfoシグネチャの変更と安全措置**
    *   `src/Renderer.h` および `src/Renderer.cpp` にある `Renderer::SetTrackInfo` の引数を改修し、新たに `DrumMoveType moveType` を受け取るように変更する。
    *   **【重要】** `Application` 層の呼び出し修正はタスク3で行うため、本タスクでのビルドエラーを防ぐ目的で、`Renderer.h` 側の宣言には一時的にデフォルト引数（例: `DrumMoveType moveType = DrumMoveType::Jump`）を付与しておくこと。
    *   本タスクでは引数の受け取り口を追加するのみとし、`SetTrackInfo` 内部のバケツリレーや物理演算ロジックの変更は絶対に行わないこと。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳密な限定**: 本タスクは「データ構造の定義」と「シグネチャの変更（デフォルト引数付き）」のみ。Application側の呼び出し修正や、Renderer内部の物理演算ロジックには絶対に触れないこと。

-----------------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 2 : バケツリレーと位置ワープ処理の実装
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_1905_RES_Phase21-9_TrackDrumRebuild.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは完全独立型ドラムエンジンのコアロジック実装である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_1905_RES_Phase21-9_TrackDrumRebuild.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「バケツリレーと位置ワープ処理(Task 2)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: 実体データとしてのバケツリレー確立**
    *   `src/Renderer.h` に、`DrumSlotData m_oldDrumSlot;` と `DrumSlotData m_nowDrumSlot;` をメンバ変数として追加する（コンストラクタ等で適切に初期化すること）。
    *   `src/Renderer.cpp` の `SetTrackInfo` 呼び出し時、新しい情報が渡された瞬間、無条件で `m_oldDrumSlot = m_nowDrumSlot;` を実行し、既存データを完全に退避させる（絶対的なバケツリレー）。
    *   退避後、渡された曲名、アーティスト名等の引数を `m_nowDrumSlot` に保存する。（※画像ビットマップの保存については、後続の `SetAlbumArt` などの呼び出し時に `m_nowDrumSlot.artBitmap` にセットされるよう合わせて改修すること）。
*   **要件2: DrumMoveType に応じた現在位置の正確なワープ**
    *   `SetTrackInfo` にて、引数 `moveType` の意図に従い、アニメーションの目標地点（`currentTrackIndex`）に対する現在のドラム位置（`m_drumPosition`）を正確にワープさせるロジックを実装する。
        *   `DrumMoveType::Next` または `DrumMoveType::CrossPlaylist` : `m_drumPosition = static_cast<float>(currentTrackIndex) - 1.0f;` （1つ手前から順方向へアニメーションさせる）
        *   `DrumMoveType::Prev` : `m_drumPosition = static_cast<float>(currentTrackIndex) + 1.0f;` （1つ先から逆方向へアニメーションさせる）
        *   `DrumMoveType::Reset` : `m_drumPosition = static_cast<float>(currentTrackIndex);` （UIクリア時などのため、アニメーションさせず即時着地させる）
        *   `DrumMoveType::Jump` : 遠距離の回転アニメーションを表現するため、`m_drumPosition` は上書きせず現在の位置を維持する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳密な限定**: Application層の呼び出し元変更（タスク3）や、Widget_TrackInfo層での描画切り替え（タスク4, 5）には絶対に触れないこと。今回は Renderer 内部でのデータ退避と、回転アニメーション開始前の位置ワープ（仕込み）のみに限定する。


-----------------------------------------------------------------------------------------


### 作業指示書 REQ: Phase 21-9 Task 3 : Application層の操作意図(DrumMoveType)伝達
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_1905_RES_Phase21-9_TrackDrumRebuild.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは完全独立型ドラムエンジンのためのApplication層呼び出し改修である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_1905_RES_Phase21-9_TrackDrumRebuild.md）の「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「Application層の操作意図伝達(Task 3)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
Application層の各処理において `m_renderer.SetTrackInfo` を呼び出している全箇所（初期化、曲送り/戻し、プレイリスト切り替え、クリアなど）を調査し、その操作コンテキストに合致する `DrumMoveType` を明示的に引数として渡すように修正する。

*   **要件1: `PlayCurrentTrack` のシグネチャ拡張と伝播**
    *   `src/Application.h` および `src/Application_Playback.cpp` の `PlayCurrentTrack()` の引数に、デフォルト値を持つ `DrumMoveType moveType = DrumMoveType::Jump` を追加する。
    *   `PlayCurrentTrack` 内部で `m_renderer.SetTrackInfo` を呼び出す際、受け取った `moveType` をそのまま Renderer へ伝播させる。
*   **要件2: 各アクションからの意図伝達（神経接続）**
    *   メディアキー操作やUI操作から `PlayCurrentTrack` を呼び出す箇所を特定し、操作の意図に合わせた `DrumMoveType` を引数として渡すように修正する。
        *   曲送り（NextTrack）や自動ループ再生時: `DrumMoveType::Next`
        *   曲戻し（PrevTrack）時: `DrumMoveType::Prev`
        *   リスト一覧からの切り替え時（SwitchPlaylist）: `DrumMoveType::CrossPlaylist`
        *   リスト内での任意の曲クリック（JumpToIndex）等: `DrumMoveType::Jump`
*   **要件3: 初期実装形式（UIクリア等）の純化**
    *   `src/Application_Playlist.cpp` の `ClearPlaylist()` や、初期化時の空打ちなど、直接 `m_renderer.SetTrackInfo(L"NO TRACK", L"---", ...);` と文字列を渡している箇所を特定する。
    *   これらはUIリセットを意味するため、必ず `DrumMoveType::Reset` を第4引数（`moveType`）として明示的に渡すように修正する。
*   **要件4: デフォルト引数のパージ（安全装置の解除）**
    *   Application層のすべての呼び出し元への適用が完了したら、タスク1で `src/Renderer.h` の `SetTrackInfo` 宣言に一時的に付与していたデフォルト引数（`= DrumMoveType::Jump` 等）を削除し、引数の受け渡しを強制（純化）する。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳密な限定**: 本タスクはApplication層からの呼び出し元の修正と、Renderer.hのシグネチャ純化のみ。`WidgetContext` へのデータ追加（タスク4）や、`Widget_TrackInfo` での描画切り替え（タスク5）には絶対に触れないこと。

-----------------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 21-9 Task 4 & 5 : 描画層の純化とOLD/NOWスナップショットの適用
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_1905_RES_Phase21-9_TrackDrumRebuild.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは完全独立型ドラムエンジン構築の最終ステップである「描画層の純化」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_1905_RES_Phase21-9_TrackDrumRebuild.md）の「タスク4」「タスク5」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「描画層の純化(Task 4 & 5)が完了しました。完全独立型ドラムエンジンの完成です。動作確認をお願いします！」と報告すること。

#### 【実装要件】
現在発生しているインデックス不整合による描画バグを根本解決するため、Widget_TrackInfo の描画がプレイリストデータを一切見ず、Rendererから渡された OLD / NOW のスナップショットのみを参照するように純化する。

*   **要件1: WidgetContext へのスナップショット追加 (タスク4)**
    *   `src/WidgetContext.h` に `DrumSlotData` の定義を `Renderer.h` から移動させる（WidgetContext が Renderer をインクルードしないよう依存関係を整理すること）。
    *   `WidgetContext` 構造体に `DrumSlotData oldDrumSlot;` と `DrumSlotData nowDrumSlot;` をメンバとして追加する。
    *   ※ `currentTrackIndex` 等の既存変数はドラムの目標位置計算に必要なのでそのまま残す。
*   **要件2: Renderer からのデータ受け渡し**
    *   `src/Renderer_Context.cpp` の `BuildRenderContext` （または `BuildAnimationContext`）にて、Renderer が保持している `m_oldDrumSlot` と `m_nowDrumSlot` を、そのまま `ctx.oldDrumSlot` と `ctx.nowDrumSlot` にコピーして Widget 層へ渡すように修正する。
*   **要件3: Widget_TrackInfo の描画純化 (タスク5)**
    *   `src/Widget_TrackInfo.cpp` の `Draw` メソッド内にある仮想スロット描画ループ（`for (int i = minSlot; i <= maxSlot; ++i)`）を以下のように改修する。
    *   ループ内でプレイリストの配列（`ctx.shuffleMetadataList` など）へインデックスアクセスして曲情報や画像を取得している処理を **すべて削除** する。
    *   代わりに、描画対象のスロット `slotIndex (i)` が目標位置の `ctx.currentTrackIndex` と一致するかどうかで純粋に分岐させる。
        *   `i == static_cast<int>(ctx.currentTrackIndex)` の場合: `ctx.nowDrumSlot` の画像、タイトル、アーティスト、トラックNoを描画する。
        *   それ以外（`i != static_cast<int>(ctx.currentTrackIndex)`）の場合: すべて **無条件で** `ctx.oldDrumSlot` の画像、タイトル、アーティスト、トラックNoを描画する。

#### 【絶対遵守ルール (Constraints)】
*   **プレイリスト参照の完全禁止**: `Widget_TrackInfo.cpp` の仮想スロット描画ループ内において、`ctx.shuffleMetadataList[i]` のようにインデックスでプレイリスト情報を取得する処理は絶対に許容しない。必ずスナップショットデータ（`oldDrumSlot` / `nowDrumSlot`）を使用すること。
