### Phase 9-5: UI部品のWidget（クラス）化（作業指示書）

#### 【本フェーズのゴール】
Renderer は UI要素を実装するクラスではない。
Renderer は以下の3つのみを担当する。
・Widget のライフサイクル管理
・描画順序の管理
・共通リソースの管理
各 Widget が個々のUI実装を担当し、Renderer はそれらを統括するだけのファサード（Facade）となること。

---

#### 【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（_docs/logs/YYYYMMDD_HHMM_Phase9-5_Widgetization.md）として新規作成する。
2. 作業計画をタスクリストに分割し、作業レポート末尾に記載する。
3. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、ユーザーの承認を待つこと。

---

#### 【実装要件】

**1. 目的**
現在 `Renderer` クラスに集約されている各UI要素の変数と描画・更新メソッドを、独立した `Widget` クラス群としてカプセル化し、上記のゴール（ファサード化）を達成する。

**2. 対象ファイル**
*   `src/Renderer.h`, `src/Renderer.cpp`
*   `src/Widget.h` (新規作成: 基底インターフェース `IWidget`)
*   各Widgetクラスのヘッダ・実装ファイル (※ファイルが増えすぎるのを防ぐため、`src/Widgets.h` / `src/Widgets.cpp` などにまとめて記述してもよい)
*   `CMakeLists.txt`

**3. 【絶対禁止事項・依存関係のルール】**
AIによる循環依存や設計破壊を防ぐため、以下を厳守すること。
*   **Widget 同士が直接参照し合ってはならない。**
*   **Widget は Renderer や Application を保持・参照してはならない。**
*   **Widget が他の Widget の関数を呼び出すことは厳禁。**
*   **引数の制限**: `CreateResources` や `Draw` などのメソッドは `Renderer*` を受け取ってはならない。描画に必要な `ID2D1DeviceContext*` や `IDWriteFactory*` など、必要最低限のオブジェクトと描画パラメータのみを受け取ること。
*   **Rendererの状態非保持**: Renderer は Widget 間の通信を仲介するが、UI固有の状態（m_trackName, m_progress 等）を保持する責務は持たない。必要な情報は `Application` 等から受け取り、各 Widget へ受け渡すこと。

**4. ライフサイクルとインターフェース設計（IWidget）**
Direct2Dのリソース管理作法に倣い、以下のインターフェースを実装させること。
*   `CreateResources(ID2D1DeviceContext* context, ...)`
*   `ReleaseResources()`
*   `UpdateAnimation(float deltaTime, ...)`
*   `UpdateLayout(...)`
*   `Draw(ID2D1DeviceContext* context, ...)`

**5. 段階的な実装タスクリスト**
※クラス設計を根本から変える大規模なリファクタリングであるため、必ず以下の順序でUIごとに安全に移行し、各タスクの最後にビルド・動作確認を挟むこと。

*   **Task 1: 基底クラス `IWidget` の定義と Renderer 側の管理基盤構築**
    *   `src/Widget.h` に基底インターフェースを作成。
    *   `Renderer` 内にWidgetを保持するコンテナ（`std::vector<std::unique_ptr<IWidget>> m_widgets;` 等）を準備。
*   **Task 2: 基本UIウィジェット群の独立化**
    *   `AppLogoWidget`, `TrackInfoWidget`, `NextTrackWidget` を作成。
    *   関連変数と処理を移行し、ビルド・動作確認。
*   **Task 3: コントロールウィジェット群の独立化**
    *   `PlaybackControlsWidget`, `VolumeControlWidget`, `SeekBarWidget` を作成。
    *   関連変数と処理を移行し、ビルド・動作確認。
*   [x] **Task 4: 特殊UIウィジェット群の独立化**
    *   `PlaylistWidget`, `ResizeGripWidget` を作成。
    *   関連変数と処理を移行し、ビルド・動作確認。
*   **Task 5: Rendererの大掃除と依存関係の最終監査**
    *   `Renderer` 内に特定のUIに依存した変数やメソッドが残っていないか確認する。
    *   `Renderer` が Widget の具体クラスに直接依存してよいのは **生成時のみ** とする。それ以降の更新・描画処理は、必ず `IWidget` インターフェース経由でのみ行う構造（ポリモーフィズム）になっていることを確認する。


