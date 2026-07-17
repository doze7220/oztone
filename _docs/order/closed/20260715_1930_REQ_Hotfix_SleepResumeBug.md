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

------------------------------------------------------------------------------------------------
### 作業指示書 REQ: Hotfix スリープ復帰バグ追加対応（デバイス復旧ポーリング機構） (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260715_1930_RES_Hotfix_SleepResumeBug.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\20260715_1930_RES_Hotfix_SleepResumeBug.md）の末尾に「HOTFIX: デバイス復旧ポーリング機構の追加」の項目を追加し、原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「デバイス復旧ポーリング機構の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
スリープ復帰時（`PBT_APMRESUMEAUTOMATIC`）に、OSのオーディオデバイスが認識される前に `miniaudio` が初期化されてしまい、ダミーデバイスを掴んで無音で再生が進んでしまうバグを修正する。復帰時に即座にエンジンを再構築するのではなく、有効なデバイスが検出されるまで待機（ポーリング）する「遅延蘇生」を実装する。

*   **要件1: デバイス存在確認メソッドの追加 (`AudioPlayer`層)**
    *   `AudioPlayer` クラスに、現在有効な出力デバイスが存在するかどうかを確認するメソッド（例: `HasValidOutputDevice()`）を実装する。
    *   `miniaudio` のデバイス列挙API（`ma_context_get_devices` など）を用いて、出力デバイス（Playback devices）の数が1つ以上あるかを判定する処理を構築すること。

*   **要件2: レジューム待機状態の導入 (`Application`層)**
    *   `Application` クラスに、デバイス復旧待ち状態を示すフラグ（例: `m_isWaitingForDevice` = false）を追加する。
    *   スリープ復帰コールバック (`OnPowerResume`) 内での即時再構築（`Initialize` と `Play`）をやめ、代わりに `m_isWaitingForDevice = true;` をセットして待機モードに移行する。

*   **要件3: メインループでの監視と遅延蘇生 (`Application`層)**
    *   `Application::Run` または `ForceRender` などの毎フレーム処理内にて、`m_isWaitingForDevice` が true の場合のポーリング処理を追加する。
    *   毎フレーム `m_audioPlayer.HasValidOutputDevice()` を確認し、`false` の間は何もしない（シークバーも進めない）。
    *   `true`（デバイスがOSに認識された）になった瞬間に、待機フラグを解除し、`m_audioPlayer.Initialize()` でエンジンを再構築する。
    *   その後、サスペンド時に退避しておいた状態（再生中の曲パス、再生位置秒数）をもとに再ロード（`Play`）とシーク（`Seek`）を行い、再生状態を完全に復旧させる。

#### 【絶対遵守ルール (Constraints)】
*   **ダミーデバイスの回避**: デバイスが復旧するまでは絶対に `miniaudio` のエンジン（`ma_engine` や `ma_sound`）を初期化しないこと。
*   **処理負荷の抑制**: デバイス列挙APIの呼び出しは、`m_isWaitingForDevice` が `true` の期間（通常はスリープ復帰後の数秒間）のみ実行し、平常時の再生ループに一切の負荷をかけないこと。
