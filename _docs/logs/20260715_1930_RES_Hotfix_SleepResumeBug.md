# RES:HOTFIX作業レポート: スリープ復帰時のオーディオエンジン無限ループ（ゾンビ化）バグ修正

## 1. 実装目的
OSのスリープ復帰時に、miniaudioが消失したオーディオデバイスを捕捉できずプロセスがゾンビ化する問題を防ぐため、OSの電源イベントをフックしてオーディオエンジンのサスペンド・レジューム処理を安全に行う。

## 2. 調査内容
miniaudioの仕様上、オーディオデバイスが消失した際に内部のオーディオスレッドがスピンループに陥るケースがある。これを防ぐには、システムがスリープに入る前(`WM_POWERBROADCAST`の`PBT_APMSUSPEND`時)に明示的にデバイスを破棄(`Uninitialize`)し、復帰時(`PBT_APMRESUMEAUTOMATIC`)に再構築(`Initialize`)して再生状態を復旧させる必要がある。

## 3. 対象ファイル
* src/Window.h
* src/Window_Proc.cpp
* src/Application.h
* src/Application_Initialize.cpp
* src/Application.cpp

## 4. 実装タスクリスト
[x] タスク1: OS電源イベントのフック (`Window`層)
[x] タスク2: オーディオエンジンのサスペンド対応 (`Application`層)
[x] タスク3: オーディオエンジンのレジューム対応 (`Application`層)

## 5. 詳細作業内容
* タスク1: OS電源イベントのフック (`Window`層)
    - `src/Window.h`にスリープ移行・復帰を通知するためのコールバック(`SetPowerSuspendCallback`, `SetPowerResumeCallback`)を追加した。
    - `src/Window_Proc.cpp`にて`WM_POWERBROADCAST`メッセージを捕捉し、`PBT_APMSUSPEND`と`PBT_APMRESUMEAUTOMATIC`に応じて各コールバックを発火させるようにした。
* タスク2: オーディオエンジンのサスペンド対応 (`Application`層)
    - `src/Application.h`に、サスペンド時の再生状態(`m_suspendIsPlaying`)と再生位置(`m_suspendPosition`)を退避するメンバ変数を追加した。
    - `src/Application.cpp`にて`OnPowerSuspend()`を実装し、状態退避後に`m_audioPlayer.Uninitialize()`を呼び出してデバイスを解放するようにした。
* タスク3: オーディオエンジンのレジューム対応 (`Application`層)
    - `src/Application.cpp`にて`OnPowerResume()`を実装し、`m_audioPlayer.Initialize()`で再構築を行うようにした。
    - 保存した状態から対象曲を再ロードし、再生位置へのシークおよび再生・一時停止状態の復旧処理を実装した。
    - `src/Application_Initialize.cpp`の`SetupCallbacks()`にて各コールバックのバインドを行った。
