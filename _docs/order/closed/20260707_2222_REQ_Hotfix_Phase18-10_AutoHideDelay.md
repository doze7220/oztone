### 作業指示書 REQ: Phase 18-10 Hotfix 3: 離脱ディレイ中のUIホバー判定抜け完全修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\closed\20260707_0951_RES_Phase18-10_AutoHideDelay.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（`D:\ozlab\oztone\_docs\RES(Hotfix)_template.md`）を元に、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHSS_RES_Hotfix_Phase18-10_AutoHideDelay.md`）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
*   **不具合の真の原因** : 
    Phase 18-10で「離脱ディレイ」を実装した際、タイマーによる展開維持ロジックを描画側（WidgetのUpdateAnimation等）にのみ追加したため、入力判定側（Windowクラス）との間に乖離が生じている。
    マウスがホバー領域から外れた瞬間、Window側のフラグは即座に `false` になり、それに伴い拡張されていたホバー判定領域（例：ロゴメニューの展開幅）がシュリンク（縮小）してしまう。その結果、ディレイによって描画で維持されているメニュー上にマウスを戻しても、Window側が領域外と判定して空振りしてしまい、ホバー状態に復帰できずアイコンが反応しなくなっていた。
*   **修正対象** : ロゴ拡張メニュー、および同様のディレイを持つプレイリストのホバー判定関連（`src/Window.h/cpp`, `src/Application.cpp`, `src/Renderer.h/cpp` 等）
*   **修正内容 (アーキテクチャの同期)** :
    描画側の展開状態（現在のアニメーション進行度 progress が 0 より大きいこと等）を、Window クラスの当たり判定（`IsInLogoMenuRegion`, `IsInPlaylistRegion` 等）の領域拡張条件にフィードバック・同期させるロジックを構築する。
    1. Renderer (または各 Widget) から現在のロゴメニューおよびプレイリストのアニメーション展開度（progress）を取得するゲッターを追加する。
    2. Application の毎フレーム処理（`ForceRender` 等）で、取得した progress を Window クラスへ伝達する（例：`m_window.SetLogoMenuProgress(...)`, `SetPlaylistProgress(...)` 等のセッターを追加）。
    3. Window クラスのヒットテスト判定（`IsInLogoMenuRegion`, `IsInPlaylistRegion` 等）において、自身のホバーフラグだけでなく、伝達された展開状態（progress > 0.0f 等）を評価し、UIが少しでも展開・ディレイ中であれば判定領域を拡張したまま維持するように修正する。
