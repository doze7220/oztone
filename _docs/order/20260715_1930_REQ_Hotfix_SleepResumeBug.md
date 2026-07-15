### 作業指示書 REQ: Hotfix スリープ復帰時のオーディオエンジン無限ループ（ゾンビ化）バグ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_SleepResumeBug.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
OSのスリープ復帰時に、miniaudioが消失したオーディオデバイスを捕捉できずスピンループに陥り、プロセスがゾンビ化するバグを修正する。
OSの電源イベントをフックし、スリープ移行時と復帰時にオーディオエンジンを安全に再構築する仕組みを導入する。

*   **要件1: OS電源イベントのフック (`Window`層)**
    *   `src/Window_Proc.cpp` の `WindowProc` にて `WM_POWERBROADCAST` メッセージを捕捉する。
    *   `wParam` が `PBT_APMSUSPEND`（スリープ移行）および `PBT_APMRESUMEAUTOMATIC`（スリープ復帰）であることを検知する。
    *   `Window.h` に電源イベント伝達用のコールバック（例: `SetPowerSuspendCallback`, `SetPowerResumeCallback` 等）を追加し、イベント発生時に発火させる。

*   **要件2: オーディオエンジンのサスペンド対応 (`Application`層)**
    *   `Application.cpp` にてサスペンド時のコールバックを受け取る。
    *   スリープに入る直前に、現在の再生状態（再生中かどうか）と **現在の再生位置（秒数）** をメンバ変数に退避して保存する。
    *   `m_audioPlayer.Uninitialize()` などを呼び出して、miniaudioのオーディオスレッドを完全に終了・破棄させ、デバイスを解放する。

*   **要件3: オーディオエンジンのレジューム対応 (`Application`層)**
    *   復帰時のコールバックを受け取る。
    *   `m_audioPlayer.Initialize()` を呼び出し、オーディオエンジンを再構築する。
    *   退避しておいた状態をもとに、対象の曲を再ロード（`Play`）し、保存しておいた再生位置へシーク（`Seek`）して、再生状態を完全に復旧させる。

#### 【絶対遵守ルール (Constraints)】
*   **デッドロックの防止**: `WM_POWERBROADCAST` はOSレベルの割り込みメッセージであるため、コールバック内での処理は迅速に行うこと。また、`Uninitialize()` のスレッドジョインでフリーズしないよう順序に気をつけること。
*   **アーキテクチャの維持**: `Window` クラスは具体的なオーディオ処理を持たず、必ずコールバック経由で `Application` クラスに委譲すること。
