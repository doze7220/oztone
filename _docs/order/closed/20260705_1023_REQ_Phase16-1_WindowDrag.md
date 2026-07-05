### REQ: Phase 16-1: ウィンドウドラッグ移動の全体化と移動ロック (計画立案)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【実装要件】
本フェーズでは、現在左上のロゴアイコン領域に限定されているウィンドウのドラッグ移動処理を「UI要素がない背景部分全体」に拡張する。同時に、システムトレイメニューからウィンドウの移動を禁止する「画面固定（移動ロック）」機能を実装する。

* **要件1: 画面固定設定の追加 (ConfigManager / Window)**
  * `ConfigManager` に `LockWindowPosition` (bool, デフォルトfalse) を追加し、保存・読み込みを実装する。
  * `Window.cpp` のシステムトレイメニュー（`TRAY_MENU_ORDER` 等）に「画面固定 (Lock Window Position)」を追加し、トグル状態の表示と設定の切り替え処理を実装する。
* **要件2: ドラッグ移動判定の全体化 (Window)**
  * `Window::WindowProc` の `WM_LBUTTONDOWN` におけるドラッグ移動発動（`SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0)`）の条件を拡張する。
  * 既存の `IsInLogoRegion` だけでなく、「プレイリスト領域、ロゴ拡張メニュー領域、再生コントロール領域、音量コントロール領域などの **どのUI要素の判定にも引っかからない（ホバー状態ではない）完全な背景部分**」をクリックした場合にドラッグ移動が発動するようにロジックを修正する。
  * ただし、`ConfigManager` の `LockWindowPosition` が true の場合は、この背景部分およびロゴ部分でのドラッグ移動を一切発動させないようにする。

#### 【絶対遵守ルール (Constraints)】
* 既存のUI要素（シークバーやロゴメニュー等）のクリック判定を邪魔しないよう、ドラッグ判定は「他のすべてのUI判定をすり抜けた後のフォールバック」として適切に順序付けること。

#### 【作業終了後】
1. 本プロンプトの要件を満たす実装計画書を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase16-1_WindowDrag.md`）として新規作成する。
2. 実装の背景（目的）と、詳細作業内容（タスクリスト）を記載する。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。
