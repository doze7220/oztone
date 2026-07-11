### 作業指示書 REQ: Phase 18-1 Hotfix: ピン留め時の下部UI判定ズレ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（`D:\ozlab\oztone\_docs\RES(Hotfix)_template.md`）を元に、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHSS_RES_Hotfix_Phase18-1_PlaylistPinning.md`）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
*   **不具合の原因** : Phase 18-1でプレイリストのピン留め機能を追加した際、描画側（`LayoutCalculator`）にはピン留めに伴う「キャンバスの動的圧縮（幅の縮小・X座標のオフセット）」処理を追加したが、入力判定側（`Window` クラスの下部UI判定ロジック）にその補正を追加し忘れていた。そのため、UIの表示位置が圧縮・移動しているにも関わらず、当たり判定が元の広いウィンドウ位置のまま取り残され、シークバーや再生・音量コントロールのクリック・ホバー判定がズレてしまっていた。（描画と判定の二重管理による乖離）
*   **修正箇所** : `src/Window.cpp` にある下部コントロール領域のヒットテストメソッド群（`IsInPlaybackControlRegion`, `GetPlaybackButtonAt`, `IsInVolumeControlRegion` など、マウスのX座標を用いて領域やボタン位置を判定している箇所すべて）
*   **修正内容** :
    1. 物理座標から論理座標 (`logicalX`, `logicalWidth` 等) を算出した直後に、`m_config->GetIsPlaylistPinned()` が `true` の場合、プレイリストの幅 (`playlistWidth = m_config->GetPlaylistWidth()`) を用いて座標と幅を仮想的に補正する処理を追加する。
    2. 左配置 (`m_config->GetPlaylistPosition() == 0`) の場合：プレイリストの幅の分だけコントロールの描画起点が右にズレているため、マウス座標をそれに合わせるべく `logicalX -= playlistWidth; logicalWidth -= playlistWidth;` と補正する。もし補正後の `logicalX < 0` であれば、それはプレイリスト領域内へのマウスアクションであるため `false` または該当なし（-1 等）として早期リターンする。
    3. 右配置 (`m_config->GetPlaylistPosition() == 1`) の場合：`logicalWidth -= playlistWidth;` と補正する。もし `logicalX >= logicalWidth` であれば同様に早期リターンする。
    4. 補正された `logicalX` と `logicalWidth` を用いて、既存のボタン位置計算や領域判定ロジックをそのまま実行させる。これにより、描画側（LayoutCalculator）の圧縮ロジックと当たり判定が完全に同期する。
