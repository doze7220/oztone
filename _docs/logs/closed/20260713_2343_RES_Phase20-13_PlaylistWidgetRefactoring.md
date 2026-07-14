# RES:実装計画・作業レポート Phase 20-13: PlaylistWidgetの解体準備（実装整理と抽出）

## 1. 実装目的
現在700行を超えて肥大化している `src/Widget_Playlist.cpp` に対し、将来的な物理ファイル分割を見据え、既存コードをそのまま専用のプライベートメソッドへ抽出（関数化）することで、クリーンな構造へ整理する。
本タスクでは物理的なファイル分割や、ロジック変更・リファクタリング（if文の共通化など）は一切行わず、「既存コードを抽出して独立した関数へ移動させる」ことに特化し、将来そのまま別ファイルに切り出せる独立性を確保する。

## 2. アーキテクチャ設計
### 要件1: Draw() メソッドの抽出・目次化
    - 現在巨大化している `Draw` メソッド内の処理を抽出し、以下のプライベートメソッドを定義する。
        - `DrawGrip(...)`: グリップ線と矢印の描画処理を抽出。
        - `DrawBackground(...)`: 背景矩形の描画処理を抽出。
        - `DrawToolbar(...)`: ツールバー全体の描画処理を抽出。
        - `DrawPlaylistItems(...)`: プレイリスト/曲アイテム一覧の描画処理を抽出。
    - `Draw` メソッド内は、算出した `PlaylistLayout` を上記メソッド群に渡して順次呼び出すだけの、ひと目で描画手順が分かる「目次構造」へとシェイプアップする。

### 要件2: リスト描画処理の二分化（抽出のみ）
    - `DrawPlaylistItems` 内部で `ctx.isPlaylistListViewMode` によって分岐している「プレイリスト一覧」と「曲一覧」の描画ループ部分のコードをそのまま抽出し、それぞれ以下の独立したメソッドへ分離する。
        - `DrawPlaylistList(...)`: プレイリスト一覧モードの描画処理
        - `DrawTrackList(...)`: 曲一覧モードの描画処理

### 要件3: ツールバーの細分化（抽出のみ）
    - `DrawToolbar` 内部の描画処理について、さらに以下のメソッドとしてコードをそのまま抽出し、細分化する。
        - `BuildToolbarText(...)`: ツールバー中央のテキスト構築ロジック（描画文字列の決定処理）
        - `DrawPinButton(...)`: ピン留めボタンの描画処理

### 要件4: 共通処理とデータ構造の設計案（今回は実装しない）
    - **色計算ユーティリティの分離**: `ParseHexColor` ラムダ式や、ホバー時のアルファブレンド計算 (`GetBlendedTextColor`) は将来的に描画クラスから分離し、別ユーティリティクラス等へ移行可能な設計とする。
    - **データ構築と描画の分離**: 各アイテム描画ループ内で個別に構築している表示文字列や位置情報 (`title`, `artist`, `timeStr`, `isPlaying`, `isFocused`) については、描画フェーズに入る前に `TrackVisualData` のような構造体リストとして事前生成し、描画メソッド（`DrawTrackList`等）には生成済みの表示用データを渡すだけの疎結合な設計へと段階的に移行していく方針とする。今回はこの設計案をドキュメントに残すに留める。

## 3. 実装タスクリスト
[x] タスク1: ヘッダ定義の更新 (`Widget_Playlist.h`)
    - 抽出する各種プライベートメソッドの宣言を追加する。
[x] タスク2: `Draw()` 内からの `DrawGrip()` の抽出と実装
    - 依存関係（引数）を整理しコードを移動。
[x] タスク3: `Draw()` 内からの `DrawBackground()` の抽出と実装
    - 依存関係（引数）を整理しコードを移動。
[x] タスク4: `Draw()` 内からの `DrawToolbar()` および付随する細分化メソッド（`DrawPinButton`, `BuildToolbarText`）の抽出と実装
    - 依存関係（引数）を整理しコードを移動。
[x] タスク5: `Draw()` 内からの `DrawPlaylistItems()`, `DrawPlaylistList()`, `DrawTrackList()` の抽出と実装
    - 依存関係（引数）を整理しコードを移動。
[x] タスク6: `Draw()` 本体を目次構造へと整理
    - 各抽出したメソッドを順に呼び出す形へと整える。

