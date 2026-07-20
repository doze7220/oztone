##### 作業指示書 REQ: Phase 23-1: TagManagerの純化と既存背景アート機構の完全解体 (調査・計画立案)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料: D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】を満たすための全コード調査を行い、パージ対象となるコード、および後続フェーズで「背景マネージャ」へ移譲すべきコードの接点をすべてリストアップすること。
3. リストアップした情報に基づき、解体と配線図作成を行うための細かなタスクリストを構築すること。
4. 構築した計画書を、既存実装Hotfix追記対応時のフォーマットを参考に、実装計画書（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-1_BackgroundArt_PurgePlan.md）として新規作成（出力）すること。
5. チャットにて「解体計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
今後の「背景マネージャ」新設および「背景クロスフェード」実装に向けた事前準備として、現在の `TagManager` による重いフル解像度アルバムアート（APIC）の抽出機能をシステムから完全にパージ（削除）する。
中途半端な互換性維持やサムネイル画像での代用は一切行わない。本フェーズ中は一時的にビルドが破損することを許容し、徹底的な洗い出しを行う。

*   **要件1: TagManagerの純化（画像抽出のパージ）**
    *   `TagManager` クラスから、ID3v2タグ等を用いた画像バイナリ（APIC）の抽出ロジック、および画像データを保持・返却するメソッドや変数をすべて特定し、パージ対象とする。テキスト情報（曲名・アーティスト名）の抽出のみを行う純粋なクラスへと純化する。
*   **要件2: Application層のロード処理のパージ**
    *   曲決定時（`PlayCurrentTrack` 等）に即時実行されている、背景アート用の画像デコードや `Renderer` へのセット処理（`SetBackgroundArt` 等）を特定し、パージ対象とする。
*   **要件3: Renderer層の背景アート機構の完全パージ**
    *   `Renderer` および描画層において、背景アートを保持する変数（`m_backgroundArtBitmap` 等）、および `DrawBackground` メソッド内での画像描画処理を特定し、パージ対象とする。（※ダークオーバーレイ描画は残す）
*   **要件4: 関連するフレーミング操作・背景関連処理の完全洗い出し（移譲・結線の準備）**
    *   画像の拡大縮小・オフセットに関わる `ArtFramingDatabase` への連携部分、レイアウト計算（`LayoutCalculator`）、およびマウス操作（`Window_Mouse.cpp` などでの右ドラッグ操作等）、**「画像は読み込まなくなるが、現在背景画像を扱っているコード（接点）」をすべて洗い出してリストアップ**すること。
    *   後続のフェーズで新設する「背景マネージャ」へ結線・移譲するための**カタログ（配線図）**を作ることが目的であるため、無理に無効化してビルドを通そうとする必要はない。
*   **要件5: 潜在的な背景読み込みロジックの網羅的チェック**
    *   要件1〜3以外にも、背景画像の読み込みを密かに仕込んでいる箇所がないかを全コード検索して洗い出すこと。具体的には `LoadBitmapFromMemory` やWICに関連するデコード・ストリーム読み込み処理をキーワードとして調査し、背景画像に関するロード処理の残骸を1つ残らず特定すること。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **ビルド破損の許容と網羅性の優先**: 本解体外科手術中は、背景依存のコードが切断されることによる一時的なビルド破損を許容する。それよりも「どこに背景に関連する神経（コード）が通っているか」を1つ残らずリストアップすることを最優先とすること。
*   **徹底的なパージ**: サムネイル画像での代用など、一時的な誤魔化しのロジックは絶対に組み込まないこと。「背景画像は一切描画しない」状態へ完全に削ぎ落とすこと。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-1 タスク1: TagManagerの純化 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md

###### 【作業手順（厳守事項）】
本プロンプトは(Phase 23-1 タスク1: TagManagerの純化)のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md））のタスク1の項目に、、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします（※本タスク時点ではビルドエラーが発生する可能性があります）」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/TagManager.h`, `src/TagManager.cpp`
*   `Load` メソッドの `skipImage` 引数を削除し、常にテキスト情報（曲名・アーティスト名）のみを抽出するように純化する。
*   `GetAlbumArtBytes()` メソッドおよび戻り値となる `m_albumArtBytes` メンバ変数を完全に削除する。
*   `ExtractAlbumArtBinary()` メソッドを完全に削除する。
*   TagLib を用いた APIC (Picture) フレームの探索・抽出ロジック（`Load` および `ExtractAlbumArtBinary` 内）を全て削除する。

#### 【絶対遵守ルール (Constraints)】
*   **コンテキストの厳守**: 本タスクでは `TagManager` の純化のみを行うこと。他ファイル（`Application` や `Renderer` など）の修正は後続のタスクで行うため、この段階では絶対に触れないこと。他ファイルとの連携が切れることによる一時的なビルド破損は許容する。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-1 タスク2: Application層のロード処理パージ (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md

###### 【作業手順（厳守事項）】
本プロンプトは(Phase 23-1 タスク2: Application層のロード処理パージ)のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク2のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）のタスク2の項目に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします（※本タスク時点でも他層の依存が残っているためビルドエラーが発生する可能性があります）」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/Application_Playback.cpp`, `src/Application_Initialize.cpp`
*   `Application_Playback.cpp` の `PlayCurrentTrack` 内において、タスク1で削除された `TagManager::GetAlbumArtBytes()` を呼び出そうとしている箇所、および得られたバイナリを用いて `m_renderer.LoadBitmapFromMemory` で画像をデコードする一連の処理を完全に削除すること[1]。
*   同メソッド内での `m_renderer.SetBackgroundArt(artBitmap.Get());` の呼び出しを完全に削除すること[1]。
*   同メソッド内での `m_framingDb.GetFraming` および `m_renderer.SetBackgroundFraming` の呼び出しについて、Renderer側の口が後続タスクで塞がれるため、関連する呼び出しを削除またはコメントアウト（無効化）すること[1]。
*   `Application_Initialize.cpp` 内の `SetupCallbacks` において、`m_window.SetArtFramingMoveCallback`、`SetArtFramingScrollCallback`、`SetArtFramingResetCallback` 内で呼ばれている `m_renderer.SetBackgroundFraming` への伝達処理（および `ClampArtFraming`）を削除またはコメントアウト（無効化）すること[1]。

