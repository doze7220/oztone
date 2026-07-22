##### 作業指示書 REQ: Hotfix / 仮想スクロールバグ修正 最終形態-B (方向反転とサムネイル消失の復旧) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは仮想スクロール機能の最終的な不具合（方向逆転と画像消失）を修正するHotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VirtualScrollFinal_B.md）として新規作成すること。原因と対応内容を追記すること。
4. チャットにて「仮想スクロールの方向反転とサムネイル復旧が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
現在、マウスホイールによる仮想スクロールの移動方向が逆になっている問題と、スクロール開始時にドラムのアルバムアートが消失してしまう問題（古い仕様の残骸によるセット漏れ）が発生している。以下の2点を修正する。

*   **要件1: スクロール方向と符号の完全反転**
    *   `src/Application_Initialize.cpp` （またはコールバック登録箇所）の仮想スクロール入力処理において、ホイール方向（`delta`）に対する処理が逆になっている。
    *   現在の `delta > 0` (UP) と `delta < 0` (DOWN) に対する `m_playlistManager.Previous() / Advance()` の呼び出し、および `m_trackDrum.StartDrumAnimation` に渡す距離の符号（+1 / -1）の割り当てを **完全に逆に書き換える**。
*   **要件2: `dataProvider` 内でのサムネイル画像取得の復旧（残骸パージ）**
    *   同コールバック内で `StartDrumAnimation` に渡している `dataProvider` ラムダ式を修正する。
    *   現在、ラムダ式内で `slot->artBitmap` に画像が正しくセットされていない（または `nullptr` にされている古い仕様の残骸がある）。
    *   ここに、通常の再生時（`PlayCurrentTrack`）と全く同じ **「サムネイルDBからの画像取得とセット処理」を補完する**。
    *   具体的には、対象パスから `m_thumbnailDatabase.GetOrGenerateThumbId` でIDを取得し（`isNew`がtrueでもエンキューは絶対にしないこと）、`m_thumbnailDatabase.GetCachedThumbnailBitmap` を用いて画像を `slot->artBitmap` にセットし、キャッシュにない場合は `RequestThumbnailLoad` を呼ぶ処理を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **アーキテクチャの純化** : 仮想スクロール中であっても、ドラムスロットには常に正規のサムネイル画像を供給すること。画像を意図的に消す（`nullptr`にする）ような古いワークアラウンドは全てパージすること。
