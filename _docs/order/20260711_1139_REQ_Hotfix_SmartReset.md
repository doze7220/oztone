##### 作業指示書 REQ: Hotfix 設定初期化のホットリロード化と初期値同期 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポートを新規作成（例：`YYYYMMDD_HHMM_RES_Hotfix_SmartReset.md`）し、対応内容を記録すること。
3. **【重要】** 実装完了後のチャット報告にて、「`DEFAULT_INI_CONTENT` には存在するがC++側の変数として存在しないもの」、あるいは「C++側の設定変数として存在するが `DEFAULT_INI_CONTENT` に記述されていないもの」のリストを必ずユーザーへ報告すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
ユーザーの指示による3段階のタスク順序に従い、設定初期化機構を再起動方式からホットリロード方式へ完全移行する。

*   **タスク1: 自己再起動処理の完全な削除**
    *   `src/Window.cpp` の `WM_COMMAND` 内 `ID_TRAY_RESET_ALL` の処理から、`ShellExecuteW` および `WM_CLOSE` 等のプロセス再起動に関わる記述を完全に削除する。
    *   代わりに、コールバック `m_onResetAllCallback` を呼び出すように修正する（`Window.h` への宣言とセッターも追加すること）。

*   **タスク2: `ConfigManager` の初期値同期と `ResetToDefaults()` の実装**
    *   `src/ConfigManager.h` および `src/ConfigManager.cpp` を精査し、コンストラクタ等で設定されるメンバ変数の初期値を、最新の `DEFAULT_INI_CONTENT` に書かれている値と完全に一致させる（移植する）。
    *   メモリ上のすべての設定用メンバ変数を、上記で同期した「初期値」に強制的にリセットするメソッド `void ResetToDefaults();` を新設する。
    *   ※フェーズ18-3で実装済みのINIファイルバックアップ機能はそのまま活かすこと。初期化中のINI上書きを防止するガードフラグ（`m_isResetting`等）が必要であれば適宜導入すること。

*   **タスク3: スマート初期化（ホットロード）のパイプライン構築**
    *   `src/Application.h` / `.cpp` に `void ResetAllSettings();` を新設し、以下の順序で処理を実装する。
        1. `m_config->SaveDefaultSettings();` （既存の機能により、バックアップファイルが作成され、新しいデフォルトINIが生成される）
        2. `m_config->ResetToDefaults();` （メモリ上の環境設定値を真の初期値にリセットする）
        3. `m_config->LoadSettings();` （生成された新しいINIから値を再ロードする）
        4. `m_renderer.ReloadResources();` （UIの動的再構築：ホットロードの実行）
    *   最後に、ウィンドウのサイズを 1024x512 に戻し、画面中央へ移動させる処理（`ID_TRAY_RESET_POS` で行っている処理と同等のもの）を実行する。
    *   `Application::Initialize` にて、`m_window.SetResetAllCallback` にこの `ResetAllSettings` を登録する。
