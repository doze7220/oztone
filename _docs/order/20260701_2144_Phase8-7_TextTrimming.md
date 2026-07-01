【目的】
Phase 8-6: 文字溢れ対応。現在の曲情報が画面幅いっぱいに使えるようDirectWriteのトリミング（...）を適用します。また、NextTrack機能を「INIファイルからのみ有効化できる隠し機能」としてデフォルトで非表示にします。重なりによるレイアウト破綻はユーザーの自己責任とします。

【実装要件】
1. 隠しフラグの追加 (ConfigManager.h, ConfigManager.cpp)
- `[Layout_NextTrack]` セクションに `EnableNextTrack` (bool) を追加し、ゲッターを実装してください。
- 初期化時および `DEFAULT_INI_CONTENT` のデフォルト値を `0` (false) に設定してください。

2. DirectWriteのトリミング実装 (Renderer.cpp)
- `IDWriteFactory::CreateEllipsisTrimmingSign` と `SetTrimming` を用いて、現在の曲名・アーティスト名・シークバー時間、およびNextTrack用のテキストフォーマットに「...」による省略設定を追加してください。

3. 描画ロジックの修正 (Renderer.cpp)
- `Render` メソッド内にて、NextTrackの描画処理一式（画像、曲名、アーティスト名、ラベル等）を `if (m_config->GetEnableNextTrack())` で囲み、フラグが有効な時のみ描画されるようにしてください。
- 現在の曲名およびアーティスト名を描画する `DrawTextLayout` 生成時の許容最大幅 (`maxWidth`) は、「現在のウィンドウ論理幅から右端の最低限のマージンのみを引いた値」に設定し、画面の右端まで贅沢にテキストを広げられるようにしてください。（※隠し機能であるNextTrackとの衝突回避計算は不要です）

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点と懸念点をまとめた実装レポートを出力してください。