## 4. 詳細作業内容
### タスク1: ヘッダ定義の更新 (`Widget_Playlist.h`)
    - 完了: `PlaylistLayout` の前方宣言を追加。
    - 完了: 抽出予定の8つのプライベートメソッド (`DrawGrip`, `DrawBackground`, `DrawToolbar`, `DrawPlaylistItems`, `DrawPlaylistList`, `DrawTrackList`, `BuildToolbarText`, `DrawPinButton`) のプロトタイプ宣言を追加。
    - 完了: 将来のファイル分割を見据え、引数として `ID2D1DeviceContext*`, `WidgetContext`, `ConfigManager`, `PlaylistLayout` を受け取るように設計し依存関係を整理した。
### タスク2: `Draw()` 内からの `DrawGrip()` の抽出と実装
    - 完了: `PlaylistWidget::Draw` 内のグリップ線と矢印アイコンの描画処理を抽出し、`DrawGrip(...)` メソッドとして新たに実装した。
    - 完了: `Draw()` 内の抽出元に `DrawGrip` の呼び出しを記述した。
### タスク3: `Draw()` 内からの `DrawBackground()` の抽出と実装
    - 完了: `Draw()` 内の背景矩形の描画処理 (`FillRectangle`) を抽出し、`DrawBackground(...)` メソッドとして実装した。
    - 完了: 抽出後の引数として `ID2D1DeviceContext* context`, `const WidgetContext& ctx`, `const ConfigManager* config`, `const PlaylistLayout& layout` を渡すように設計し、メンバ変数への直接依存を避けるとともに元のメソッド内で呼び出すように変更した。
    - 完了: `PushAxisAlignedClip` はこのタスクでは抽出せず元の位置に残し、純粋な背景描画のみを分離することで、直後に描画されるツールバーがクリッピング領域外となり消えてしまうバグを未然に防いだ。
### タスク4: `Draw()` 内からの `DrawToolbar()` および付随する細分化メソッドの抽出と実装
    - 完了: `Draw()` 内のツールバー描画処理を抽出し、`DrawToolbar(...)` メソッドとして実装した。
    - 完了: `DrawToolbar` 内部の描画文字列の決定処理を `BuildToolbarText(...)` に抽出し、ピン留めボタンの描画処理を `DrawPinButton(...)` に抽出して細分化した。
    - 完了: 各抽出したメソッドが `ID2D1DeviceContext*`, `WidgetContext`, `ConfigManager`, `PlaylistLayout` などの必要な依存関係を引数から受け取るように設計し、コードの独立性を高めた。元の `Draw()` 内の抽出元には `DrawToolbar(...)` の呼び出しを記述した。
### タスク5: `Draw()` 内からのリスト描画系の抽出と実装
    - 完了: `PlaylistWidget::Draw` 内のアイテム描画処理を抽出し、`DrawPlaylistItems(...)` として実装した。
    - 完了: `DrawPlaylistItems` 内部での「プレイリスト一覧」と「曲一覧」の条件分岐内描画ループを、それぞれ `DrawPlaylistList(...)` および `DrawTrackList(...)` として明確に分離・抽出した。
    - 完了: 抽出に伴い、`GetBlendedTextColor` などの必要なローカルラムダを各抽出メソッド内に配置し、依存関係（`ID2D1DeviceContext*`, `WidgetContext`, `ConfigManager`, `PlaylistLayout`）を引数から受け取るように整理した。
    - 完了: 元の `Draw()` 内の抽出元には、クリッピングなどの順序を正しく保持したまま `DrawPlaylistItems(...)` の呼び出しを記述した。
### タスク6: `Draw()` 本体を目次構造へと整理
    - 完了: `src/Widget_Playlist.cpp` の `Draw()` メソッド内に、処理のブロックごとに「1. 描画用レイアウト情報の構築」「2. プレイリスト各要素の描画 (目次)」という整理用コメントを付与した。
    - 完了: 各描画メソッド (`DrawBackground`, `DrawToolbar`, `DrawPlaylistItems`) 呼び出し間の不要な空行を削除し、このWidgetがどのような順序で何を描画しているかが一目で理解できる究極に美しい目次状態へとシェイプアップした。機能・描画順序には一切変更を加えていない。

## 5. HOTFIX1
### 原因・理由: 
    - なし
### 対応: 
    - なし
