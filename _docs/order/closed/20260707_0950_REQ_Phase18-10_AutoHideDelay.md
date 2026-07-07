### 作業指示書 REQ: Phase 18-10: 隠れるUIの共通離脱ディレイUX
以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-10_AutoHideDelay.md`）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
「必要な時だけ現れる忍者UI」の操作性を向上・統一するため、ロゴ拡張メニュー、プレイリスト、下部コントロール（シークバー等）の「隠れるUI」すべてに対し、マウスホバーが外れても指定時間展開を維持する「離脱ディレイ（猶予時間）」の仕組みを実装する。

* **要件1: ConfigManager の拡張**
  * 各UIのセクションに離脱ディレイ設定（float型, デフォルト: 3.0f [秒]）を追加し、読み書きとゲッターを実装する。
    * `[Layout_LogoMenu]` セクション: `MenuLeaveDelay`
    * `[Layout_Playlist]` セクション: `PlaylistLeaveDelay`
    * `[Layout_Window]` (またはコントロール共通) セクション: `ControlLeaveDelay`

* **要件2: UpdateAnimation における状態管理への遅延タイマー導入**
  * `Renderer::UpdateAnimation` （または各Widgetの更新処理）に、各UI用のタイマー変数を追加する。
    * 例: `m_logoMenuLeaveTimer`, `m_playlistLeaveTimer`, `m_controlLeaveTimer` (各float, 初期値0.0f)
  * 各UIの更新ロジックを以下のように共通化して改修する：
    1. **現在ホバー中** の場合：該当するタイマーを ConfigManager から取得したディレイ値にリセットし、UIを展開状態へ進める（アルファ値加算やスライドイン）。
    2. **ホバーから外れた** 場合：直ちに格納処理を始めるのではなく、タイマーを `deltaTime` で減算する。
    3. **タイマーが 0.0f 以下** になった場合に初めて、UIを格納する（アルファ値減算やスライドアウト）処理へ移行する。
  * ※プレイリストが「ピン留め (Pinned)」状態の場合は、タイマーに関わらず常に展開状態を維持する既存ロジックと衝突しないよう考慮すること。

#### 【作業終了後】
1. 作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-10_AutoHideDelay.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **アーキテクチャの厳守**: タイマーの管理と減算処理は描画ループ（Draw）内ではなく、必ず `UpdateAnimation` 内で `deltaTime` を用いて行うこと。


### 作業指示書 REQ: Phase 18-10 Hotfix: 展開維持状態とホバー判定領域の同期 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（`D:\ozlab\oztone\_docs\logs\20260707_0951_RES_Phase18-10_AutoHideDelay.md`）の末尾に「HOTFIX 1」の項目を追加し、原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【不具合の原因】
Phase 18-10 で `UpdateAnimation` にディレイを導入したことで、マウスが外れた瞬間に `Window` 側のホバーフラグ（`m_isPlaylistHovered` 等）は即座に `false` となるが、UI自体はディレイで展開状態を維持するようになった。
しかし、`Window::IsInPlaylistRegion` や `Window::IsInLogoMenuRegion` などのホバー当たり判定領域の拡張が、純粋な `m_isPlaylistHovered` 等にのみ依存したままであったため、**UIが開いているのに当たり判定だけが狭い領域（非ホバー状態の領域）に縮小してしまい、再ホバーを受け付けなくなる**という「表示状態と当たり判定の不一致」が発生していた。

#### 【実装要件】
描画層（Widget）が「実質的に展開状態である（ディレイ中または格納アニメーション中）」ことを、`WidgetContext` を経由して `Application` へフィードバックし、それを `Window` のホバー判定領域維持に適用する。

* **要件1: WidgetContext の拡張**
  * `WidgetContext.h` に、出力用フラグへのポインタ `bool* outIsPlaylistExpanded = nullptr;` および `bool* outIsLogoMenuExpanded = nullptr;` を追加する。

* **要件2: Window クラスの拡張と当たり判定の修正**
  * `Window.h / .cpp` にメンバ変数 `bool m_isPlaylistExpanded = false;` および `bool m_isLogoMenuExpanded = false;` を追加し、それぞれのセッターメソッド (`SetPlaylistExpanded(bool)`, `SetLogoMenuExpanded(bool)`) を実装する。
  * `Window::IsInPlaylistRegion` の中での幅拡張の条件（`PlaylistWidth` を使うか否か）を、`m_isPlaylistHovered` だけでなく `m_isPlaylistHovered || m_isPlaylistExpanded` に変更する。
  * `Window::IsInLogoMenuRegion`（または該当のヒットテスト・排他制御部分）も同様に、展開状態の判定に `m_isLogoMenuHovered || m_isLogoMenuExpanded` を加味し、メニューが実質的に展開中であれば領域を広く保ち、干渉排除が機能するように修正する。

* **要件3: Application の描画ループでの連携**
  * `Application::ForceRender` (または更新処理部) にてローカル変数 `bool isPlaylistExpanded = false;` と `bool isLogoMenuExpanded = false;` を定義し、`WidgetContext` 構築時に各 `outIs...` ポインタへアドレスをセットする。
  * `m_renderer.UpdateAnimation` 呼び出し後、取得した値を `m_window.SetPlaylistExpanded()` および `m_window.SetLogoMenuExpanded()` を用いて `Window` へフィードバックする。

* **要件4: Widget の UpdateAnimation での状態出力**
  * `PlaylistWidget::UpdateAnimation` にて、`if (ctx.outIsPlaylistExpanded)` が有効な場合、自身の状態（例: 離脱タイマーが `> 0.0f` であるか、またはアニメーション座標が完全に格納されていない状態か）を評価し、値を代入する。
  * `LogoMenuWidget::UpdateAnimation` にて、同様に自身の状態（離脱タイマーが `> 0.0f`、または `m_menuProgress > 0.0f` かどうか）を評価し、値を代入する。
