# RES:HOTFIX作業レポート: グランドフィナーレ前の不具合一掃 (Hotfix 一括実行)

## 1. 実装目的
グランドフィナーレ（最終確認）前に発見された複数の軽微な不具合や要望（MP4拡張子の除外、ビジュアライザの音量依存解消、ヘイローダストビジュアライザの画面サイズ連動、設定初期化時の自動再起動）を一括で修正し、アプリの品質とUXを向上させること。

## 2. 調査内容
- **MP4除外**: `Application::OnFilesDropped` 内にて、サポート外である `.mp4`, `.m4a` 拡張子を実験的に許可していた箇所を特定。
- **ビジュアライザの音量依存解消**: `AudioPlayer::ProcessAudioFrames` にて、オーディオエンジンから渡される波形データがマスターボリューム適用後になっていることを確認。
- **ヘイローダスト画面サイズ連動**: `Visualizer::DrawCircleParticle` にて、描画領域のサイズ(`width`, `height`)に依存せず固定の定数で飛散・速度の計算が行われていることを確認。
- **設定初期化時の再起動**: `Window::WindowProc` の `WM_COMMAND` 処理において、`ID_TRAY_RESET_ALL` メッセージの処理が記述されておらず、以前の処理で終了のみ行われていた（あるいは機能不全だった）ことを確認。

## 3. 詳細作業内容
- **1. MP4除外対応**:
    - `src/Application.cpp` の `IsSupportedAudioFile` および `IsValidAudioFile` ラムダ式内にある `[EXPERIMENTAL] MP4/M4A Support` のブロックをコメントアウトし、プレイリストに当該ファイルが追加されないよう修正した。
- **2. ビジュアライザの音量依存解消**:
    - `src/AudioPlayer.cpp` の `ProcessAudioFrames` にて、`GetVolume()` を用いて現在の音量を取得し、音量が0より大きい場合は波形データ（`monoSample`）を音量で除算（逆算）することで、元の振幅レベルに補正してFFTへ渡すよう処理を改修した。
- **3. ヘイローダスト（円形ビジュアライザ）の画面サイズ連動**:
    - `src/Visualizer.cpp` の `DrawCircleParticle` にて、描画領域の短辺と基準サイズ(1024.0f)の比率から `scaleFactor` を算出。レーザーの長さ(`length`)・速度(`speed`)、パーティクルのサイズ(`size`)・速度(`pSpeed`)に乗算し、ウィンドウサイズに連動した演出となるよう修正した。
- **4. 設定初期化時の自動再起動**:
    - `src/Window.cpp` の `WM_COMMAND` に `case ID_TRAY_RESET_ALL:` を追加。`m_config->SaveDefaultSettings()` で設定を初期化したのち、`GetModuleFileNameW` と `ShellExecuteW` を用いて自身のプロセスを新規起動し、その直後に `WM_CLOSE` を発行して終了（再起動）する処理を実装した。
- **ドキュメント修正**:
    - `PROJECT_ARCHITECTURE.md` から MP4/M4A 関連の実験的サポートに関する記述を削除し、「設定を初期化」時の再起動挙動を追記した。
