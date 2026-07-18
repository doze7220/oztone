### 作業指示書 REQ: Phase 21-7 プレイリストのプロダクトデザイン化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Phase 21-7_Playlist_ProductDesign.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「プレイリストのUIブラッシュアップとCD帯の組み込みが完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
プレイリストのUIを改修し、ツールバーのレイアウト最適化（左寄せ・右寄せ）と、リストアイテム各行へのCD帯風トラックナンバー（ハードコード排除の完全データ駆動）の付与を行う。

*   **要件1: ConfigManager へのプレイリスト用 CD帯パラメータ追加**
    *   `src/ConfigManager.h`, `src/ConfigManager_Playlist.cpp`, `src/ConfigManager_DefaultIni.h` を改修する。
    *   `[Layout_Playlist]` セクションに以下の設定項目を追加し、読み書きとゲッターを実装する（変数名には `m_playlist` プレフィックスを付ける等、適切にカプセル化すること）。
        *   `TrackCountOffsetX` (float, 初期値: 0.0f)
        *   `TrackCountBoxWidth` (float, 初期値: 15.0f)
        *   `TrackCountUnderLineX` (float, 初期値: 16.0f)
        *   `TrackCountUnderLineWidth` (float, 初期値: 2.0f)
        *   `TrackCountFontSize` (float, 初期値: 15.0f)
        *   `TrackCountLetterSpacing` (float, 初期値: -1.0f)
        *   `TrackCountBoxFontColor` (文字列, 初期値: "#000000")
        *   `TrackCountBoxBaseColor` (文字列, 初期値: "#FFFFFF")
        *   `TrackCountBoxBaseOpacity` (float, 初期値: 0.4f)

*   **要件2: ツールバー下段のレイアウト改修 (`Widget_Playlist_Toolbar.cpp` / `Widget_Playlist_Resources.cpp`)**
    *   `m_toolbarTextFormat` のアライメントを `DWRITE_TEXT_ALIGNMENT_LEADING`（左寄せ）へ変更する。
    *   総曲数表示用として、右寄せ（`DWRITE_TEXT_ALIGNMENT_TRAILING`）のテキストフォーマット `m_toolbarCountTextFormat` を新規生成する。
    *   ツールバー描画部にて、左寄せで現在の説明テキストを描画し、同じY座標の右端に右寄せで総曲数（例: `120 Tracks`）を描画するよう改修する。

*   **要件3: リソース生成の追加 (`Widget_Playlist_Resources.cpp`)**
    *   `CreateResources` にて、INIから取得した色情報 (`WidgetCommon::HexToColorF` を使用) を元に、CD帯背景用の `m_trackCountBoxBrush` とテキスト用の `m_trackCountTextBrush` を生成する。
    *   同じく `CreateResources` にて、`TrackCountFontSize` などを適用し、センタリング設定（`DWRITE_TEXT_ALIGNMENT_CENTER`）を持たせた `m_trackCountTextFormat` を生成し、`SetCharacterSpacing` で `TrackCountLetterSpacing` を適用する。

*   **要件4: リストアイテムへのデータ駆動CD帯描画 (`Widget_Playlist_DrawItems.cpp`)**
    *   曲一覧のアイテム描画ループ内において、INIパラメータを用いたCD帯を描画する。
    *   ボックスのX座標は `layout.x + TrackCountOffsetX`、幅は `TrackCountBoxWidth`、高さは `layout.itemHeight` とする。
    *   背景ボックスを描画する際、現在のトラック（`i == currentTrackIndex`）の場合はブラシの色を `ctx.playingItemColor` に変更し、それ以外の場合は `TrackCountBoxBaseColor` とする（どちらの場合も不透明度は `TrackCountBoxBaseOpacity` を適用）。
    *   `Widget_TrackInfo.cpp` と同様の手法を用いてキャンバスを -90度回転（`SetTransform`）させ、トラック番号（例: `i + 1`）を描画し、`TrackCountUnderLineX` と `TrackCountUnderLineWidth` に基づいてアンダーラインを描画する。描画後は必ずトランスフォームを元に戻すこと。

#### 【絶対遵守ルール (Constraints)】
*   **レイアウトの破壊防止**: リストのタイトルやアーティスト名の描画開始X座標が既存のままでCD帯と重なる場合は、`TrackCountOffsetX + TrackCountBoxWidth` の幅を考慮して右へオフセットし、美しく並ぶように調整すること。
