### 作業指示書 REQ: Phase 20-7: Window.cpp 分割に向けた事前調査と整理・解体計画
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. まず `src/Window.cpp` 全体をスキャンし、物理分割（別ファイル化）を行う前に「関数の抽出」や「重複の排除」といった事前の整理（リファクタリング）が必要かどうかを調査すること。
3. 調査結果を踏まえ、以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画、兼作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-7_WindowSplitPlan.md）として新規作成すること。
4. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

---

#### 【実装要件】
現在1200行を超える `Window.cpp` を、AI-IDEのコンテキスト節約のため、実装のみを責務別の .cpp ファイルへ物理分割する。
分割後は **「ある機能を修正する際に、対象となる.cppだけを読めば実装を理解できる」** ことを目標とする。

*   **要件1: WindowProcの薄いディスパッチャ化と抽出**
    *   現在の `WindowProc` はほぼ全ての機能に触れており、そのまま分割すると `Window_Proc.cpp` に巨大なロジックが残ってしまう可能性が高い。
    *   これを防ぐため、`WindowProc` 自体は薄いディスパッチャ（司令塔）として維持し、各メッセージ処理は個別のメンバ関数（`HandleXXXX()`）へ抽出する整理計画を立てること。
    *   抽出候補を可能な限り洗い出し、責務ごとに整理すること（例: `HandleMouseMove()`, `HandleMouseLeave()`, `HandleMouseWheel()`, `HandleTrayIcon()`, `HandleCommand()`, `HandleDestroy()`, `HandleLogoMenuClick()`, `HandlePlaybackClick()`, `HandlePlaylistClick()` など）。
*   **要件2: コンテキスト単位での解体計画**
    *   抽出されたメンバ関数群を、以下の責務別ファイル（例）へ物理分割するロードマップを策定する。
        *   `Window_Proc.cpp` (薄いメッセージディスパッチの基盤)
        *   `Window_Mouse.cpp` (各UIの座標判定とマウスイベント処理)
        *   `Window_TrayMenu.cpp` (トレイメニュー生成とコマンド処理)
        *   `Window_DropTarget.cpp` (ファイルドロップ処理)
        *   `Window_Initialize.cpp` (ウィンドウ生成・初期化)
        *   `Window_System.cpp` (フック・グローバルホットキー等のシステム処理)
*   **要件3: ユーティリティファイルの作成禁止**
    *   分割単位は必ず「責務単位」とし、`WindowUtility.cpp` や `WindowCommon.cpp` のような機能横断的なファイルは **絶対に新設しない** こと。

#### 【絶対遵守ルール (Constraints)】
*   **Window.h の変更制約**: `Window.h` への「新しい `private` メンバ関数の宣言追加」は許可する。ただし、データメンバ（変数）の追加や、公開インターフェース（`public`）の変更は絶対に禁止する。
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なリファクタリング（整理と分割）の計画のみを立案すること。

-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 1 : Window.h の更新と WindowProc の整理
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cpp分割に向けた事前整理（関数の抽出）の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク1」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク1（Window.h の更新とWindowProcの整理）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在 `Window.cpp` の `WindowProc` にベタ書きされている巨大なメッセージ処理ロジックを個別のメンバ関数に抽出し、`WindowProc` を純粋なディスパッチャ（司令塔）へと整理する。

*   **要件1: `Window.h` への関数宣言追加**
    *   `Window.h` の `private` セクションに、抽出するメッセージ処理関数（例: `HandleMouseMove()`, `HandleMouseLeave()`, `HandleMouseWheel()`, `HandleTrayIcon()`, `HandleCommand()`, `HandleDestroy()`, `HandleLogoMenuClick()`, `HandlePlaybackClick()`, `HandlePlaylistClick()` など、可能な限りの責務）のプロトタイプ宣言を追加する。
    *   データメンバや `public` インターフェースは絶対に変更しないこと。
*   **要件2: `Window.cpp` 内での関数抽出と `WindowProc` の整理**
    *   `Window.cpp` 内で、上記で宣言した `HandleXXX()` メソッドを実装し、`WindowProc` 内の `switch` / `case` 文からロジックを完全に移動させる。
    *   `WindowProc` は、受け取ったメッセージに応じて各 `HandleXXX()` を呼び出すだけの「薄いディスパッチャ」として機能するように再構築すること。

