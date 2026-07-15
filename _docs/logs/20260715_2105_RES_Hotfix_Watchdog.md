# RES:HOTFIX作業レポート: ウォッチドッグ機構の実装

## 1. 実装目的
オーディオスレッドが原因不明のスピンループ等に陥り再生位置が停滞する問題に対し、メインスレッドから異常を検知し自動でオーディオエンジンを蘇生させる「ウォッチドッグ機構」を実装するため。

## 2. 調査内容
メインループ (`Application::Run`) にて、オーディオの再生状態と位置をインターバルで監視。
再生中にも関わらず一定時間再生位置が進まない場合（Timeout）、既存のスリープ復帰用デバイス復旧待ち機構（`m_isWaitingForDevice`）を利用してオーディオエンジンを安全に強制再起動させる。

## 3. 対象ファイル
* `src/ConfigManager.h`
* `src/ConfigManager_System.cpp`
* `src/ConfigManager_DefaultIni.h`
* `src/Application.h`
* `src/Application_Render.cpp`

## 4. 実装タスクリスト
[x] タスク1: ConfigManager へのパラメータ追加 (`WatchdogInterval`, `WatchdogTimeout`)
[x] タスク2: Application クラスへのウォッチドッグ状態マシンの導入 (`WatchdogState` 追加)
[x] タスク3: Application::Run への2段階監視ロジックの実装
[x] タスク4: Dead確定時に既存のデバイス復旧待ちモジュールを利用した自己修復処理を合流させる

## 5. 詳細作業内容
* タスク1: `ConfigManager_DefaultIni.h` に `[System]` セクションを追加しデフォルト値を定義。`ConfigManager_System.cpp` の `LoadSystemSettings()` 内で `std::clamp` を用い、安全な下限値（0.5, 2.0）を適用した値の読み込みを実装。
* タスク2: `Application.h` に `WatchdogState` enum class、及び状態管理用メンバ変数（`m_watchdogState`, `m_lastWatchdogPollTime`, `m_lastWatchdogPosition`, `m_watchdogWarningStartTime`）を追加。
* タスク3: `Application_Render.cpp` のメインループ先頭で `GetTickCount64()` を用い、設定されたインターバルごとのポーリングで再生位置の停滞を検知し `Warning` 状態へ遷移。その後 `Timeout` に達するまで毎フレーム監視するロジックを追加。
* タスク4: `Timeout` 経過時に `m_suspendPosition` に現在位置を退避、`m_suspendIsPlaying = true` に設定した後、`m_audioPlayer.Uninitialize()` でエンジンを破棄し、`m_isWaitingForDevice = true` に設定してデバイスの安全な再認識・自動復帰処理へ合流させた。
