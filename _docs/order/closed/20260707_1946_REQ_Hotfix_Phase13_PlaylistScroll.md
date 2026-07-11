### 作業指示書 REQ: Hotfix_Phase13 (追加): プレイリスト描画時のスクロールクランプ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（`D:\ozlab\oztone\_docs\RES(Hotfix)_template.md`）を元に、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHSS_RES_Hotfix_Phase13_PlaylistScroll.md`）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
*   **不具合の原因** : Phase 13でツールバーを導入した際、ウィンドウの高さが低くなった場合（510px以下等）に、ツールバーの高さを差し引いたプレイリストの描画可能領域（viewHeight）が狭くなり、スクロールの下限値（minScroll）と上限値（maxScroll）の計算において大小関係が逆転、または意図しない負の値に引っ張られる現象が発生していた。その結果 `std::clamp` が未定義の挙動または意図しない値を返し、一番上の要素がツールバーの下に潜り込んで描画されてしまっていた。
*   **修正箇所** : `src/Widgets.cpp` の `PlaylistWidget::Draw`（およびスクロール量のクランプ計算を行っている箇所）
*   **修正内容** :
    1.  スクロール量の限界値を計算する際、表示可能領域 `viewHeight = layout.clipRect.bottom - layout.clipRect.top;` とリストの総高さ `totalHeight = totalItems * layout.itemHeight;` を正確に使用する。
    2.  `maxScroll = 0.0f;` （または上部の適正なマージン）とし、`minScroll = viewHeight - totalHeight;` と計算した上で、**`if (minScroll > maxScroll) { minScroll = maxScroll; }`** のように、要素数が少ない場合やウィンドウが極端に狭い場合の大小逆転を防ぐフェイルセーフ（ガード処理）を必ず入れること。
    3.  その上で `std::clamp` を適用し、描画Y座標の起点が `layout.clipRect.top` を下回らない（潜り込まない）ように安全に計算すること。
    4.  曲一覧モード・プレイリスト一覧モードの双方の描画ループ・計算箇所において、同様の安全対策が適用されるように確認・修正すること。

### 作業指示書 REQ: Hotfix_Phase13 (追加3): プレイリスト・リサイズ時のスクロール位置ズレ完全修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（`D:\ozlab\oztone\_docs\logs\20260707_1956_RES_Hotfix_Phase13_PlaylistScroll.md`）の末尾に「HOTFIX 3: リサイズ時のスクロール位置ズレ補正」の項目を追加し、原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
*   **不具合の真の原因** : 
    プレイリストの描画スクロール位置は `baseScrollY` (現在の曲を中央にする座標) + `m_playlistManualScrollY` (手動スクロール量) で決定される。
    `baseScrollY` の計算には `viewHeight / 2.0f` が含まれているため、ウィンドウの高さを縮めると `viewHeight` が減少し、`baseScrollY` も減少する。
    しかし、広いウィンドウの時に上端を表示するために蓄積された `m_playlistManualScrollY` の負のオフセット値がリサイズ後もそのまま維持されるため、足し合わせた描画Y座標が本来よりもマイナス方向にズレてしまい、「一番上の要素がツールバーの下に潜り込んでしまう」現象が発生していた。
*   **修正箇所** : `src/PlaylistWidget.h` および `src/PlaylistWidget.cpp` (`PlaylistWidget::Draw`)
*   **修正内容** :
    1. `src/PlaylistWidget.h` に、前回の描画時のビュー高さを記憶するためのメンバ変数 `float m_lastViewHeight = 0.0f;` を追加する。
    2. `src/PlaylistWidget.cpp` の `Draw` メソッド内において、`viewHeight` (`layout.clipRect.bottom - layout.clipRect.top`) を算出した直後に以下のロジックを追加し、リサイズによるズレを完全に相殺する。
       ```cpp
       if (m_lastViewHeight != 0.0f && m_lastViewHeight != viewHeight) {
           float diff = (viewHeight - m_lastViewHeight) / 2.0f;
           m_playlistManualScrollY -= diff;
       }
       m_lastViewHeight = viewHeight;
       ```
    3. この補正を入れることで、ウィンドウを伸縮させてもスクロールの絶対位置（見えている曲の位置）がピタッと固定され、上端にいる時にリサイズしてもツールバーの下に潜り込むことが完全になくなる。曲一覧モードとプレイリスト一覧モードの両方の計算に適用される位置（共通の `viewHeight` 算出直後など）に配置すること。