#### 【絶対遵守ルール (Constraints)】
*   **コンテキストの厳守**: 本タスクでは `Application` 層のロードおよび伝達処理のパージのみを行うこと。他ファイル（`Renderer` や `ThumbCacher` など）の修正は後続のタスクで行うため、この段階では絶対に触れないこと[2, 3]。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-1 タスク3: Renderer層の背景アート機構の完全パージ (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md

###### 【作業手順（厳守事項）】
本プロンプトは(Phase 23-1 タスク3: Renderer層の背景アート機構の完全パージ)のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク3のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク4以降をフライングで実行しないこと。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）のタスク3の項目に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/Renderer.h`, `src/Renderer.cpp`, `src/Renderer_Draw.cpp`
*   `Renderer.h` および `Renderer.cpp` から `SetBackgroundArt`, `SetBackgroundFraming`, `ClampArtFraming` メソッドの定義と実装を完全に削除する。
*   `Renderer.h` から `m_backgroundArtBitmap` （背景画像保持用）, `m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale` メンバ変数を完全に削除する。
*   `Renderer_Draw.cpp` の `DrawBackground` メソッド内において、`ID2D1Bitmap` を用いた画像描画ロジックとそれに伴うレイアウト取得処理を削除・無効化する。
    *   ※指示通り、`m_config->GetBgDarkenOpacity()` と `m_bgDarkenBrush` を用いた**ダークオーバーレイ（FillRectangle）の描画処理のみはそのまま残す**こと。

#### 【絶対遵守ルール (Constraints)】
*   **未来への結線マーカーの徹底**: 後続のフェーズで新設する「背景マネージャ」へ容易に再結線できるようにするため、メソッドを削除した箇所や `DrawBackground` 内の画像描画処理を削除・無効化した箇所には、必ず `// [Phase23-1] 背景アートパージに伴い削除。後日BackgroundManagerを結線すること` といった明確なマーカー付きコメントを残すこと。完全に跡形もなく消し去るのではなく、コードの「接点（配線跡）」を明示すること。
*   **コンテキストの厳守**: 本タスクでは `Renderer` 層のパージのみを行うこと。他ファイルの修正は後続のタスクで行うため、絶対に触れないこと。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-1 タスク4: 潜在的な背景読み込みロジックのパージ (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md

###### 【作業手順（厳守事項）】
本プロンプトは(Phase 23-1 タスク4: 潜在的な背景読み込みロジックのパージ)のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク4のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク5をフライングで実行しないこと。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）のタスク4の項目に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/Renderer.h`, `src/Renderer_Image.cpp`
*   `Renderer.h` から `LoadBitmapFromMemory` メソッドの定義を完全に削除すること。
*   `Renderer_Image.cpp` から `LoadBitmapFromMemory` の実装（WICを用いたメモリストリームからのデコード処理）を完全に削除すること。

#### 【絶対遵守ルール (Constraints)】
*   **コンテキストの厳守**: 本タスクでは潜在的な画像デコードロジックのパージのみを行うこと。他ファイル（`ThumbCacher` など）の修正は後続のタスクで行うため、この段階では絶対に触れないこと。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-1 タスク5: ThumbCacher内のサムネイル生成時画像抽出処理のパージ (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md

###### 【作業手順（厳守事項）】
本プロンプトは(Phase 23-1 タスク5: ThumbCacher内のサムネイル生成時画像抽出処理のパージ)のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク5のみ」** の実装を開始し、ソースコードの修正を実行すること。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1246_RES_Phase23-1_BackgroundArt_PurgePlan.md）のタスク5の項目に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **対象ファイル**: `src/ThumbCacher.cpp`
*   `ThumbCacher.cpp` の非同期処理ループ（`WorkerLoop` 等）内において、サムネイルを生成するために `TagManager::ExtractAlbumArtBinary(filepath)` を呼び出して生バイナリを取得しようとしている箇所を完全に削除（パージ）すること。
*   このパージによりサムネイル生成機能が一時的に機能しなくなるが、制約事項に従い、妥協的な代替処理（ダミーの画像ロードなど）は一切組み込まず、純粋に呼び出し処理とそれに付随するバイナリ取得処理を削ぎ落とすこと。

#### 【絶対遵守ルール (Constraints)】
*   **コンテキストの厳守**: 本タスクでは `ThumbCacher` 内の画像抽出呼び出しのパージのみを行うこと。他ファイルには絶対に触れないこと。
*   **代替処理の禁止**: サムネイルが生成できなくなることに対する一時的な誤魔化しのロジックは組み込まないこと。後続のフェーズでFileManagerと連携する新しいパイプラインを構築するための「完全な更地」を作ることが目的である。
