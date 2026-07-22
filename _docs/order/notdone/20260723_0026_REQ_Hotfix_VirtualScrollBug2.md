##### 作業指示書 REQ: Hotfix / 仮想スクロールバグ修正2 (二重発注ブロックとスクロール方向の標準化) (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトは仮想スクロール実装に伴うドラム挙動異常バグの統合Hotfixである。必ず以下の順序で作業を行うこと。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VirtualScrollBug2.md）として新規作成すること。原因と対応内容を追記すること。
4. チャットにて「ドラムバグの修正（二重発注ブロックと方向標準化）が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
本タスクでは、仮想スクロール時のドラム二重発注バグの修正と、マウスホイールの回転方向を「ブラウザやプレイリストの標準的なスクロール挙動」に一致させる改修を行う。

*   **要件1: `PlayCurrentTrack` への二重発注ブロックフラグ追加**
    *   `src/Application.h` および `src/Application_Playback.cpp` の `PlayCurrentTrack` のシグネチャに、仮想スクロール確定であることを示すフラグ `bool isVirtualScrollConfirm = false` を追加する。
    *   `PlayCurrentTrack` 終盤にある `m_trackDrum.StartDrumAnimation(...)` を `if (!isVirtualScrollConfirm)` で囲み、仮想スクロール確定時はドラムアニメーションの追加発注を完全にブロックする。
    *   ブロックした場合（`else`側）は、引数として生成していた `onComplete` ラムダ式を即座に直接実行し、正規画像の適用とフレーミング反映が行われるようにする。
    *   `src/Application_Render.cpp` の仮想スクロール確定処理にて `PlayCurrentTrack(true)` を呼び出しフラグを伝達する。
*   **要件2: ホイール回転方向と論理インデックスの標準化**
    *   `src/Application_Initialize.cpp` (または仮想スクロールのコールバック登録箇所) において、ホイールの `delta` に応じたターゲットインデックスおよびドラムエンジンへ渡す相対距離の増減方向を修正する。
    *   「ホイールUP（`delta > 0`） ＝ 前の曲へ戻る（`-1`）」
    *   「ホイールDOWN（`delta < 0`） ＝ 次の曲へ進む（`+1`）」となるよう、`m_virtualScrollTargetIndex` の加減算と、`m_trackDrum.StartDrumAnimation` へ渡す距離の符号を正しく設定する。
    *   ※ドラムの物理的な描画方向は既存の `TrackDrum` エンジンが自動的に正しく処理するため、Widget層の描画ロジック等には一切触れないこと。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクは `PlayCurrentTrack` のブロック処理と、スクロール入力コールバック内の符号の標準化のみを責務とする。Widget層の描画ロジック等には絶対に触れないこと。