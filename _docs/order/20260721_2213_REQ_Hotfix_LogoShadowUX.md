##### 作業指示書 REQ: ロゴアイコンの非ホバー透過・フェードUXおよびシャドウの完全パージ (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトはロゴアイコンの非ホバー透過・フェードUX実装、および不要なシャドウ描画の完全パージである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_LogoShadowUX.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/ConfigManager.h`, `src/ConfigManager_LogoMenu.cpp`, `src/ConfigManager_DefaultIni.h`, `src/Widget_AppLogo.h`, `src/Widget_AppLogo.cpp`
*   **タスク1: ConfigManagerの拡張 (フェード設定の追加)**
    *  `[Layout_AppLogo]` セクションに以下を追加し、INIパースおよびゲッターを実装すること。
        *  `LogoIdleOpacity` (非ホバー時の透明度。デフォルト `0.5f`)
        *  `LogoFadeSpeed` (アニメーション速度。デフォルト `5.0f`)
    *  `DEFAULT_INI_CONTENT` への追記も忘れずに行い、SSOTを維持すること。
*   **タスク2: アニメーション状態変数の追加と更新**
    *  `Widget_AppLogo.h` にフェード進行度を保持する `float m_logoHoverAlpha = 0.0f;` を追加。
    *  `Widget_AppLogo.cpp` の `UpdateAnimation` にて、`ctx.isHovered` または `ctx.isLogoMenuExpanded`（維持ディレイ中）の**どちらかが true** の場合は `m_logoHoverAlpha` を `config->GetLogoFadeSpeed() * deltaTime` で加算し、両方 false の場合は減算するフェードロジックを実装する（`0.0f` 〜 `1.0f` にクランプ）。
*   **タスク3: 描画ロジックのクロスフェード対応とシャドウの完全パージ**
    *  `Widget_AppLogo.cpp` の `Draw` 等に存在する、ロゴアイコンに対するドロップシャドウ（`m_shadowEffect` 等）の適用および描画処理を**完全に削除（パージ）**する。
    *  従来の「`if-else` による画像の切り替え描画」を廃止する。
    *  常にベースの白アイコン（`app_logo.png`）を描画し、不透明度は `LogoIdleOpacity` から `1.0f` へと `m_logoHoverAlpha` に応じて線形補間（Lerp）する。
    *  さらにその上に、カラーのホバーアイコン（`app_logo_hover.png`）を `opacity = m_logoHoverAlpha` として同じ座標へオーバーレイ描画する美しいクロスフェードを完成させる。

#### 【絶対遵守ルール (Constraints)】
*   **状態依存の厳守**: アニメーションの進行の判定には、必ず `ctx.isLogoMenuExpanded` を利用すること。これにより、背景クリックによる強制ディレイクリア時も自動的にフェードアウトする機構が担保される。