「なるほど、完璧な配慮です！！」
「レイヤー構造の定義（データ構造）」と「スワップ式フェード（振る舞い）」を要件として明確に分割することで、AIが設計を混同することなく、より純度の高い計画書を出力できるようになります！
そして、**「オフセットを考慮してフェード時にずれないようにする」**というご指摘、非常に重要です。 レイヤー式にしたことで、OLDレイヤーは「古い画像と古いフレーミング」、NEWレイヤーは「新しい画像と新しいフレーミング」をそれぞれ完全に独立して持つ形になるため、原理上ズレることはありません。しかし、AIにそれを明確に「それぞれ独立して適用しろ」と意識させておかないと、1つの共通フレーミング変数を使い回して台無しにする恐れがあります。
ご要望の分割と改良（フェード速度のINI適用・オフセットの独立維持）を盛り込んだ、【Phase 23-8 計画立案用REQ】の最終アップデート版を錬成いたしました！
##### 作業指示書 REQ: Phase 23-8: 背景マネージャーの純化
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反（Managerの巨大化、Rendererへの状態混入など）がないか確認して計画を純化すること。（※監査プロセスのテキスト出力は不要）
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-8_BackgroundManager.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
背景描画機構を「4層レイヤー構造」および「NEWスワップ式のクロスフェード」へと進化させ、Rendererを完全な受動態へと純化させる。

*   **要件1: INI設定の `[Background]` セクションへの昇格と拡張**
    *   `src/ConfigManager_DefaultIni.h` 内の `DEFAULT_INI_CONTENT` を修正し、`[Layout_Window]` に存在した `BgOpacity`, `BgDarkenOpacity`, `BackgroundArtMode` を新規の `[Background]` セクションへ移動させること。
    *   **同時に、クロスフェードの速度を制御するためのパラメータ（例: `CrossfadeDuration` または `CrossfadeSpeed`）を `[Background]` セクションへ新規追加すること。**
    *   `src/ConfigManager_Window.cpp` (または該当のパース実装ファイル) において、上記パラメータのパース先とセーブ先を `[Background]` セクションへ変更・追加すること。なお、既存のゲッターメソッド名（`GetBgOpacity()` 等）はそのまま維持し、過渡期の重複コードを作らないこと。

*   **要件2: `BackgroundManager` のレイヤー構造定義**
    *   `src/BackgroundManager.h` に描画指示書となる `BackgroundLayer` 構造体を定義する。
        (例: `enum Type { Image, ColorFill }; ID2D1Bitmap* bitmap; float offsetX, offsetY, scale; D2D1_COLOR_F color; float opacity;`)
    *   `BackgroundManager` は `std::vector<BackgroundLayer> GetLayers()` を提供し、**以下の順序で動的に要素を追加できるレイヤーリスト**を構築して返却すること。
        1. **下敷きレイヤー**: `BackgroundArtMode` や画像有無に応じたプレースホルダー等。
        2. **現在背景画像レイヤー (OLD)**: 常に不透明度 1.0f (または `BgOpacity` を適用) で描画。OLD画像固有のフレーミング情報を適用する。
        3. **次背景画像レイヤー (NEW)**: クロスフェード進行中のみ登場。不透明度を 0.0f から 1.0f へ徐々に上げる。NEW画像固有のフレーミング情報を適用する。
        4. **ポストエフェクトレイヤー (可変長)**: 将来的なエフェクト（ワイプ等）の追加を見据え、ここから先は複数枚のレイヤーをスタックできるようにする。今回はまず1枚目として、`BgDarkenOpacity` を用いた黒の半透明カラーフィルレイヤーを追加する。

*   **要件3: スワップ式クロスフェードの実装**
    *   **速度制御**: 要件1で追加したINI設定値（`CrossfadeSpeed` 等）を用いて、フレーム間のフェード進行速度（アルファ値の加算量）を制御すること。
    *   **オフセット・フレーミングの完全独立維持**: OLDとNEWのレイヤーは、それぞれ独立したWIC画像とフレーミング情報（X, Y, Scale）を保持・適用して描画すること。これにより、フェード中に過去の画像の位置や拡大率が新しい設定に引っ張られてズレたりリセットされたりすることを完全に防ぐ。
    *   **スワップ式ロジック**: クロスフェードの進行度（NEWレイヤーの不透明度）が 1.0f に達した瞬間、NEW が持っていたWIC画像とフレーミング情報を OLD へとスワップ（上書き格上げ）し、NEW を空にしてフェード処理を完了させること。

*   **要件4: Renderer からの背景状態およびINI参照の完全パージ**
    *   `src/Renderer.h` および `src/Renderer.cpp` から `m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale` などのフレーミング状態を完全に削除する。
    *   フレーミング変更の操作（右ドラッグ移動やホイール拡縮）の受け渡し先を、Renderer から `BackgroundManager` へと繋ぎ変えること。
    *   `src/Renderer_Draw.cpp` の `DrawBackground` メソッドを改修し、独自の描画条件分岐や INI 設定（`GetBgOpacity()`, `GetBgDarkenOpacity()`, `GetBackgroundArtMode()`）の直接呼び出しを **全削除** する。
    *   `m_backgroundManager.GetLayers()` で受け取ったレイヤー配列を `for` ループで回し、画像の描画（`CalculateBackgroundLayout` 使用）やカラーフィルの描画を順番にスタンプするだけの純粋な塗装工（ペインター）へと純化させること。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-8_BackgroundManager.md）に、詳細作業内容を記載する（タスクリストに含める）こと。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **Rendererの状態非保持** : Renderer クラスに、背景の位置や拡大率を記憶する変数を1ミリも残してはならない。また、背景に関する INI 設定値も直接読み込んではならない（すべてレイヤー情報として受け取る）。
*   **暗転（黒浮き）の禁止** : OLD画像とNEW画像の両方のアルファ値を同時に下げるような処理は絶対に書かないこと。NEW画像のアルファ値だけを上げ、完了時にスワップすること。