#### 【絶対遵守ルール (Constraints)】
*   **物理分割の禁止**: 本タスクは同一ファイル内（`Window.cpp`）での整理（抽出）に留め、まだ別の `.cpp` ファイルへの切り出し（物理分割）は行わないこと（後続のタスクで実行するため）。
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なリファクタリングのみを行うこと。

-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 2 : Window_Mouse.cpp の分離
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cppからの「Window_Mouse.cpp」の物理分割の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク2」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク2（Window_Mouse.cpp の分離）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
タスク1で整理された `Window.cpp` 内のマウスイベント処理と座標判定メソッドを、新規ファイル `src/Window_Mouse.cpp` へ物理分割する。

*   **要件1: `Window_Mouse.cpp` の新規作成とコード移行**
    *   `src/Window_Mouse.cpp` を新規作成し、先頭に必要なインクルード（`#include "Window.h"` や設定、ユーティリティなど）を記述する。
    *   `Window.cpp` から以下のメソッド群（実装）を切り取り、`Window_Mouse.cpp` へ完全に移行する。
        *   座標判定系（`IsInLogoRegion`, `IsInPlaybackControlRegion`, `IsInVolumeControlRegion`, `GetPlaybackButtonAt`, `IsInPlaylistRegion`, `GetLogoMenuButtonAt` など）
        *   マウス入力ハンドラ系（`HandleMouseMove`, `HandleMouseLeave`, `HandleLButtonDown`, `HandleRButtonDown`, `HandleLButtonDblClk`, `HandleMouseWheel` など）
*   **要件2: CMakeLists.txt への追加**
    *   新設したファイルがコンパイル対象となるよう、`CMakeLists.txt` のソースリストに `src/Window_Mouse.cpp` を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なファイルの切り出し（移動）のみを行うこと。
*   **スコープの厳守**: 今回はマウス入力・座標判定に関するメソッドのみを移行すること。トレイメニューやシステム関連等の他メソッドは後続タスクで移行するため `Window.cpp` にそのまま残すこと。
*   **ヘッダの維持**: `Window.h` のクラス定義は変更しないこと。
-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 3 : Window_TrayMenu.cpp の分離
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cppからの「Window_TrayMenu.cpp」の物理分割の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク3」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク3（Window_TrayMenu.cpp の分離）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Window.cpp` に残存しているシステムトレイアイコンおよびメニュー処理関連のメソッドを、新規ファイル `src/Window_TrayMenu.cpp` へ物理分割する。

*   **要件1: `Window_TrayMenu.cpp` の新規作成とコード移行**
    *   `src/Window_TrayMenu.cpp` を新規作成し、先頭に必要なインクルード（`#include "Window.h"` や設定、ユーティリティなど）を記述する。
    *   `Window.cpp` から以下の定義・実装を切り取り、`Window_TrayMenu.cpp` へ完全に移行する。
        *   メニュー構築用の配列定義（`TRAY_MENU_ORDER` 等）
        *   システムトレイのメッセージハンドラ（`HandleTrayIcon`）
        *   メニューコマンドのハンドラ（`HandleCommand`）
        *   その他、メニュー項目に関連する処理があれば併せて移行する。
*   **要件2: CMakeLists.txt への追加**
    *   新設したファイルがコンパイル対象となるよう、`CMakeLists.txt` のソースリストに `src/Window_TrayMenu.cpp` を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なファイルの切り出し（移動）のみを行うこと。
*   **スコープの厳守**: 今回はシステムトレイ・メニュー処理に関するメソッドのみを移行すること。D&Dやシステム系処理は後続タスクで移行するため `Window.cpp` に残すこと。
*   **ヘッダの維持**: `Window.h` のクラス定義は変更しないこと。

