##### 作業指示書 REQ: 初回完全同期着地（1秒タイムアウト付き）機構の実装 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトは、起動直後の初期描画において、メタデータ・サムネ・背景の非同期ロード完了を待ってから再生と描画を開始する「初回完全同期着地機構」の実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_InitialSyncLoad.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Application.h`, `src/Application_Initialize.cpp`, `src/Application_Playback.cpp`, `src/Application_Render.cpp`
*   **タスク1: 状態フラグの追加 (Application.h)**
    *  `Application` クラスに、初期ロード待機状態を示す `bool m_isInitialLoading = false;` と、タイムアウト計測用の `ULONGLONG m_initialLoadStartTime = 0;` を追加する。
*   **タスク2: 初期起動時のフラグセット (Application_Initialize.cpp)**
    *  `Application::Initialize` の末尾付近、プレイリストをロードして最初の曲を再生する直前に `m_isInitialLoading = true;` および `m_initialLoadStartTime = GetTickCount64();` をセットする。
*   **タスク3: 音声再生の保留とフレーミングロック判定の遅延 (Application_Playback.cpp)**
    *  `Application::PlayCurrentTrack` において、`m_isInitialLoading` が `true` の場合は、メソッド終盤の `m_audioManager.Play()`（音声の再生開始）をスキップ（保留）するように分岐を追加する。
    *  また、フレーミング操作のロックフラグ (`m_isCurrentBackgroundPlaceholder`) の確定処理も、初期ロード中はスキップし、ロード完了時に判定するようにする。
*   **タスク4: メインループでの同期待機とフェイルセーフ (Application_Render.cpp)**
    *  `Application::Run`（またはメインループ内）の描画処理実行前に、`m_isInitialLoading` が `true` の場合の待機ステート処理を追加する。
    *  毎フレーム以下の条件が**すべて満たされたか**をチェックする。
        1. **メタデータ**: TrackDatabase に該当曲の `isMetaLoaded` が `true` になっているか。
        2. **サムネイル**: ThumbnailManager から該当曲のキャッシュ画像（`GetCachedThumbnailBitmap`）が取得できるか。
        3. **背景**: BackgroundManager のレイヤー情報などに該当パスの画像が準備されているか（またはプレースホルダフラグが確定可能か）。
    *   **同期完了またはタイムアウト**:
        *  上記のデータが「すべて揃った」場合、または `GetTickCount64() - m_initialLoadStartTime >= 1000`（1秒経過）した場合。
        *  `m_isInitialLoading = false;` に更新する。
        *  保留していた `m_audioManager.Play()` を呼び出して音楽をスタートさせる。
        *  ここで改めて `m_isCurrentBackgroundPlaceholder` の判定（ロックフラグの確定）を行い、`ForceRender()` 等で完全な状態の画面を一気に描画させる。

#### 【絶対遵守ルール (Constraints)】
*   **ノンブロッキングの厳守**: `Sleep(1000)` や `while` 等でメインスレッドを物理的に停止させることは絶対に禁止する。必ず `Run` のメッセージループを回しながら `GetTickCount64()` でフレームごとに経過時間をチェックするステートマシン方式で実装すること。
