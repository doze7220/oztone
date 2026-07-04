# Phase 9-4: リソースのキャッシュ化 実装計画書

## 1. 目的
現在の `Renderer::Render` および各 `Draw○○()` メソッド内で毎フレーム実行されてしまっている Direct2D/DirectWrite リソース（ブラシ、ジオメトリ、テキストレイアウト）の生成処理を排し、事前生成（キャッシュ化）による再利用へ移行する。
これにより、不要なリソース生成コストを削減し、`Render()` メソッドが「キャッシュ済みリソースを用いて描画APIを呼び出すだけ」の純粋関数となるアーキテクチャを確立する。

## 2. 状態更新ルール
- **キャッシュ生成の禁止（描画時）**: `Render()` や `Draw○○()` 内部で `CreateSolidColorBrush`, `CreatePathGeometry`, `CreateTextLayout` を一切呼ばないこと。
- **キャッシュ対象外**: スペクトラム、再生位置(`progress`)、音量(`volume`)、フェード値(`m_controlAlpha`)、手動スクロール量などの「毎フレーム変化する値」はキャッシュせず、描画時にブラシの `SetOpacity()` や `SetTransform()` 等を用いて動的パラメータとして適用する。
- **更新タイミング**: ウィンドウリサイズ時、DPI変更時、曲情報更新時、Initialize時などにキャッシュを再生成・更新する。

## 3. タスクリスト

- [x] **Task 1: ブラシ (Brush) のキャッシュ化**
  - `Renderer.h` に必要なブラシ（`m_bgDarkenBrush`, `m_fallbackBlackBrush`, `m_seekBarBgBrush`, `m_seekBarFgBrush`, `m_controlBrush`, `m_playlistBgBrush`, `m_playlistHighlightBrush`, `m_resizeGripBrush` など）をメンバ変数として追加する。
  - `Renderer::Initialize` 内で `CreateSolidColorBrush` を用いて一括生成する。
  - 各 `Draw○○()` 内のブラシ生成処理を削除し、動的なアルファ値は `SetOpacity()` を使用して適用する。ビルド確認を行う。
  
- [x] **Task 2: ジオメトリ (Geometry) のキャッシュ化**
  - `Renderer.h` にベクターアイコン用（再生/一時停止/次/前ボタン、スピーカーコーン形状、リサイズグリップ形状など）の `ID2D1PathGeometry` メンバ変数を追加する。
  - `Renderer::Initialize`（または専用のキャッシュ初期化メソッド）で `CreatePathGeometry` を呼び出し、各パスを構築する。
  - 描画時は `SetTransform()` を用いて平行移動等を適用し、キャッシュされたジオメトリを描画するように変更する。ビルド確認を行う。

- [x] **Task 3: TextLayoutのライフサイクル整理**
  - `Render()` および `Draw○○()` から `CreateTextLayout` を完全に排除する。
  - `Renderer.h` に `TextLayout` を保持するメンバ変数群（またはコンテナ）を追加する。
  - キャッシュ更新専用メソッド `UpdateTextLayouts()` を新設し、曲情報・フォント・ウィンドウサイズ等の変更タイミングで再生成するようにルーティングする。
  - シークバーの再生時間 (`timeString`) などの変動値レイアウトについても、`Render()` 内での生成をやめ、外部から更新されるタイミングでレイアウトを再生成する仕組みへ移行する。ビルドして文字描画やトリミングを確認する。

- [x] **Task 4: キャッシュ化の最終監査**
  - `Renderer.cpp` 全体に対して `CreateSolidColorBrush`, `CreatePathGeometry`, `CreateTextLayout` の呼び出しを検索し、`Render()` またはそのサブメソッドにこれらの動的生成処理が残っていないか目視および grep で監査する。
  - 万が一残っていた場合は、上記Taskの基準に従いキャッシュ化への移行作業を行う。
  - アニメーションや変動パラメータなどの理由で動的生成が不可避であると判断される特殊なケースが存在した場合は、ソースコード上にコメントとして理由を明記する。

---

