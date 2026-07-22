##### 作業指示書 REQ: Hotfix / プレイリストのピン留め機能のアーキテクチャ適正化と負の遺産パージ
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは、プレイリストのピン留め（固定）機能のアーキテクチャを適正化し、他UIのホバーを阻害するバグを修正するとともに、過去のワークアラウンドをパージするHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_PlaylistPinningRefactoring.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. チャットにて「プレイリストのピン留め機能の適正化および負の遺産パージが完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
現在、プレイリストのピン留め（`IsPlaylistPinned`）が有効な場合、`Window` 層のホバー判定が過剰に専有され、他の UI（TrackInfo等）のホバー判定を阻害する不具合が発生している。これを解消するため、「Window層での強制判定」を廃止し、「Widget層での離脱ディレイの維持」によって展開状態を保つアーキテクチャへと改修する。さらに、それに伴い不要となる過去のワークアラウンドを削除する。

*   **要件1: Window層の純化 (強制ホバー専有の廃止)**
    *   `src/Window_Mouse.cpp` の `IsInPlaylistRegion` 等において、ピン留めフラグ（`GetIsPlaylistPinned()`）を用いて「マウス座標に関わらずホバー領域を強制的に拡張（または true を返す）」しているロジックを **完全に削除** する。
    *   `Window` 層は、純粋に「マウスの物理・論理座標がプレイリストの展開領域内に存在するかどうか」だけの判定を行うように純化する。
*   **要件2: Widget層でのディレイ減算停止による展開維持**
    *   `src/Widget_Playlist_Update.cpp` の `UpdateAnimation` メソッドにおいて、離脱ディレイ（`m_playlistLeaveTimer` 等）のタイマー減算処理を改修する。
    *   `ctx.isPlaylistHovered` が false であっても、`config->GetIsPlaylistPinned()` が true の場合は、**タイマーを減算しない（または常に最大値にセットし続ける）** ように変更する。
    *   これにより、マウスが外れてもタイマーが 0 にならないため、スライドアウト（格納アニメーション）が発生せず、常に開きっぱなしの状態が維持される。
*   **要件3: 過去のピン留め用ワークアラウンドのパージ**
    *   `src/Window_Mouse.cpp` の `HandleRButtonDown` （背景フレーミング操作の判定）や `HandleLButtonDown` （背景クリックの判定）において、ピン留めによる `m_isPlaylistHovered == true` の常時化を回避するために入れられていた迂回ロジック（例: `!IsInPlaylistRegion(xPos, yPos)` への置き換えなど）を特定し、本来の美しくシンプルな排他制御（`!m_isPlaylistHovered` を用いる形など）へと戻し、不要な条件式をクリーンアップする。

#### 【絶対遵守ルール (Constraints)】
*   **責務の厳守** : Window層は「入力の検知」、Widget層は「アニメーション（状態）の維持」という責務を徹底すること。
*   **スコープの制限** : 本タスクはピン留めに関するバグ修正および関連するコードの純化のみを対象とする。TrackInfo領域のホバー判定サイズ異常など、他のバグ修正は含めないこと。
