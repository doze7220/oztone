# RES:実装計画・作業レポート Phase 23-9: 背景データ層の純化とレイヤーグループ化

## 1. 実装目的
背景マネージャー移行に伴う設定の独立化と、透過機能・背景全体への不透明度（BgOpacity）適用機構を実装する。データ層の純化（ConfigManager_Backgroundへの分割）と、Rendererの純化（不透明度をRenderer側で直接読み取らずにレイヤーグループ化により適用する仕組みの導入）を達成する。

## 2. アーキテクチャ設計
### 要件1: ConfigManager_Background.cpp の分離 (データ層の純化)
- `src/ConfigManager_Window.cpp` に残存している `[Background]` セクションの設定パース処理および保存処理を、新設する `src/ConfigManager_Background.cpp` へと移行し、ファイル分割とカプセル化を行う。

### 要件2: ウィンドウ透過の維持（下敷きレイヤーのパージ）
- `BackgroundManager::GetLayers()` において、一番下に追加している「下敷き用の黒色カラーフィルレイヤー」の追加処理を削除する。これにより、OZtoneのデスクトップ透過機能を維持する。

### 要件3: レイヤーグループ機構の定義と BgOpacity の全体適用
- `BackgroundLayerType` 列挙型 に `LayerGroupBegin` と `LayerGroupEnd` を追加する。
- `BackgroundManager` のレイヤー構築ロジックにて、背景要素追加直前に `LayerGroupBegin` を挿入し（不透明度は `BgOpacity` を設定）、全要素の追加直後に `LayerGroupEnd` を挿入する。背景全体を1つのグループとしてカプセル化する。

### 要件4: Renderer のレイヤーグループ描画対応
- `Renderer` のデバイスリソース作成処理内で `ID2D1Layer` リソース (`m_backgroundLayer`) を作成する。
- `Renderer_Draw.cpp` のレイヤー描画ループにおいて、`LayerGroupBegin` が来た際は `PushLayer` を呼び出し（`layer.opacity` を適用）、`LayerGroupEnd` が来た際は `PopLayer` を呼び出す。Renderer側で直接 `BgOpacity` の設定値を読み取ることは禁止とする。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManager_Background.cpp の分離 (データ層の純化)
- [x] タスク2: ウィンドウ透過の維持とレイヤーグループ機構の定義
- [ ] タスク3: Renderer のレイヤーグループ描画対応
- [ ] タスク4: ドキュメントの更新 (PROJECT_ARCHITECTURE.md)

## 4. 詳細作業内容
### タスク1: ConfigManager_Background.cpp の分離 (データ層の純化)
**【対象ファイル】**
- `src/ConfigManager_Window.cpp` : 更新
- `src/ConfigManager_Background.cpp` : 新規作成
- `src/ConfigManager.h` : 更新 (宣言の整理等が必要な場合)
- `CMakeLists.txt` : 更新
**【作業内容】**
- `ConfigManager_Window.cpp` から `BgOpacity`, `BgDarkenOpacity`, `BackgroundArtMode`, `CrossfadeDuration` などの `[Background]` セクションパース・保存ロジックを抽出。
- `ConfigManager_Background.cpp` を作成し、抽出したロジックを配置。
- `CMakeLists.txt` に `ConfigManager_Background.cpp` を追加する。
- `D:\ozlab\oztone\_docs\logs\20260721_0026_RES_Hotfix_ConfigManager_Background.md` 参照

### タスク2: ウィンドウ透過の維持とレイヤーグループ機構の定義
**【対象ファイル】**
- `src/BackgroundManager.h` : 更新
- `src/BackgroundManager.cpp` : 更新
**【作業内容】**
- `BackgroundLayerType` に `LayerGroupBegin`, `LayerGroupEnd` を追加。
- `BackgroundManager::GetLayers()` から下敷きの黒色カラーフィルレイヤーの追加処理を削除。
- 同メソッド内で、背景描画層の追加の直前に `LayerGroupBegin`（opacity=BgOpacity設定）、直後に `LayerGroupEnd` をリストに追加。
- （完了）下敷きの黒色カラーフィルレイヤーを削除し、`LayerGroupBegin`と`LayerGroupEnd`を追加して背景要素全体をグループ化するよう実装。

### タスク3: Renderer のレイヤーグループ描画対応
**【対象ファイル】**
- `src/Renderer.h` : 更新
- `src/Renderer_Initialize.cpp` (デバイス依存リソース生成部) : 更新
- `src/Renderer_Draw.cpp` : 更新
**【作業内容】**
- `Renderer` に `ID2D1Layer* m_backgroundLayer` メンバ変数を追加し、`CreateDeviceDependentResources` 内で `CreateLayer` を用いて生成、解放処理も追加する。
- `Renderer_Draw.cpp` の背景レイヤー描画ループ内で `LayerGroupBegin` に応じて `PushLayer` を呼び出し（`layer.opacity` から不透明度取得）、`LayerGroupEnd` で `PopLayer` を呼び出す。
- （未着手）

### タスク4: ドキュメントの更新 (PROJECT_ARCHITECTURE.md)
**【対象ファイル】**
- `PROJECT_ARCHITECTURE.md` : 更新
**【作業内容】**
- ConfigManager の物理分割ファイル一覧などに影響があるため、`ConfigManager_Background.cpp` の内容を追記する。
- （未着手）
