# ConfigManagerの導入とウィンドウスタイルの動的変更

指定されたPhase 1 - Step 2の要件に基づき、`ConfigManager` クラスの新規作成と、それに伴う `Application` クラス、`Window` クラスの修正を行います。

## User Review Required

> [!IMPORTANT]
> **iniファイルの生成ロジックと実行ファイルのパス取得について**
> iniファイルは実行ファイルと同階層に生成するため、`GetModuleFileNameW`を使用して実行ファイルのフルパスを取得し、それをベースに`OZtone.ini`のパスを構築します。

> [!IMPORTANT]
> **タスクバー非表示の実装について**
> `ShowTaskbar=0` の場合、ウィンドウスタイルに `WS_EX_TOOLWINDOW` を設定します。これによりタスクバーからは消えますが、Alt+Tabのリストにも表示されなくなります。これはアクセサリとして一般的な挙動ですが、問題があればフィードバックをお願いします。

## Open Questions

特になし。要件は明確です。

## Proposed Changes

### Configuration Manager (New)

#### [NEW] [ConfigManager.h](file:///D:/ozlab/oztone/src/ConfigManager.h)
- シングルトンは避け、`Application`のメンバとしてインスタンス化します。
- `Initialize()` メソッドで実行ファイルパスから `OZtone.ini` のパスを解決し、読み込み（存在しなければ作成）を行います。
- `GetShowTitleBar()`, `GetShowWindowFrame()`, `GetShowTaskbar()` メソッドを提供します。

#### [NEW] [ConfigManager.cpp](file:///D:/ozlab/oztone/src/ConfigManager.cpp)
- Win32 API (`GetPrivateProfileIntW`, `WritePrivateProfileStringW`) を用いてiniの読み書きを実装します。

---

### Application & Window Subsystem

#### [MODIFY] [Application.h](file:///D:/ozlab/oztone/src/Application.h)
- `ConfigManager` をメンバ変数 `m_config` として追加し、`#include "ConfigManager.h"` を追加します。

#### [MODIFY] [Application.cpp](file:///D:/ozlab/oztone/src/Application.cpp)
- `Initialize` 内で、まず `m_config.Initialize()` を呼び出します。
- その後 `m_window.Initialize(hInstance, nCmdShow, m_config)` のように、`ConfigManager`への参照を渡してウィンドウを初期化します。

#### [MODIFY] [Window.h](file:///D:/ozlab/oztone/src/Window.h)
- `Initialize` の引数に `const ConfigManager& config` を追加します。

#### [MODIFY] [Window.cpp](file:///D:/ozlab/oztone/src/Window.cpp)
- `config` の設定値に従い、`dwStyle` および `dwExStyle` を動的に組み立てます。
- 枠なし・タスクバー非表示時は `WS_POPUP` と `WS_EX_TOOLWINDOW` を使用します。
- タイトルバー・枠なし時でもドラッグ移動等の処理は現状含まれていませんが、今回は「スタイルの動的切り替え」に留め、次フェーズでの対応を想定します（必要があれば調整します）。

---

### Documentation

#### [MODIFY] [PROJECT_ARCHITECTURE.md](file:///D:/ozlab/oztone/PROJECT_ARCHITECTURE.md)
- 「5. 実装済みクラス・関数リファレンス」に `ConfigManager` の情報を追記します。

## Verification Plan

### Automated Tests
- 本プロジェクトには現在自動テストの枠組みがないため、手動検証を行います。

### Manual Verification
1. アプリケーションをビルドして実行します。
2. 初回起動時、実行ファイルと同階層に `OZtone.ini` が作成されることを確認します。
3. デフォルト設定 (`ShowTitleBar=0`, `ShowWindowFrame=0`, `ShowTaskbar=0`) の状態で起動し、ウィンドウが枠なし・タイトルバーなし・タスクバー非表示になることを確認します。
4. いったん終了し、`OZtone.ini` の値を書き換え（例: `ShowTitleBar=1`, `ShowWindowFrame=1`, `ShowTaskbar=1`）、再度起動してタイトルバーや枠、タスクバーへの表示が有効になることを確認します。
5. 作業完了後、`_docs/logs/YYYYMMDD_HHMM_Phase1_Step2.md` を作成して実装内容を報告します。