-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 4 : Window_DropTarget.cpp の分離
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cppからの「Window_DropTarget.cpp」の物理分割の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク4」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク4（Window_DropTarget.cpp の分離）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Window.cpp` に残存している OLE Drag and Drop（ファイルドロップ）関連の実装を、新規ファイル `src/Window_DropTarget.cpp` へ物理分割する。

*   **要件1: `Window_DropTarget.cpp` の新規作成とコード移行**
    *   `src/Window_DropTarget.cpp` を新規作成し、先頭に必要なインクルード（`#include "Window.h"` や `<shellapi.h>` など）を記述する。
    *   `Window.cpp` から `DropTarget` クラスの実装部分（`QueryInterface`, `AddRef`, `Release`, `DragEnter`, `DragOver`, `DragLeave`, `Drop` など）を全て切り取り、`Window_DropTarget.cpp` へ完全に移行する。
*   **要件2: CMakeLists.txt への追加**
    *   新設したファイルがコンパイル対象となるよう、`CMakeLists.txt` のソースリストに `src/Window_DropTarget.cpp` を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なファイルの切り出し（移動）のみを行うこと。
*   **スコープの厳守**: 今回は `DropTarget` 関連のメソッドのみを移行すること。システム連携処理や初期化処理は後続タスクで移行するため `Window.cpp` に残すこと。
*   **ヘッダの維持**: `Window.h` のクラス定義は変更しないこと。

-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 5 : Window_System.cpp の分離
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cppからの「Window_System.cpp」の物理分割の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク5」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク5（Window_System.cpp の分離）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Window.cpp` に残存しているシステム連携・キーフック・プロセス間通信関連のメソッドを、新規ファイル `src/Window_System.cpp` へ物理分割する。

*   **要件1: `Window_System.cpp` の新規作成とコード移行**
    *   `src/Window_System.cpp` を新規作成し、先頭に必要なインクルード（`#include "Window.h"` 等）を記述する。
    *   `Window.cpp` から以下の定義・実装を切り取り、`Window_System.cpp` へ完全に移行する。
        *   低レベルキーボードフック処理 (`LowLevelKeyboardProc` の実装)
        *   システムメッセージのイベントハンドラ（タスク1で抽出された `WM_COPYDATA`、`WM_HOTKEY`、`WM_APP_MEDIAKEY` などに対応する `HandleXXX` メソッド群）
*   **要件2: CMakeLists.txt への追加**
    *   新設したファイルがコンパイル対象となるよう、`CMakeLists.txt` のソースリストに `src/Window_System.cpp` を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なファイルの切り出し（移動）のみを行うこと。
*   **スコープの厳守**: 今回はシステム連携・フック処理に関するメソッドのみを移行すること。ウィンドウの初期化処理（Initialize等）は次タスクで移行するため `Window.cpp` に残すこと。
*   **ヘッダの維持**: `Window.h` のクラス定義は変更しないこと。
-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 6 : Window_Initialize.cpp の分離
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cppからの「Window_Initialize.cpp」の物理分割の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク6」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク6（Window_Initialize.cpp の分離）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Window.cpp` に残存しているウィンドウ生成、初期化、破棄関連のメソッドを、新規ファイル `src/Window_Initialize.cpp` へ物理分割する。

*   **要件1: `Window_Initialize.cpp` の新規作成とコード移行**
    *   `src/Window_Initialize.cpp` を新規作成し、先頭に必要なインクルード（`#include "Window.h"` 等）を記述する。
    *   `Window.cpp` から以下の定義・実装を切り取り、`Window_Initialize.cpp` へ完全に移行する。
        *   コンストラクタ (`Window::Window()`) およびデストラクタ (`Window::~Window()`)
        *   ウィンドウクラス登録および生成・初期化処理 (`Window::Initialize()`)
        *   ウィンドウ破棄のイベントハンドラ（`Window::HandleDestroy()`）
*   **要件2: CMakeLists.txt への追加**
    *   新設したファイルがコンパイル対象となるよう、`CMakeLists.txt` のソースリストに `src/Window_Initialize.cpp` を追加する。

#### 【絶対遵守ルール (Constraints)】
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なファイルの切り出し（移動）のみを行うこと。
*   **スコープの厳守**: 今回はウィンドウの生成・初期化・破棄に関するメソッドのみを移行すること。大元のディスパッチャ（`WindowProc` 等）は最終タスクで移行するため `Window.cpp` に残すこと。
*   **ヘッダの維持**: `Window.h` のクラス定義は変更しないこと。

