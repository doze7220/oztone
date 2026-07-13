### 作業指示書 REQ: Hotfix: 背景クリックによるUIフェードアウト機能の復元 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260711_1120_RES_Hotfix_ActiveFadeOut.md (過去の実装仕様)

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの調査および修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ActiveFadeOut_Restore.md）として新規作成し、原因（どのファイル分割時に漏れたか）と対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
Phase 20 の一連のファイル分割（Application, Window, Widgets）の過程で欠落してしまった、「UI要素がない背景部分をクリックした際に、UI維持ディレイを0にしてすぐ閉じ始める（能動的フェードアウト）」機能を、現在の分割されたファイル構成に合わせて完全に復元する。

各層において以下の実装が欠落していないか調査し、欠落している場合は復元（再実装）すること。

*   **要件1: Window層の復元 (`src/Window_Mouse.cpp`, `src/Window.h` 等)**
    *   `m_onBackgroundClickCallback` の宣言およびセッターが存在しているか確認・復元する。
    *   `WM_LBUTTONDOWN` （ドラッグ移動前）および `WM_RBUTTONDOWN` （フレーミング前）において、どのUI領域にもヒットしなかった「純粋な背景クリック」の際にコールバックを発火する処理を復元する。
*   **要件2: Application層の復元 (`src/Application_Initialize.cpp` 等)**
    *   コールバック登録部（`SetupCallbacks` や `Initialize` など）において、`m_window.SetBackgroundClickCallback` を登録し、内部で `m_renderer.ForceClearHoverDelays()` を呼び出す処理を復元する。
*   **要件3: Renderer層の復元 (`src/Renderer.h / .cpp`)**
    *   `ForceClearHoverDelays()` メソッドが存在し、内部で `m_controlLeaveTimer` などを `0.0f` にリセットしつつ、`m_widgets` に登録されたすべてのWidgetの `ForceClearHoverDelay()` を呼び出しているか確認・復元する。
*   **要件4: Widget層の復元 (`src/Widget_Playlist.cpp`, `src/Widget_LogoMenu.cpp`, `src/Widget.h` 等)**
    *   `IWidget` インターフェースに `virtual void ForceClearHoverDelay() {}` が存在するか確認・復元する。
    *   `PlaylistWidget` および `LogoMenuWidget` 等において、同メソッドがオーバーライドされており、内部のディレイ用タイマー変数が `0.0f` にリセットされる処理を復元する。

#### 【絶対遵守ルール (Constraints)】
*   **アーキテクチャの厳守**: 現在の「1ファイル = 1責務（分割後）」の構成を崩さず、適切なファイルに対してコードを復元すること。
