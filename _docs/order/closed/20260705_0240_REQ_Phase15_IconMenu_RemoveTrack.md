### REQ: ロゴ拡張メニューからの「曲削除」項目の廃止 (実装実行)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【現状の仕様と修正目的】
Phase 13にてプレイリストUIのツールバーに「再生曲の削除」機能が統合されたため、ロゴ拡張メニューに存在している同機能（🗑️）は役割が重複している。
ストイックなUIを保つため、ロゴ拡張メニューから「再生中の曲をプレイリストから削除する」項目を完全に削除する。

#### 【作業手順（厳守事項）】
事前の計画立案は省略し、即座に以下の手順でコードの修正を実行すること。
1. 以下の【実装要件】に従い、対象ファイルのコードを的確に修正すること。
2. 修正が完了したら、チャットにて「ロゴ拡張メニューからの曲削除項目の廃止が完了しました。動作確認をお願いします」と報告すること。

#### 【実装要件】
* **要件1: メニュー配列と定数の削除 (Window.h / Window.cpp)**
  * `src/Window.h` に定義されている曲削除用コマンドID（`ID_LOGO_REMOVE_TRACK` またはそれに類する定数）を削除する。
  * `src/Window.cpp` のコンストラクタ内にある `m_logoMenuItems` の初期化リストから、曲削除アイコン（`L"🗑️"`）の項目を削除し、配列を詰める。
  * `src/Window.cpp` の `WindowProc` 内、`WM_LBUTTONDOWN` メッセージ処理におけるロゴメニューのクリック判定から、該当コマンドIDがクリックされた際の処理ブロックを完全に削除する。
* **要件2: ホバーテキスト分岐の削除 (Widgets.cpp)**
  * `src/Widgets.cpp` の `LogoMenuWidget::Draw` メソッド内において、ホバー対象の `commandId` を判定して説明テキストを切り替えている箇所から、該当コマンドIDに対する分岐（例: `textToDraw = L"REMOVE TRACK";` 等）を完全に削除する。

#### 【作業終了後】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase15_IconMenu_RemoveTrack.md`）として新規作成する。
2. 原因と、詳細作業内容を記載する。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う。

#### 【対象ファイル】
* `src/Window.h`
* `src/Window.cpp`
* `src/Widgets.cpp`
