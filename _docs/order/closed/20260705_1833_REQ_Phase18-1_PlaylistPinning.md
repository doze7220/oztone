### 作業指示書 REQ: Phase 18-1: プレイリスト表示固定（ピン留め）機能 (計画立案)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-1_PlaylistPinning.md`）として新規作成すること。
3. レポートのフォーマットは、`PROJECT_CONSTITUTION.md` の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
プレイリストを常に展開した状態に固定（ピン留め）し、他のUI要素がプレイリストに被らないよう描画領域を動的に避ける（圧縮・移動する）機能を実装する。

* **要件1: 設定の拡張 (ConfigManager)**
  * ピン留め状態を保持する `IsPlaylistPinned` (bool) を追加し、INIファイルから読み書き可能にする。
  * `[Layout_Playlist]` セクションに、ピン留めのサブアイコン（南京錠）を描画するためのオフセットおよびフォントサイズ設定（例: `PinSubIconOffsetX`, `PinSubIconOffsetY`, `PinSubIconFontSize`）を追加する。
* **要件2: ピン留めトグルボタンの実装 (PlaylistWidget / Window / Application)**
  * プレイリスト上部のツールバーにピン留めボタンを追加する。
  * **描画仕様**:
    * ピン留めOFF時（自動格納）: ベースアイコン「📌」（不透明度0.4f程度）の右下に、白文字＋黒フチで「🔓」（半分のサイズ）を描画。ツールチップは「表示モード: 自動格納」。
    * ピン留めON時（画面固定）: ベースアイコン「📌」（不透明度1.0fの白）の右下に、白文字＋黒フチで「🔒」（半分のサイズ）を描画。ツールチップは「表示モード: 画面固定」。
  * **操作仕様**: ツールバーのクリックイベントを拡張し、このボタンがクリックされた際に `ConfigManager` の `IsPlaylistPinned` を反転させる。
* **要件3: LayoutCalculator の動的領域圧縮**
  * `LayoutCalculator` の各計算処理（`CalculateTrackInfoLayout`, `CalculateSeekBarLayout`, `CalculatePlaybackControlsLayout`, `CalculateVolumeControlLayout`, `CalculateVisualizerLayout` 等）において、以下のオフセット処理を追加する。
  * **条件**: `IsPlaylistPinned == true` かつ 「現在のウィンドウ幅 > 最小幅(495) + PlaylistWidth」である場合のみ、以下のオフセットを適用する（ウィンドウが狭い場合はオフセット処理を無効化する）。
  * **左配置時 (`PlaylistPosition == 0`)**: 各UIのX座標の基点を `PlaylistWidth` 分だけ右にずらし、有効キャンバス幅を `PlaylistWidth` 分狭めて計算する。
  * **右配置時 (`PlaylistPosition == 1`)**: 各UIのX座標の基点はそのままとし、有効キャンバス幅のみを `PlaylistWidth` 分狭めて計算する。
* **要件4: 排他制御の解除 (Window)**
  * 現在 `Window::WindowProc` 等において、「プレイリスト展開中 (`m_isPlaylistHovered`) は背面にあるシークバーやコントロールのホバー・クリック判定をスキップする」という排他制御が入っている。
  * `IsPlaylistPinned` が true の場合はUIが重ならないため、この排他制御を解除し、プレイリストが開いていても他のUIを操作できるように修正する。

#### 【作業終了後】
1. 作業レポート（`...RES_Phase18-1_PlaylistPinning.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。


### 作業指示書 REQ: Phase 18-1: プレイリストピン留めの不具合修正と調整 (Hotfix)

以下のプロジェクトルールと開発資料、前回の作業レポートを熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
* D:\ozlab\oztone\_docs\logs\20260705_1834_RES_Phase18-1_PlaylistPinning.md`

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（`D:\ozlab\oztone\_docs\logs\20260705_1834_RES_Phase18-1_PlaylistPinning.md`）の末尾に「HOTFIX」の項目を追加し、原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

#### 【実装要件】
現在発生しているピン留め機能に関連する不具合の修正と、UIのアフォーダンス調整を行う。

* **修正1: ロゴ関連UIの大移動漏れ (`src/LayoutCalculator.cpp`)**
  * `CalculateAppLogoLayout` および `CalculateLogoMenuLayout` においても、他のUIと同様にピン留めON（かつウィンドウ幅が十分な場合）のオフセット処理（左配置時に `PlaylistWidth` 分だけ右にX座標をシフトする処理）を追加する。
* **修正2: ウィンドウ外へのマウス退出時の展開維持 (`src/PlaylistWidget.cpp`)**
  * `PlaylistWidget::UpdateAnimation` 内のスライド計算のベースとなる展開判定条件を、現在の「プレイリストホバー中かどうか」だけでなく、「ピン留め状態であるか」を含めるように修正する。
  * 修正例: `bool isExpanded = ctx.isPlaylistHovered || config->GetIsPlaylistPinned();`
* **修正3: コンフィグの永続化 (`src/ConfigManager.cpp`)**
  * `ConfigManager::SetIsPlaylistPinned` 内でフラグを反転/設定した直後に、`WritePrivateProfileStringW` を用いて `[Layout_Playlist]` セクションの `IsPlaylistPinned` として INI ファイルへ即時保存するように修正する。また、コンストラクタおよび `LoadSettings` での読み込み処理も保証すること。
* **追加調整: ピン留め中のアフォーダンス調整 (`src/PlaylistWidget.cpp`)**
  * `PlaylistWidget::Draw` 内で行われている「プレイリスト引き出しグリップの矢印（◀ または ▶）」の描画処理について、ピン留めがON (`config->GetIsPlaylistPinned() == true`) の場合は矢印ジオメトリの描画をスキップするように条件分岐を追加する。（※縦のグリップ線については残しても消しても良いが、矢印は確実に消すこと）
