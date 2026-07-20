# RES:実装計画・作業レポート Phase 23-3: AudioManagerの新設

## 1. 実装目的
本フェーズでは、「音に関すること」をすべて統括する司令塔である `AudioManager` クラスを新設し、既存の音声関連クラスをその配下にカプセル化・再編する。これにより、外部（Application等）に対する音声操作の窓口を単一化し、巨大化しつつあった `AudioPlayer` を解体・純化するとともに、外部へのライブラリ依存（miniaudio）を完全に隠蔽する。

## 2. アーキテクチャ設計
### 要件1: AudioManager クラスの新設
- `src/AudioManager.h` および `src/AudioManager.cpp` を新規作成し、音声再生および解析の唯一の外部インターフェース（Facade）として機能させる。
- `miniaudio.h` などの外部ライブラリ依存は内部（ヘッダの先など）に閉じ込め、外部に露出しない設計とする。

### 要件2: AudioPlayer の卒業とカプセル化
- 現在の `AudioPlayer` クラスを `AudioPlaybackEngine` へリネームし、再生機能に特化した内部コンポーネントへ純化する。
- 外部（Application等）からは `AudioPlaybackEngine` を隠蔽し、`AudioManager` 経由でのみアクセス可能にする。

### 要件3: TrackAnalyzer の音声解析責務の統合
- 現在 `AudioPlayer` に実装されている `ScanAudioData` (FFT波形事前スキャン処理) を、新規コンポーネント `AudioAnalyzer` として分離・カプセル化する。
- `TrackAnalyzer` が行っていた音声スキャン要求は、すべて `AudioManager` 経由（内部的に `AudioAnalyzer` へ委譲）で行うように統合する。

### 要件4: 外部クラスの配線付け替え
- `Application.h/cpp` にある `AudioPlayer` のインスタンスや利用箇所を `AudioManager` へ差し替える。
- `TrackAnalyzer.cpp` における `AudioPlayer::ScanAudioData` の呼び出しを `AudioManager::ScanAudioData` へ差し替える。

## 3. 対象ファイル
- `src/AudioAnalyzer.h` (新規作成)
- `src/AudioAnalyzer.cpp` (新規作成)
- `src/AudioManager.h` (新規作成)
- `src/AudioManager.cpp` (新規作成)
- `src/AudioPlayer.h` (編集)
- `src/AudioPlayer.cpp` (編集)
- `src/Application.h` (編集)
- `src/Application.cpp` (編集)
- `src/Application_FileDrop.cpp` (編集)
- `src/Application_Initialize.cpp` (編集)
- `src/Application_Playback.cpp` (編集)
- `src/Application_Playlist.cpp` (編集)
- `src/Application_Render.cpp` (編集)
- `src/TrackAnalyzer.cpp` (編集)
- `CMakeLists.txt` (編集)

## 4. 実装タスクリスト
- [x] タスク1: AudioAnalyzer の新設と ScanAudioData 処理の移管
- [x] タスク2: AudioPlayer を AudioPlaybackEngine へリネーム・純化
- [x] タスク3: AudioManager の新設（Facadeクラスの構築）
- [x] タスク4: 外部クラス (Application, TrackAnalyzer) の配線付け替え
- [ ] タスク5: CMakeLists.txt および PROJECT_ARCHITECTURE.md の更新

## 5. 詳細作業内容
### タスク1: AudioAnalyzer の新設と ScanAudioData 処理の移管
- `src/AudioAnalyzer.h` および `src/AudioAnalyzer.cpp` を新規作成。
- `AudioPlayer` にあった `ScanAudioData` の実体、およびFFT処理（`PerformFFT` 等）を `AudioAnalyzer` に移管。
- タスク制約「外部クラスからの呼び出し箇所の修正はフライングで行わない」を遵守するため、一時的に `AudioPlayer::ScanAudioData` （非推奨・委譲用）を残し、ビルドを維持しつつ該当処理の実体を分離。
- `CMakeLists.txt` を更新してビルド成功を確認。

### タスク2: AudioPlayer を AudioPlaybackEngine へリネーム・純化
**【作業内容】**
- `src/AudioPlayer.h` と `src/AudioPlayer.cpp` 内のクラス定義を `AudioPlayer` から `AudioPlaybackEngine` にリネーム。
- 既存の外部ファイル (Application 等) のコンパイルエラーを回避するため、`src/AudioPlayer.h` の末尾に `using AudioPlayer = AudioPlaybackEngine;` のエイリアスを追記。
- ファイル名 (`AudioPlayer.h/cpp`) は現状のまま維持し、影響範囲を最小限に抑制。

### タスク3: AudioManager の新設（Facadeクラスの構築）
**【作業内容】**
- `src/AudioManager.h` および `src/AudioManager.cpp` を新規作成。
- `AudioPlaybackEngine` を `std::unique_ptr` で保持し、各種音声操作メソッド（`Play`, `Stop`, `Pause`, `Seek`, `SetVolume`, `GetSpectrumData` 等）を委譲。
- `miniaudio.h` などの依存が外部へ漏出しないようにカプセル化（前方宣言を使用）。
- `AudioAnalyzer` の `ScanAudioData` を呼び出す静的メソッドを追加。
- 新規作成した2ファイルを `CMakeLists.txt` に追加。

### タスク4: 外部クラス (Application, TrackAnalyzer) の配線付け替え
**【作業内容】**
- `Application.h` および `Application_*.cpp` (Playback, Render, Initialize, Playlist, FileDrop 等) 内の `AudioPlayer m_audioPlayer` を `AudioManager m_audioManager` に置換し、各種メソッド呼び出しを移行。
- `TrackAnalyzer.cpp` のバックグラウンド音声解析における `AudioPlayer::ScanAudioData` を `AudioManager::ScanAudioData` 呼び出しに置換。
- `AudioPlayer.h` に一時的に追加していた `using AudioPlayer = AudioPlaybackEngine;` のエイリアスを削除し、外部からの直接利用を完全に遮断。
### タスク5: CMakeLists.txt および PROJECT_ARCHITECTURE.md の更新
- (実行後記載)
