##### 作業指示書 REQ: Drumエンジンのカプセル化とフリップ更新の純化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\closed\20260718_2213_RES_Hotfix_Phase21-9_DoubleBuffering.md ※該当機能を実装した作業レポート

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DrumEngine_Capsulation.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: Drumエンジンの入力インターフェース純化 (`SetDrumTarget`等)**
    *   `src/Renderer.cpp` (またはDrumを管理する層) に存在する、目標をセットするメソッド（`SetDrumTarget` 等）の引数を変更し、新しい曲のインデックスや相対距離だけでなく、「新しい曲のメタ情報（テキスト等）」を直接引数として受け取るように改修する（例: `SetDrumTarget(const TrackMetadata& newTrack)`）。
*   **要件2: 内部フリップとコピーへの完全集約 (DrumSlotへの直接操作禁止)**
    *   外部（`Application` 等）でDrumSlotの中身を操作・コピーする処理やバッファ構築処理があれば全て削除する。
    *   `SetDrumTarget` の内部ロジックを以下の3ステップのみに純化させる。
        1. スロットのフリップ (`m_currentDrumSlotIndex = 1 - m_currentDrumSlotIndex;`)
        2. フリップ後の新しいカレントスロット (`drumSlots[m_currentDrumSlotIndex]`) の画像（artBitmap）を一旦クリア（またはフォールバック状態）にする。
        3. 同カレントスロットに、引数で渡された `newTrack` のテキスト情報（曲名、アーティスト等）をディープコピー（代入）する。
    *   ※ これにより、フリップによって裏へ回ったOLDスロットは一切上書きされず、前の曲のメタ情報と画像が安全に保護される。
*   **要件3: 描画層（Widget）からの外部参照の完全パージ**
    *   `src/Widget_TrackInfo.cpp` の描画処理において、テキスト情報を `shuffleMetadataList` などの外部プレイリストから抽出（参照）している処理を **完全に削除** する。
    *   カメラ `i` の位置が `0`（カレント）なら `drumSlots[ctx.currentDrumSlotIndex]` を、`ctx.animatingOldIndexOffset`（過去）なら `drumSlots[1 - ctx.currentDrumSlotIndex]` からテキストと画像を抽出して描画する。
    *   それ以外の中間スロットについては、テキストも画像も一切抽出せず（空のデータとして扱い）、ガラス板のフォールバックのみを描画する。

#### 【絶対遵守ルール (Constraints)】
*   **OLDスロットの不可侵**: OLDスロットのデータを外部から操作・コピー・計算してはならない。スロットの更新は「フリップしてNOWを上書きする」というDrumエンジン内部の単一のメソッド呼び出しのみで完結させること。
