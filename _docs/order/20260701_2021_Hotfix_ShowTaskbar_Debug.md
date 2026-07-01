【目的】
タスクバー表示設定のHotfix：ConfigManagerの `ShowTaskbar` フラグがウィンドウスタイルに反映されず、タスクバーにアプリアイコンが表示されなくなってしまった先祖返りバグを修正します。

【実装要件】
1. ウィンドウスタイル構築ロジックの修正 (Window.cpp)
対象: src/Window.cpp の `Window::Initialize` メソッド
- 拡張ウィンドウスタイル (`dwExStyle`) を構築・指定している箇所を探してください。
- 現在ハードコードされている設定に加えて、ConfigManager の `GetShowTaskbar()` の値を評価する分岐を復活させてください。
  - `m_config->GetShowTaskbar()` が `true` の場合：`dwExStyle` に `WS_EX_APPWINDOW` を追加（タスクバーに強制表示）
  - `m_config->GetShowTaskbar()` が `false` の場合：`dwExStyle` に `WS_EX_TOOLWINDOW` を追加（タスクバーから隠す）
- ※現在設定されている `WS_EX_LAYERED` などの他の拡張スタイルは絶対に消さず、ビット論理和（`|`）で正しく合成されるようにしてください。

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点と懸念点をまとめた実装レポートを出力してください。
こ