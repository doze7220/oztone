##### 作業指示書 REQ: PlaylistTimeLetterSpacing が適用されないバグの修正 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは PlaylistTimeLetterSpacing 適用漏れバグの Hotfix である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_PlaylistTimeLetterSpacing.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Widget_Playlist_DrawItems.cpp`
*  プレイリストのトラック一覧描画ループ内において、各トラックの「再生時間（timeString）」を描画する処理を改修する。
*  現在の実装では `ConfigManager` に `PlaylistTimeLetterSpacing` という設定値が存在するにもかかわらず、描画時に適用されていないため、INIで文字間隔を変更しても反映されないバグが発生している。
*  これを解消するため、再生時間テキストを描画するための `IDWriteTextLayout` オブジェクトに対して `SetCharacterSpacing` メソッドを呼び出し、`ConfigManager` から取得した `PlaylistTimeLetterSpacing` の値を適用するロジックを追加すること。
*  （※ もし現状の描画が `DrawText` を用いて行われている場合は、`CreateTextLayout` を用いて `IDWriteTextLayout` を生成し、`SetCharacterSpacing` を適用してから `DrawTextLayout` で描画する形式へ変更すること）

#### 【絶対遵守ルール (Constraints)】
*   **レイアウトの維持**: 再生時間のテキスト描画は既存の右寄せ（Trailing）レイアウトなどを崩さず、純粋に文字間隔（Letter Spacing）のプロパティのみを付与すること。