-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 Task 7 : Window_Proc.cpp の作成と最終クリーンアップ
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはWindow.cppの解体ショー最終タスクの「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク7」および「タスク8」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク7および8（Window.cppの完全解体）(Phase 20-7)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`Window.cpp` に残存している純粋なディスパッチャ（`WindowProc`等）を `src/Window_Proc.cpp` へ移行し、旧 `Window.cpp` を物理削除して完全な解体を完遂する。

*   **要件1: `Window_Proc.cpp` の新規作成とコード移行**
    *   `src/Window_Proc.cpp` を新規作成し、先頭に必要なインクルード（`#include "Window.h"` 等）を記述する。
    *   `Window.cpp` に残っている全ての実装（`WindowProcStatic` や `WindowProc` など）を `Window_Proc.cpp` へ完全に移行する。
*   **要件2: 旧ファイルの削除と CMakeLists.txt の更新**
    *   不要となった `src/Window.cpp` をファイルシステムから物理的に削除する。
    *   `CMakeLists.txt` のソースリストから `src/Window.cpp` を削除し、代わりに `src/Window_Proc.cpp` を追加する。
*   **要件3: 最終ビルド確認 (タスク8)**
    *   古いファイルが完全に消滅し、分割された全てのファイル群（Mouse, TrayMenu, DropTarget, System, Initialize, Proc）が正しくコンパイルされ、アプリケーションが正常に動作することを最終確認する。

#### 【絶対遵守ルール (Constraints)】
*   **ヘッダの維持**: `Window.h` のクラス定義やインターフェースは一切変更しないこと。
*   **機能変更の禁止**: 既存の挙動や機能は一切変更せず、純粋なファイルの切り出しとクリーンアップのみを行うこと。

-----------------------------------------------------------------------------
### 作業指示書 REQ: Phase 20-7 最終整理とアーキテクチャ資料の更新
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 20-7の最終整理およびドキュメント更新の「実装実行」である。直ちに以下の【実装要件】に従ってドキュメントの修正を実行すること。
2. 作業完了後、既存の作業レポート（logs\20260712_1901_RES_Phase20-7_WindowSplitPlan.md）の「タスク7」および「タスク8」のチェックボックスを完了 [x] にし、詳細作業内容を追記すること。
3. チャットにて「タスク7, 8の完了およびPROJECT_ARCHITECTURE.mdの更新が完了しました」と報告すること。

#### 【実装要件】
`Window.cpp` の完全解体に伴い、アーキテクチャの変更をプロジェクト資料へ反映する。

*   **要件1: 開発資料 (`PROJECT_ARCHITECTURE.md`) の更新**
    *   `PROJECT_ARCHITECTURE.md` の `Window クラス` の解説セクションを更新する。
    *   ApplicationやConfigManagerの解説と同様に、「AI-IDEでの開発効率とコンテキスト節約のため、クラス設計（.h）はそのままに、実装（.cpp）が責務ごとに以下の6つのファイルに物理分割されている」旨を明記し、以下のリストを追記する。
        *   `Window_Proc.cpp`: メッセージディスパッチの基盤（薄い司令塔）
        *   `Window_Mouse.cpp`: 座標判定（ヒットテスト）とマウス入力イベント処理
        *   `Window_TrayMenu.cpp`: トレイアイコンおよびコンテキストメニューの生成・コマンド処理
        *   `Window_DropTarget.cpp`: OLE Drag and Drop によるファイルドロップ処理
        *   `Window_System.cpp`: 低レベルフック、グローバルホットキー、多重起動防止などのシステム連携処理
        *   `Window_Initialize.cpp`: ウィンドウの登録・生成・初期化および破棄処理
    *   既存の各メソッド機能説明（`bool Initialize` や `LRESULT WindowProc` など）の記述は消さずに維持すること。

#### 【絶対遵守ルール (Constraints)】
*   **コード変更の禁止**: 本タスクはドキュメントおよび作業レポートの更新のみであり、C++ソースコードの変更は絶対に行わないこと。

-----------------------------------------------------------------------------
