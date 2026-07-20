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
- `src/AudioPlayer.h` (編集)
- `src/AudioPlayer.cpp` (編集)
- `CMakeLists.txt` (編集)

## 4. 実装タスクリスト
- [x] タスク1: AudioAnalyzer の新設と ScanAudioData 処理の移管
- [x] タスク2: AudioPlayer を AudioPlaybackEngine へリネーム・純化
- [ ] タスク3: AudioManager の新設（Facadeクラスの構築）
- [ ] タスク4: 外部クラス (Application, TrackAnalyzer) の配線付け替え
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
- (実行後記載)

### タスク4: 外部クラス (Application, TrackAnalyzer) の配線付け替え
- (実行後記載)

### タスク5: CMakeLists.txt および PROJECT_ARCHITECTURE.md の更新
- (実行後記載)
