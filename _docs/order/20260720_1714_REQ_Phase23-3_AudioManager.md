##### 作業指示書 REQ: Phase 23-3: AudioManagerの新設 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反（Managerの巨大化など）がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-3_AudioManager.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
本フェーズでは、「音に関すること」をすべて統括する司令塔である `AudioManager` クラスを新設し、既存の音声関連クラスをその配下にカプセル化・再編する。
*   **要件1: AudioManager クラスの新設**
    *   `src/AudioManager.h` および `src/AudioManager.cpp` を新規作成する。
    *   外部（Application等）に対する音声操作（再生、停止、シーク、音量調整、リアルタイムFFTスペクトル取得、波形の事前解析など）の唯一の窓口となるAPIを設計する。
*   **要件2: AudioPlayer の卒業とカプセル化**
    *   現在「神クラス」になりかけている `AudioPlayer` クラスを解体、または `AudioManager` 内部の専用コンポーネント（PlaybackEngine等の概念）としてカプセル化し、「AudioPlayer」という名前のクラスを外部の依存から完全に隠蔽（あるいは内部クラスへリネーム）する。
*   **要件3: TrackAnalyzer の音声解析責務の統合**
    *   `TrackAnalyzer` が行っていたFFT波形事前スキャンなどの音声解析処理を、`AudioManager` 配下の責務（AudioAnalyzer等の概念）として統合・整理する。
*   **要件4: 外部クラスの配線付け替え**
    *   `Application` 等で直接 `AudioPlayer` や `TrackAnalyzer` の音声解析機能にアクセスしていた箇所を、すべて新設した `AudioManager` 経由に置き換える。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-3_AudioManager.md）に、詳細作業内容を記載する（タスクリストに含める）こと。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回は `AudioManager` の新設と、既存音声クラス（AudioPlayer, TrackAnalyzerの音声部分）のカプセル化・配線付け替えのみを行う。将来的な「メモリ再生への移行（FileManagerとの結線）」などは後続フェーズで行うため、ファイルパスから再生している現在の miniaudio の挙動自体は無理に変更しなくてよい。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-3 Task 1 : AudioAnalyzer の新設と ScanAudioData 処理の移管
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1717_RES_Phase23-3_AudioManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-3 Task 1: AudioAnalyzer の新設と ScanAudioData 処理の移管である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1717_RES_Phase23-3_AudioManager.md）を読み、今回の自分のスコープが「タスク1」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1717_RES_Phase23-3_AudioManager.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
5. チャットにて「タスク1の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在 `AudioPlayer` クラスが抱え込んでしまっている「波形スキャン（事前解析）」の責務を、専用のコンポーネントとして分離・独立させる。
*   **要件1: AudioAnalyzer クラスの新設**
    *   `src/AudioAnalyzer.h` および `src/AudioAnalyzer.cpp` を新規作成すること。
*   **要件2: ScanAudioData 処理の移管**
    *   現在 `src/AudioPlayer.h` / `.cpp` に実装されている `ScanAudioData` メソッド（およびそれに付随するFFT処理やヘルパー関数）を `AudioAnalyzer` に移植すること。
    *   `AudioPlayer` 側からは該当のコードを完全に削除（カット）し、再生エンジンとしての純度を高める準備をすること。
