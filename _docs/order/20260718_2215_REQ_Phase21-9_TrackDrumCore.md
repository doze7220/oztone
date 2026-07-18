### 作業指示書 REQ: Phase 21-9: 画像・テキストのフリップサイクル化
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260718_2213_RES_Hotfix_Phase21-9_DoubleBufferingmd.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは、プレイドラムのテキスト・画像保持を「ダブルバッファリング（フリップサイクル）方式」へ刷新するアーキテクチャ改修の「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260718_2213_RES_Hotfix_Phase21-9_DoubleBufferingmd.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「画像・テキストのフリップサイクル化がすべて完了しました。ビルド・動作確認をお願いします！」と報告すること。

#### 【実装要件】
*   **要件1: DrumSlot 構造体の新設とバケツリレー変数の完全パージ**
    *   `src/WidgetContext.h` に、曲情報をひとまとめにした構造体 `DrumSlot` を定義する。
        （保持データ: `std::wstring trackTitle`, `std::wstring trackArtist`, `std::wstring trackNumber`, `ComPtr<ID2D1Bitmap> artBitmap`）
    *   `src/Renderer.h` および `src/WidgetContext.h` から、既存のバケツリレー用退避変数（`oldTrackTitle`, `oldTrackArtist`, `oldArtBitmap`, `currentArtBitmap` など）をすべて削除する。
    *   代わりに `DrumSlot drumSlots[1];`（または std::array）と `int currentDrumSlotIndex = 0;` を追加する。
*   **要件2: フリップサイクルの実装 (Renderer / Application層)**
    *   曲が切り替わるタイミング（新しい目標スロットが決定する `SetDrumTarget` 等）で、インデックスをフリップする処理（`m_currentDrumSlotIndex = 1 - m_currentDrumSlotIndex;`）を実行する。
    *   フリップ直後、新しい「現在のスロット（`drumSlots[m_currentDrumSlotIndex]`）」に対して、最新のテキスト情報（トラックナンバー含む）と画像をセットする。
    *   非同期ロードで画像が遅れて到着した場合も、常に「現在のスロット」の `artBitmap` を更新するように配線する。
*   **要件3: Widget_TrackInfo のハイブリッド描画の適応**
    *   `src/Widget_TrackInfo.cpp` の描画ループにて、参照先をフリップサイクルに対応させる。
    *   目標スロット（`i == 0`）を描画する際は、現在のスロット（`ctx.drumSlots[ctx.currentDrumSlotIndex]`）から全情報を参照する。
    *   直前までNOWだったOLDスロット（例: 進行方向に応じた `i == 1` または `i == -1`）を描画する際は、過去のスロット（`ctx.drumSlots[1 - ctx.currentDrumSlotIndex]`）から全情報を参照し、残像として描画する。
    *   それ以外の中間スロット（大ジャンプ時）は、テキストとトラックナンバーをオンデマンドで直接取得し、画像はガラス板を描画する既存の方式を維持する。

#### 【絶対遵守ルール (Constraints)】
*   **ゼロコピー原則**: 画像やテキストを古い変数へ「コピー（代入）」するバケツリレー処理は絶対に記述しないこと。現在と過去の切り替えは、常にインデックス（`0` ⇔ `1`）のフリップのみで解決すること。
