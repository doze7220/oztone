##### 作業指示書 REQ: Drumエンジンのイベント駆動化と意思決定の純化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DrumEngine_StepCapsulation.md（直前の作業レポート)

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DrumEngine_EventDriven.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: Drumエンジン内部への「イベントハンドラ（完了通知受け取り）」の新設**
    *   `src/Renderer.h` および `src/Renderer.cpp` に、1スロット分のアニメーション完了を処理するイベントハンドラメソッド `void OnSlotAnimationCompleted();`（または類似の名称）を private（あるいはカプセル化された領域）に追加する。
*   **要件2: イベントハンドラ内部での自律的な意思決定ロジック実装**
    *   `OnSlotAnimationCompleted` 内部にて、現在の進行度（相対距離）が目標に達しているかどうかを判断する。
    *   **【目的地に未到達の場合】**: `dataProvider` から次の経由地のメタデータを取得し、`m_currentDrumSlotIndex = 1 - m_currentDrumSlotIndex;` でフリップを行い、データを新スロットにコピーする。画像はクリアする。
    *   **【目的地に到達した場合】**: フリップは行わず、保持している `onComplete` コールバックを呼び出してアニメーション完了を外部（Application）へ通知し、ドラムエンジンの待機状態へ移行する。
*   **要件3: UpdateAnimation の「連絡係」への完全純化**
    *   `src/Renderer_Update.cpp` の `UpdateAnimation` では、物理演算によって「1スロット分アニメーションが進んだ（視界から消えた）」ことを検知した際、直接フリップ操作やデータの代入、あるいは `onComplete` の呼び出しを **絶対に行わない**。
    *   代わりに、**要件1で新設した `OnSlotAnimationCompleted()` を呼び出す（「アニメが終わったぞ」と報告する）のみの処理** へと純化させる。

#### 【絶対遵守ルール (Constraints)】
*   **命令ではなく通知（Event-Driven）**: アニメーション計算層は「状態の変更を命じる」のではなく「事実を通知する」こと。フリップするかどうか、終了するかどうかの判断と実行はすべて通知を受けたエンジン本体（`OnSlotAnimationCompleted` 等）の自己責任とすること。

---------------------------------------------------------------------------------------------
###### 【実装要件】
*   **要件1: UpdateTextLayoutsからのDrumSlot上書き処理の完全パージ**
    *   `src/Renderer_Update.cpp` の `UpdateTextLayouts` メソッド内部を精査し、`m_drumSlots[m_currentDrumSlotIndex]` のテキスト情報（`trackTitle`, `trackArtist`, `trackNumber` 等）に対して、外部から取得した最新情報を毎フレーム代入（上書きコピー）している処理を **完全に削除（パージ）** すること。
*   **要件2: 依存の排除と責務の純化**
    *   `UpdateTextLayouts` は、既に `DrumSlot` の中に入っているテキスト情報を用いて `IDWriteTextLayout` などの「描画用リソースの再計算・キャッシュ構築」を行うだけの純粋なメソッドへと改修すること。
    *   外部データの取得は、`OnSlotAnimationCompleted` 内での `dataProvider` 呼び出しくらいでしか行ってはならない。
*   **要件3: タグの非同期更新（自己修復）反映用の安全な窓口の確保**
    *   もし非同期ロードでタグ情報や画像が遅れて到着した場合に備え、`Renderer.h/cpp` に `void UpdateCurrentDrumSlot(const TrackMetadata& updatedData);` のような安全なメソッドがなければ追加する。このメソッドは、**アニメーションが停止している（待機中）の場合にのみ** カレントスロットを上書きすることを許可する設計とすること。
