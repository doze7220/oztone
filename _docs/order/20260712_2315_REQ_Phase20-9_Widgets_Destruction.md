### 作業指示書 REQ: Phase 20-9: Widgets.cpp の完全解体とプレフィックス命名分割 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはリファクタリングの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの解体・ファイル分割を実行すること。
2. 作業が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-9_Widgets_Destruction.md）を新規作成し、詳細作業内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「Widgets.cppの完全解体が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
**[背景と大目的]**
AI-IDEのコンテキスト節約および保守性の極限向上を目指し、現在複数の小型Widgetが同居している `src/Widgets.h` および `src/Widgets.cpp` を完全に解体する。
その際、ファイル名の一覧性を高めるため、すべてのファイル名に `Widget_` というプレフィックスを付与する。

*   **要件1: 各Widgetの個別ファイル化 (プレフィックス命名)**
    *   現在 `Widgets.h / .cpp` 内に残存している全てのWidgetクラスを、以下の命名規則に従ってそれぞれ個別のヘッダおよび実装ファイルへと完全に物理分割する。
        *   `AppLogoWidget` → `src/Widget_AppLogo.h / .cpp`
        *   `TrackInfoWidget` → `src/Widget_TrackInfo.h / .cpp`
        *   `NextTrackWidget` → `src/Widget_NextTrack.h / .cpp`
        *   `SeekBarWidget` → `src/Widget_SeekBar.h / .cpp`
        *   `GlobalHotkeysWidget` → `src/Widget_GlobalHotkeys.h / .cpp`
        *   `ResizeGripWidget` → `src/Widget_ResizeGrip.h / .cpp`
        *   `OsdWidget` → `src/Widget_Osd.h / .cpp`
    *   ※ファイル名は変わるが、C++のクラス名（`AppLogoWidget` 等）は変更しないこと。
*   **要件2: 依存関係の再構築**
    *   新しく作成した各実装ファイルにおいて、必要なヘッダ（`WidgetCommon.h` や `WidgetContext.h` など）を過不足なくインクルードする。
    *   呼び出し元（`Renderer.cpp` や `Application.cpp` など）のインクルードパスを、新設した `Widget_*.h` 群へと書き換える。
*   **要件3: `Widgets.h / .cpp` の物理削除とビルド設定の更新**
    *   中身が完全に空になった `src/Widgets.h` と `src/Widgets.cpp` をプロジェクトから物理削除する。
    *   `CMakeLists.txt` から `Widgets.cpp / .h` を削除し、新しく生成した `Widget_*.cpp / .h` ファイル群をすべて追加して、ビルドが通るように構成する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 本作業は純粋なファイルの物理分割であり、各Widgetの描画ロジックや計算結果、UIの見た目、アニメーション等の挙動を一切変えてはならない。

----------------------------------------------------------------------------------------