*   **要件3: CMakeLists.txt の更新**
    *   新設した `src/AudioAnalyzer.cpp` および `src/AudioAnalyzer.h` を CMakeLists.txt に追加し、ビルドを通すこと。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは部品の切り離し（`AudioAnalyzer` の新設と `AudioPlayer` からのコード移動）のみを行う。`AudioPlayer` のリネーム（タスク2）や、他の外部クラスからの呼び出し箇所の修正（タスク4）は絶対にフライングで行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-3 Task 2 : AudioPlayer の純化とリネーム
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1717_RES_Phase23-3_AudioManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-3 Task 2: AudioPlayer の純化とリネームである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1717_RES_Phase23-3_AudioManager.md）を読み、今回の自分のスコープが「タスク2」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク2のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1717_RES_Phase23-3_AudioManager.md）の「タスク2」のチェックボックスを完了 [x] にし、今回追加された新しいテンプレート書式（対象ファイルの列挙含む）に従って詳細作業内容を追記すること。
5. チャットにて「タスク2の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在「神クラス」になりかけている `AudioPlayer` を、再生機能に特化した `AudioPlaybackEngine` へとリネームし、内部コンポーネントとして純化させる。
*   **要件1: クラスのリネームと純化**
    *  `src/AudioPlayer.h` および `src/AudioPlayer.cpp` 内のクラス定義名 `AudioPlayer` を `AudioPlaybackEngine` へ変更すること。
    *  コンストラクタやデストラクタなどのメソッドシグネチャもすべて新しいクラス名に合わせて変更し、不要なコードが残っていないか純化すること。
    *  ※影響範囲を最小限にするため、ファイル名自体のリネーム（`AudioPlaybackEngine.h/cpp` への変更）は今回は絶対に行わないこと。
*   **要件2: 既存クラス向けのビルド維持（一時的エイリアス）**
    *  クラス名のみを変更すると `Application` などの外部クラスでコンパイルエラーが発生してしまうため、配線切り替え（タスク4）までの安全な「つなぎ」として、`src/AudioPlayer.h` の末尾（クラス定義の直後など）に `using AudioPlayer = AudioPlaybackEngine;` を追記し、既存のコードがそのままビルドを通る状態を死守すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `AudioPlayer` クラスの純化と内部リネームのみを行う。`AudioManager` の新規作成（タスク3）や外部クラスでの呼び出し名の書き換え（タスク4）は絶対にフライングで行わないこと。

-------------------------------------------------------------------------------

### 作業指示書 REQ: Phase 23-3 Task 3 : AudioManager の新設
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1717_RES_Phase23-3_AudioManager.md

#### 【作業手順（厳守事項）】
本プロンプトはPhase 23-3 Task 3: AudioManager の新設である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1717_RES_Phase23-3_AudioManager.md）を読み、今回の自分のスコープが「タスク3」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク3のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク4以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1717_RES_Phase23-3_AudioManager.md）の「タスク3」のチェックボックスを完了 [x] にし、今回追加された新しいテンプレート書式（対象ファイルの列挙含む）に従って詳細作業内容を追記すること。
5. チャットにて「タスク3の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
「音に関すること」をすべて統括する司令塔となる `AudioManager` クラスを新規に構築する。
*   **要件1: AudioManagerクラスの新設**
    *  `src/AudioManager.h` および `src/AudioManager.cpp` を新規作成すること。
    *  クラスのメンバとして、タスク1・2で分離・純化させた `AudioPlaybackEngine` (旧AudioPlayer) と `AudioAnalyzer` を保持（包含またはポインタ管理）すること。
*   **要件2: Facade（窓口）メソッドの定義と委譲**
    *  外部（Application等）からの音声操作要求を一手に引き受けるため、`Play`, `Stop`, `Pause`, `Seek`, `SetVolume`, `GetSpectrumData` など、既存の音声関連メソッドと同等のパブリックメソッドを定義すること。
    *  定義したメソッドの内部では、実際の処理をメンバである `AudioPlaybackEngine` や `AudioAnalyzer` へとルーティング（委譲）すること。
    *  ※ このファイル内で必要なインクルードのみを行い、`Application` など外部のクラスに `miniaudio` などの依存が漏れないようカプセル化を徹底すること。
*   **要件3: CMakeLists.txtの更新**
    *  新規作成した `src/AudioManager.cpp` および `src/AudioManager.h` を CMakeLists.txt に追加し、ビルドが通る状態を維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `AudioManager` の新設（枠組み作り）のみを行う。`Application` や `TrackAnalyzer` など、他の外部クラスからの呼び出し箇所の書き換え（タスク4）は絶対にフライングで行わないこと。
