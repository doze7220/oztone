### 作業指示書 REQ: Hotfix Task 8 : Widget層 - アフォーダンス描画
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260722_2146_RES_Hotfix_VirtualScrollPlan.md

#### 【作業手順（厳守事項）】
本プロンプトはTask 8・アフォーダンス描画（TrackInfoへのツールチップ表示）である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）を読み、今回の自分のスコープが「タスク8」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク8のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク9以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260722_2146_RES_Hotfix_VirtualScrollPlan.md）の「タスク8」のチェックボックスを完了 [x] にし、3. 対象ファイルに作業ファイルの列挙と、詳細作業内容を追記すること。（※既存のファイル内容を破壊せず、タスク8の完了報告を正しく追記すること）
5. チャットにて「アフォーダンス描画(Hotfix Task 8)が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
本タスクでは、仮想スクロール機能の存在をユーザーに伝えるため、TrackInfo領域へのホバー時にツールチップ（吹き出し）を表示するアフォーダンスを実装する。

*   **要件1: WidgetContextへのホバー状態伝達**
    *   `src/WidgetContext.h` に `bool isTrackInfoHovered = false;` を追加する。
    *   `src/Application_Render.cpp` (または `Application_Playback.cpp` 等の適切な場所) で `m_window.IsTrackInfoHovered()` を取得し、`Renderer::UpdateAnimation` と `Renderer::Render` 経由で `WidgetContext` に伝達するよう引数と配線を更新する。
*   **要件2: ツールチップ用リソースの生成とキャッシュ (Widget_TrackInfo)**
    *   `src/Widget_TrackInfo.h` にツールチップ描画用のリソース（`ID2D1PathGeometry` など）とフェード管理用変数 `float m_hoverAlpha = 0.0f;` を追加する。
    *   `Widget_TrackInfo::CreateResources` にて、Task 1で共通化されたツールチップを描画するために必要なジオメトリ（吹き出しの背景形状など）やテキストレイアウトを生成・キャッシュする処理を実装する（※テキストやアイコンのパラメータは `ConfigManager` の `[Layout_Tooltip]` や `[Layout_NowPlaying]` を参照すること）。
*   **要件3: フェードアニメーションと描画の実行 (Widget_TrackInfo)**
    *   `Widget_TrackInfo::UpdateAnimation` にて、`ctx.isTrackInfoHovered` をもとに `m_hoverAlpha` を更新する。表示時は即座に1.0f、離脱時は `config->GetHoverFadeOutSpeed()` 等を用いて滑らかに0.0fへ向かう「非対称フェード」を適用すること。
    *   `Widget_TrackInfo::Draw` にて、`m_hoverAlpha > 0.0f` の場合に `WidgetCommon::DrawMouseScrollTooltip` を呼び出し、ツールチップを描画する。
    *   表示座標は、アルバムアートやタイトルの配置を基準とし、`ConfigManager` の `TooltipOffsetX`, `TooltipOffsetY` (`[Layout_NowPlaying]`) を加算した位置とする。さらに描画ブラシの不透明度には `m_hoverAlpha` を乗算し、フワッと消える演出を実現すること。

#### 【絶対遵守ルール (Constraints)】
*   **DRY原則** : ツールチップそのものの描画ロジック（パスの構築や描画命令の詳細）は、必ず Task 1 で作成した `WidgetCommon::DrawMouseScrollTooltip` 等の共通ユーティリティを使用し、`TrackInfoWidget` 側で再実装しないこと。