現在のプロジェクト（Phase 1 Step 1 完了状態）をベースに、次のステップを実装します。
【今回の目的：Phase 1 - Step 2】 設定ファイル管理クラス（ConfigManager）を導入し、ウィンドウのスタイル（枠の有無やタスクバー表示）を動的に切り替えられるようにします。
【実装要件】
src/ConfigManager.h, src/ConfigManager.cpp を新規作成してください。
アプリケーション起動時に、実行ファイルと同階層の OZtone.ini を読み込む処理を実装してください。
外部ライブラリは使用せず、Win32 API標準の GetPrivateProfileInt / WritePrivateProfileString 等を使用してください。
ファイルが存在しない場合（初回起動時）は、自動的にデフォルト設定値を持つ OZtone.ini を新規作成して出力してください。
OZtone.ini には [Window] セクションを設け、以下の3項目を管理（デフォルト値はすべて 0）してください。
ShowTitleBar (0:タイトルバーなし, 1:あり)
ShowWindowFrame (0:境界線・リサイズ枠なし, 1:あり)
ShowTaskbar (0:タスクバー非表示, 1:表示)
Application クラスや Window クラスを修正し、ConfigManager から読み込んだ設定値に応じてウィンドウ生成時のフラグ（WS_CAPTION, WS_THICKFRAME, WS_EX_TOOLWINDOW など）を適切に切り替えるようにしてください。
枠なし・タスクバー非表示（設定値がすべて0）の場合は、WS_POPUP をベースにするなど、美しい枠なしウィンドウになるよう設定してください。
プロジェクト憲法の「Manager巨大化禁止」「責務分離」に従い、既存のクラスを破壊しないよう注意してください。
【事後処理】 作業が完了したら、PROJECT_ARCHITECTURE.md の「5. 実装済みクラス・関数リファレンス」に ConfigManager の情報を追記し、_docs/logs/YYYYMMDD_HHMM_Phase1_Step2.md に実装レポートを出力してください。
