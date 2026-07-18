### 作業指示書 REQ: Hotfix TrackInfoのトラックナンバー本来表示化 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_TrackInfo_OriginalTrackNo.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「TrackInfoのトラックナンバー本来表示化が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在再生中の曲情報（`TrackInfoWidget`）に表示されるCD帯風のトラックナンバーを、シャッフルモード時においても「現在のキュー順」ではなく「プレイリストに登録された本来の行番号」で表示するように改修する。

*   **要件1: Widget_TrackInfo.cpp の文字列生成ロジックの修正**
    *   `src/Widget_TrackInfo.cpp` 内の `UpdateLayout` メソッドにおいて、トラックナンバーの文字列を生成している箇所（`swprintf_s` を使用している箇所）を見つける。
    *   現在 `ctx.currentTrackIndex + 1` 等を用いて生成しているナンバーを、`ctx.shuffleIndices` を参照して本来のインデックスを取得するように修正する。
    *   配列外アクセスを防ぐため、以下のような安全なロジックを組むこと。
        ```cpp
        size_t displayNo = ctx.currentTrackIndex + 1; // フォールバック
        if (!ctx.shuffleIndices.empty() && ctx.currentTrackIndex < ctx.shuffleIndices.size()) {
            displayNo = ctx.shuffleIndices[ctx.currentTrackIndex] + 1;
        }
        swprintf_s(buf, MAX_PATH, L"%zu", displayNo);
        ```

#### 【絶対遵守ルール (Constraints)】
*   **ファイル分割の厳守**: 本修正は `Widget_TrackInfo.cpp` 内に閉じて行うこと。アーキテクチャの変更や他のWidgetへの干渉は行わないこと。

----------------------------------------------------------------------------------------

### 作業指示書 REQ: Hotfix TrackInfo本来表示化の連携漏れ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_TrackInfo_OriginalTrackNo_Fix.md`）として新規作成し、対応内容を記録すること。
3. チャットにて「連携漏れの修正が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
TrackInfoWidgetのテキストレイアウト更新時において、シャッフルインデックス配列が正しく渡されておらず、トラックナンバーの本来表示が機能していない不具合を修正する。

*   **要件1: BuildLayoutContext へのデータ代入追加**
    *   `src/Renderer_Context.cpp` を確認する。
    *   `BuildLayoutContext()` メソッド内で `WidgetContext` を構築している箇所に、`ctx.shuffleIndices = m_shuffleIndices;` の代入が漏れていれば確実に追加する。
    *   念のため、`BuildAnimationContext()` メソッド内にも同様に追加しておく。

*   **要件2: SetShuffleIndices の呼び出し順序の適正化**
    *   `src/Application_Render.cpp` を確認する。
    *   `Application::ForceRender` メソッド内において、`m_renderer.SetShuffleIndices(m_playlistManager.GetShuffleIndices());` の呼び出し位置を確認する。
    *   もしこれが `m_renderer.UpdateTextLayouts();` よりも後ろ（下）にある場合、テキスト更新時に最新のインデックスが反映されないため、**必ず `m_renderer.UpdateTextLayouts();` よりも前（先頭付近）で呼び出されるように**順序を入れ替える。
