##### 作業指示書 REQ: ロゴアイコンの非ホバー透過とフェード切り替えUX (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトはロゴアイコンの非ホバー透過とフェード切り替えUXの実装である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_LogoFadeUX.md）として新規作成すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/ConfigManager.h`, `src/ConfigManager_LogoMenu.cpp`, `src/ConfigManager_DefaultIni.h`, `src/Widget_AppLogo.h`, `src/Widget_AppLogo.cpp`
*   **タスク1: ConfigManagerの拡張**
    *  `[Layout_AppLogo]` （または既存の適切なロゴセクション）に `LogoIdleOpacity` (非ホバー時の透明度。デフォルト例 `0.5f`) と `LogoFadeSpeed` (フェードアニメーション速度。デフォルト例 `5.0f`) を追加。
    *  INIからのパースおよび `DEFAULT_INI_CONTENT` への追記（SSOT化の維持）を行う。
*   **タスク2: アニメーション状態変数の追加と更新**
    *  `Widget_AppLogo.h` にフェード進行度を保持する `float m_logoHoverAlpha = 0.0f;` を追加する。
    *  `Widget_AppLogo.cpp` の `UpdateAnimation` にて、`ctx.isHovered`（ロゴアイコン自体へのホバー）または `ctx.isLogoMenuExpanded`（ロゴ拡張メニュー維持ディレイ中）の**どちらかが true** の場合は `m_logoHoverAlpha` を `config->GetLogoFadeSpeed() * deltaTime` で加算し、両方 false の場合は減算するフェードロジックを実装する（`0.0f` 〜 `1.0f` にクランプ）。
*   **タスク3: 描画ロジックのクロスフェード対応**
    *  `Widget_AppLogo.cpp` の `Draw` にて、従来の「`if-else` による画像のパキッとした切り替え描画」を完全に廃止する。
    *  常にベースとなる白アイコン（`app_logo.png`）を描画し、その不透明度は `LogoIdleOpacity` から `1.0f` へと `m_logoHoverAlpha` に応じて線形補間（Lerp）する。
    *  さらにその上に、カラーのホバーアイコン（`app_logo_hover.png`）を `opacity = m_logoHoverAlpha` として同じ座標へオーバーレイ描画（`DrawBitmap`）する。
    *  これにより、非ホバー時の半透明白アイコンから、ホバー時の完全な色付きアイコンへ滑らかにクロスフェードする美しいUXが完成する。

#### 【絶対遵守ルール (Constraints)】
*   **状態依存の厳守**: アニメーションの進行（フェードイン・フェードアウト）の判定には、必ず `ctx.isLogoMenuExpanded` を利用すること。これにより、背景クリックによる強制ディレイクリア時も即座に `UpdateAnimation` の減算ルートに入り、自動的にフェードアウトする機構が担保される。
