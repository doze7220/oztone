### 作業指示書 REQ: Hotfix CD帯UIのブラッシュアップ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_CDBandUI_BrushUp.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「CD帯UIのブラッシュアップが完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`src/Widget_TrackInfo.cpp` におけるトラックナンバー（CD帯UI）の描画ロジックを改修し、よりシャープなプロダクトデザインへと洗練させる。

*   **要件1: 分母（母数）のパージ**
    *   テキストレイアウト生成時（`UpdateLayout` 等）の文字列生成処理において、`swprintf_s(buf, L"%zu/%zu", currentTrackIndex, totalTracks);` のようになっている部分を修正する。
    *   分母を完全に削除し、`swprintf_s(buf, L"%zu", currentTrackIndex);` のように現在のトラック番号のみの文字列を生成するように変更する [1]。

*   **要件2: テキストのボールド（太字）化**
    *   `CreateResources` などの初期化メソッド内でトラックナンバー用の `IDWriteTextFormat` を生成している箇所 (`CreateTextFormat`) を見つける。
    *   フォントウェイトの引数を `DWRITE_FONT_WEIGHT_NORMAL` から `DWRITE_FONT_WEIGHT_BOLD` へ変更し、太字化する。

*   **要件3: ドロップシャドウの完全OFF**
    *   `Draw` メソッド内でのトラックナンバーテキスト描画処理を改修する。
    *   現在、影付きで描画している処理（`WidgetCommon::DrawShadowedTextLayout` の使用など）をやめ、通常の `m_d2dContext->DrawTextLayout` を直接呼び出す形に書き換えて、シャドウを完全に排除したフラットな描画にする（または、シャドウ不透明度を強制的に0.0fとして渡す）。
