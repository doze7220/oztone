##### 作業指示書 REQ: Task 10.5 (Hotfix) 背景設定の結線修復とキー名リファクタリング (実装実行)
* ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* 開発資料: D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
* 実装計画書: D:\ozlab\oztone\_docs\logs\20260723_2128_RES_Phase24-1_ConfigManagerPlan.md

###### 【作業手順（厳守事項）】
本プロンプトは背景設定の結線修復とINIキー名衝突回避のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260723_2128_RES_Phase24-1_ConfigManagerPlan.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
4. コード修正が完全に終わった後、既存の作業レポート（20260723_2128_RES_Phase24-1_ConfigManagerPlan.md）の末尾に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
6. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
* バグの原因である、自動置換スクリプトによる `[Background]` と `[Layout_SeekBar]` の `BgOpacity` 誤爆結線を修正すること。
* `src/BackgroundManager.cpp` や `src/Renderer_Draw.cpp` 等を調査し、背景描画に関わる透過度や設定が誤って `GetLayoutSeekBar()` から取得されている箇所を、正しい `GetBackground()` に結線し直すこと。
* **【追加リファクタリング】** 今後の名前衝突事故を防ぐため、INIファイルのキー名および構造体のメンバ変数を以下のように一意な名前に変更し、パース処理と `DEFAULT_INI_CONTENT` にも反映させること。
  * `[Background]` セクション
    * `BgOpacity` -> `BackgroundOpacity`
    * `BgDarkenOpacity` -> `BackgroundDarkenOpacity`
  * `[Layout_SeekBar]` セクション
    * `BgOpacity` -> `SeekBarBgOpacity`
    * `BgColor` -> `SeekBarBgColor`
    * `FgOpacity` -> `SeekBarFgOpacity`
    * `FgColor` -> `SeekBarFgColor`
* 各 Widget 等での呼び出し元も新しい変数名に合わせて修正し、ビルドを通して背景透過やシークバーの描画が正常に行われることを確認すること。

#### 【絶対遵守ルール (Constraints)】
* **スコープの厳守**: 上記のバグ修正とキー名のリネーム処理に留め、他の無関係な処理やファイルには触れないこと。
