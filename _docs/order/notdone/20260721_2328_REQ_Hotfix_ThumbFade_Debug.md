##### 作業指示書 REQ: サムネイル遅延ロードのデバッグログ追加とポーリング間隔調整 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトはサムネイル遅延ロード処理の原因特定に向けた、デバッグログの追加と一時的なポーリング間隔の変更である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_ThumbFade_Debug.md）として新規作成すること。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/Application_Playback.cpp`, `src/Renderer_Update.cpp`
*   **タスク1: 曲再生時（初動）のデバッグログ追加 (Application_Playback.cpp)**
    *  `Application::PlayCurrentTrack` 内の `dataProvider` ラムダ式の内部（スロットへの値セット完了直後）に、ログ出力処理を追加する。
    *  出力方法: `wchar_t` バッファと `swprintf_s` を使い `OutputDebugStringW` で出力。
    *  フォーマット例: `L"[ThumbDebug] PlayCurrentTrack - thumbId: %u, artBitmap: %p\n"`
*   **タスク2: ポーリング間隔の一時変更 (Renderer_Update.cpp)**
    *  `Renderer_Update.cpp` の `UpdateAnimation` 内にあるサムネイル監視のポーリング間隔を、ログの氾濫を防ぐため一時的に `1000` (1秒) から `5000` (5秒) に変更する。
*   **タスク3: ポーリング時（継続監視）のデバッグログ追加 (Renderer_Update.cpp)**
    *  `UpdateAnimation` 内の5秒ポーリング処理のループ内に、毎スロットの状態を出力するログを追加する。
    *  出力方法: 同様に `OutputDebugStringW` を使用。
    *  フォーマット例: `L"[ThumbDebug] Polling - Slot: %d, thumbId: %u, artBitmap: %p\n"`
*   **タスク4: 削除用目印コメントの徹底**
    *  本プロンプトで追加・変更したすべての箇所（ログ出力処理の上下、および間隔を5000に変更した行）に、後日容易に検索して元に戻せるよう、必ず以下の目印コメントを記述すること。
    *  `// [DEBUG_REMOVE_LATER] デバッグ用。原因特定後に削除・復旧すること。`

#### 【絶対遵守ルール (Constraints)】
*   **既存ロジックの完全維持**: ログ出力の追加とポーリング間隔の変更のみを行い、既存のデータセット処理やポーリングの条件判定などのロジック自体には一切の変更・削除を行わないこと。
