##### 作業指示書 REQ: Hotfix / ジョグダイヤル確定ディレイのINI化 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは、ジョグダイヤリング（仮想スクロール）操作終了から楽曲再生が確定するまでの猶予時間を、INIファイルから調整可能にするHotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_JogDialConfirmDelay.md）として新規作成すること。
4. チャットにて「確定ディレイのINI化が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
ジョグダイヤルの操作を停止してから、実際に対象曲の再生が確定するまでの猶予時間（タイマー）がハードコードされているため、これを ConfigManager 経由で INI ファイルから動的に調整できるようにする。

*   **要件1: ConfigManager への設定追加**
    *   `src/ConfigManager.h` および `src/ConfigManager_Playback.cpp` (または該当する分割ファイル) にて、`JogDialConfirmDelay` (float型、秒単位) を追加する。
    *   デフォルト値は既存のハードコード値より少し長め（例: `0.8f` または `1.0f`）に設定し、`[Layout_TrackInfo]` などの適切なセクションから読み書きできるようにゲッターおよびセッターを実装する。
    *   `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` にも該当の初期値設定を追記する。
*   **要件2: Application層でのタイマー設定の置き換え**
    *   `src/Application_Initialize.cpp` (またはホイール入力のコールバック登録箇所) にて、ジョグダイヤル操作時に `m_virtualScrollTimer` にセットしている固定値を、`m_config->GetJogDialConfirmDelay()` から取得した値に置き換える。
*   **要件3: 連続操作時のタイマーリセット維持**
    *   ダイヤルを連続で回している間は、常に `m_virtualScrollTimer` が `JogDialConfirmDelay` の値で上書き（リセット）され続け、操作を完全に止めた時点から指定秒数のカウントダウンが開始される既存のロジックが維持されていることを確認する。
