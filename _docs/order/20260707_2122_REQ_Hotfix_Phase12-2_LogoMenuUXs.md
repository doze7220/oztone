### 作業指示書 REQ: Phase 18-11 事前準備: ロゴ拡張メニューの背景画像削除 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\closed\20260704_2114_RES_Phase12-2_LogoMenuUX.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（`D:\ozlab\oztone\_docs\RES(Hotfix)_template.md`）を元に、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHSS_RES_Hotfix_Phase12-2_LogoMenuUX.md`）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

#### 【実装要件】
ホバー時の動的なカラーフェードが完成したため、旧来のアフォーダンスであった「アイコンの背面に表示される背景画像」を削除し、UIを洗練させる。

*   **修正箇所1: ConfigManagerの整理**
    *   `src/ConfigManager.h` および `src/ConfigManager.cpp` から、ロゴ拡張メニューの背景画像不透明度に関する設定 `IconHoverBgAlpha` のメンバ変数、ゲッターメソッドを削除する。
    *   INIファイルのロード処理（`LoadSettings`）およびデフォルトテンプレート文字列（`DEFAULT_INI_CONTENT`）の `[Layout_LogoMenu]` セクションからも `IconHoverBgAlpha` を完全に削除する。
*   **修正箇所2: LogoMenuWidgetからの描画ロジック削除**
    *   `src/LogoMenuWidget.cpp` の `LogoMenuWidget::CreateResources` における `app_logo_back.png` (IDI_APP_LOGO_BACK) のロード処理を削除する。
    *   これに伴い不要となるメンバ変数（背景用の `m_logoBackBitmap` など）を `src/LogoMenuWidget.h` から削除する（テキスト用のドロップシャドウ等はそのまま残すこと）。
    *   `src/LogoMenuWidget.cpp` の `LogoMenuWidget::Draw` にて行われている、ホバー中のアイコン背面に背景画像を描画する処理（およびそれに付随する影の描画）を削除する。
