### 作業指示書 REQ: INI設定の大掃除と共通パラメータ(UI_Common_Parm)の統合 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリングの「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（`YYYYMMDD_HHMM_RES_Refactor_UICommonParm.md`）を新規作成し、対応内容を記録すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、共通テーマ設定に関する仕様を追記・修正すること。
4. チャットにて「INI設定の大掃除と共通化が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
各UIセクションに分散・重複している「影」「フォント」「テーマカラー」「ホバー/離脱ディレイ」のパラメータをパージし、新設する `[UI_Common_Parm]` セクションに一元管理するリファクタリングを行う。
また、将来のOSD拡張を見据え、OSD用フォントも共通パラメータとして統合する。

*   **要件1: `[UI_Common_Parm]` セクションの新設**
    *   `ConfigManager` にて以下の共通パラメータを定義し、デフォルト値の書き出しと読み込みを実装する。
        *   `ThemeColor=#FFA500`
        *   `HoverFadeOutSpeed=2.0`
        *   `BaseLeaveDelay=3.0`
        *   `BaseFontFamily=Meiryo`
        *   `MonoFontFamily="Courier New"`
        *   `IconFontFamily="Segoe UI Emoji"`
        *   `OsdFontFamily="MS Gothic"`
        *   `EnableShadow=1`
        *   `ShadowColor=#000000`
        *   `ShadowOffsetX=2.0`
        *   `ShadowOffsetY=2.0`
        *   `ShadowOpacity=0.7`

*   **要件2: 既存セクションからの重複パラメータ削除（パージ）**
    *   `[Layout_Window]`, `[Layout_VolumeControl]`, `[Layout_Playlist]`, `[Layout_OSD]`, `[Layout_LogoMenu]`, `[Layout_NowPlaying]`, `[Layout_GlobalHotkeys]`, `[Layout_PlaybackControls]` など各セクションに存在していた個別の影設定、フォント設定（`TitleFontFamily`, `MenuFontFamily`, `FontFamily` 等）、および `HoverIconColor`, `PlayingItemColor`, `HoverItemColor`, `ControlLeaveDelay` などの重複パラメータを `ConfigManager` のメンバ変数および `DEFAULT_INI_CONTENT` から完全に削除する。

*   **要件3: C++ 描画側への共通パラメータの適用**
    *   各ウィジェットのテキストフォーマット生成処理や、ブラシ生成、影エフェクト、およびホバー/離脱時の計算において、削除された個別パラメータの代わりに `m_config->Get...` を通じて `[UI_Common_Parm]` の共通値を参照するようにコードを修正する。
    *   アクティブ色・ホバー色はすべて `ThemeColor` を基準とし、OSDやチートシートのフォントは `OsdFontFamily` を参照するようにする。

*   **要件4: プレイリスト専用の例外（オーバーライド）対応**
    *   プレイリストは巨大なUIであるため、フェードアウトと離脱ディレイを独立させる。
    *   `[Layout_Playlist]` セクションに `PlaylistLeaveDelay=0.5` を新設（または維持）し、プレイリストの描画・更新処理だけはこれらを参照するように例外処理を組み込む。

#### 【絶対遵守ルール (Constraints)】
*   **AIハルシネーションの防止**: 本タスクは純粋な変数の集約とリファクタリングである。`Theme` という言葉から連想されるようなスキン機能の実装や、マテリアルデザイン等の新しい描画ロジックの追加は**絶対に**行ってはならない。
*   **描画の互換性**: 各UIのサイズや座標計算（オフセット）には一切変更を加えないこと。見た目は全く変わらないが、内部の参照先だけが共通化される状態を目指すこと。
