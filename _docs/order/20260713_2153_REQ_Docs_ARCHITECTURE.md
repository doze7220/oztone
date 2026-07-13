### 作業指示書 REQ: Hotfix: アーキテクチャ資料の最終研磨（重箱の隅の完全化）
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはアーキテクチャ資料の「最終研磨（ドキュメント更新）」実行である。直ちに以下の【更新要件】に従って `PROJECT_ARCHITECTURE.md` の修正を実行すること。
2. 作業完了後、チャットにて「アーキテクチャ資料の最終研磨が完了しました。すべてのWidgetにパスが併記され、副作用排除ルールが刻み込まれました！」と報告すること。

#### 【更新要件】
*   **要件1: Widgetリストのファイルパス表記の完全統一**
    *   `PROJECT_ARCHITECTURE.md` 内の「各具象 Widget 実装」のセクションを確認する。
    *   現在パスが併記されていない `AppLogoWidget`, `TrackInfoWidget`, `NextTrackWidget`, `SeekBarWidget`, `ResizeGripWidget`, `GlobalHotkeysWidget` などの記述の横に、`(src/Widget_AppLogo.h/cpp)` のような対応するファイルパスを併記し、すべてのWidgetの表記フォーマットを完全に統一する。
*   **要件2: 描画の副作用排除ルールの明記**
    *   同ドキュメント内の `IWidget` インターフェースのルール説明部分（またはWidget開発ルールのセクション）に、以下の「絶対の掟」を明記する。
    *   > **描画の副作用排除ルール**: 各Widgetの `Draw` 内で `SetTransform` や特殊なクリッピングを使用する場合は、必ず直前に現在の状態を保存（`GetTransform` 等）し、描画完了後に必ず元の状態へ復元すること。後続のWidgetの描画座標系を破壊してはならない。

#### 【絶対遵守ルール (Constraints)】
*   **コード変更の禁止**: 本作業は純粋なドキュメント（PROJECT_ARCHITECTURE.md）の更新のみである。C++のソースコード（.h / .cpp）やCMakeLists.txtには一切触れないこと。