## 4. 作業報告（Task 1完了）
**詳細作業内容:**
- `Renderer.h` に `m_bgDarkenBrush`, `m_fallbackBlackBrush`, `m_seekBarBgBrush`, `m_seekBarFgBrush`, `m_controlBrush`, `m_playlistBgBrush`, `m_playlistHighlightBrush`, `m_resizeGripBrush` の計8つの `ID2D1SolidColorBrush` メンバ変数を追加しました。
- `Renderer::Initialize` 内で `CreateSolidColorBrush` を呼び出し、上記すべてのブラシを静的にキャッシュ生成するように実装しました（黒・白などの基本色を使用）。
- 各 `Draw○○()` メソッド内に散在していた `CreateSolidColorBrush` の呼び出しを完全に削除しました。
- 動的なアルファ値（不透明度）の変化が必要な箇所（シークバー、フェード、影等）については、描画直前にキャッシュされたブラシに対して `SetOpacity()` を呼び出してパラメータを反映させるように修正し、Renderの純粋性を確保しました。
- 修正後、CMakeビルド（`build.bat`）を実行し、コンパイルエラー等なくビルドが成功することを確認しました。

---

## 5. 作業報告（Task 2完了）
**詳細作業内容:**
- `Renderer.h` に `m_playIconGeometry`, `m_prevIconGeometry`, `m_speakerIconGeometry`, `m_resizeGripGeometry` の計4つの `ID2D1PathGeometry` メンバ変数を追加しました。
- `Renderer::Initialize` 内にてこれら4つのジオメトリを `CreatePathGeometry` で生成しました。その際、原点`(0,0)`付近を基準とする正規化されたサイズ（スピーカーアイコンや再生ボタン）や、右下基準の座標（リサイズグリップ）でパスを構築しました。
- `Renderer.cpp` の各 `Draw○○()`（`DrawPlaybackControls`, `DrawVolumeControl`, `DrawResizeGrip`）から `CreatePathGeometry` の呼び出しを完全に削除しました。
- キャッシュしたジオメトリの描画処理では、`D2D1::Matrix3x2F` を用いた `Scale` と `Translation` のアフィン変換を合成し、`SetTransform()` で適用することで、元のレイアウト位置およびサイズ通りに描画されるよう修正しました。
- スピーカーアイコンの描画で生じた変換行列のスコープエラーを修正し、再度CMakeビルド（`build.bat`）を実行してビルドが成功することを確認しました。

---

## 6. 作業報告（Task 3完了）
**詳細作業内容:**
- `Renderer.h` に `m_timeTextLayout`, `m_volTextLayout`, `m_trackCountTextLayout`, `m_playlistTimeTextLayout` などのテキストレイアウトキャッシュ用メンバ変数と、再生成判定用の状態変数（`m_lastTimeString`, `m_lastVolume` 等）を追加しました。
- 状態変数が変更された時、または `Resize()` 時の強制フラグが立っている時にのみ `CreateTextLayout` を呼び出してレイアウトを再生成する `UpdateTextLayouts()` メソッドを新設しました。
- `Renderer.cpp` の `DrawSeekBar`, `DrawVolumeControl`, `DrawPlaylist` メソッド内にあった `CreateTextLayout` の動的生成コードを完全に削除し、キャッシュ済みのテキストレイアウト（`m_timeTextLayout.Get()` など）を描画するように修正しました。
- `Renderer::Render` のシグネチャから `timeString` 引数を削除し、`Application::ForceRender` 側で描画前に `m_renderer.UpdateTextLayouts()` を呼び出すようにアーキテクチャを変更しました。
- 変更後、CMakeビルド（`build.bat`）を実行し、エラーなくビルドが成功することを確認しました。これによって、毎フレームの `Render()` 内でのリソース生成がほぼ完全に排除されました。

---

## 7. 作業報告（Task 4完了）
**詳細作業内容:**
- `Renderer.cpp` に対して `CreateSolidColorBrush`, `CreatePathGeometry`, `CreateTextLayout` の全使用箇所をgrepおよび目視で徹底的に監査しました。
- 監査の結果、`CreateSolidColorBrush` および `CreatePathGeometry` はすべて `CreateResources()` などの初期化メソッド内のみに存在し、`CreateTextLayout` は今回新設した `UpdateTextLayouts()` 内にのみ存在していることを確認しました。
- `Render()` またはそのサブメソッドである `Draw○○()` 系メソッド内に、上記の動的リソース生成処理が一切残っていないことを確認完了しました。
- したがって、毎フレーム実行される描画処理からのリソース生成排除というPhase 9-4の目標は達成されました。
