#### 作業指示書 REQ: Drumエンジンのコールバック駆動化とシーケンス分離 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_DrumEngine_CallbackDriven.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **要件1: Drumアニメーション開始インターフェースの刷新**
    *   `Renderer` 側のドラム制御メソッド（`SetDrumTarget` 等）を `StartDrumAnimation` のような名前に変更し、以下の3つの引数を受け取るようにシグネチャを改修する。
        1. `int relativeDistance`: 目標までの相対距離。
        2. `std::function<TrackMetadata(int relativeIndex)> dataProvider`: Drumが中間データを取り寄せるためのコールバック。
        3. `std::function<void()> onComplete`: アニメーション完了時に発火するコールバック。
*   **要件2: Drum側でのオンデマンド「取り寄せ」とフリップ**
    *   `UpdateAnimation` 内にて、ドラムが1スロット分回転する（視界から消える）たびに、現在の速度や進行度から「次に表示すべき相対インデックス」を計算する（間引き計算）。
    *   そのインデックスを引数として保持している `dataProvider` を呼び出し、必要なメタデータ**のみ**を取得する。
    *   取得したメタデータをフリップ後の新しい `DrumSlot`（カレント）へディープコピーし、画像は `nullptr` とする。
*   **要件3: Application側でのシーケンス管理 (完了通知の受け取り)**
    *   `Application` 側でジャンプ等の操作が発生した際、`StartDrumAnimation` を呼び出す。
    *   `onComplete` コールバックの内部に、アニメーション完了後に行うべき「正規画像のDrumSlotへの適用」「再生状態の確定」などのシーケンス進行処理を記述し、アニメーション管理の責務をApplication側へ戻す。

#### 【絶対遵守ルール (Constraints)】
*   **完全なデカップリング**: `TrackDrum`（Renderer側のエンジン）内部から `PlaylistManager` や `TrackDatabase` を直接参照するようなコードを絶対に書いてはならない。外部データの取得は 100% `dataProvider` コールバックを経由すること。
