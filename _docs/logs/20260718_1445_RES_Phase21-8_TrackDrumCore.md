# RES:実装計画・作業レポート Phase 21-8: トラックドラム (Track Drum) コアエンジンの実装

## 1. 実装目的
曲の切り替え時（メディアキー等による Next / Prev 時など）に、物理的な「質量」と「慣性」を持ったドラムが回転するトランジションUI（トラックドラム）のコアエンジンを実装する。本フェーズではマウス操作等は含まず、「描画とバケツリレーの物理演算」のみにスコープを限定する。
INI設定で無効化された場合は従来通りの即時切り替えを完全に保証し、またアーキテクチャの原則に則り、背景アートとの完全同期および非同期画像ロードの責務（Application層への委譲）を厳格に守る設計とする。

## 2. アーキテクチャ設計
### 要件1: ConfigManagerの拡張
    - INIファイルに `[TrackDrum]` セクションを新設する。
    - `EnableTrackDrum` (デフォルト: 1), `MaxDuration` (3.0), `MinSpeed`/`MaxSpeed` (10.0/60.0), `Acceleration`/`Deceleration` (20.0/15.0) の設定管理を追加し、デフォルト値を定義する。

### 要件2: トラックドラムの描画とバケツリレー管理
    - `Widget_TrackInfo` 内に、現在位置・速度・目標インデックス・アニメーション進行度などの状態をもたせ、「消えゆく曲（OLD）」と「現れる曲（NOW）」の2つの情報セットを用いてバケツリレー形式で状態を更新する。
    - 速度は `Acceleration` / `Deceleration` を用いて計算し、S字カーブ等で制御する。
    - 目標までの距離が遠い場合は、`MaxDuration` 制約に基づき間のインデックスを間引き（スキップ）する。
    - 高速移動中（着地前）は正規の重い画像デコードをスキップし、半透明のガラス板（`FallbackArtOpacity` の流用など）を描画する。着地した瞬間にクロスフェードで出現させる。

### 要件3: 背景アートとの完全同期
    - トラックドラムの回転アニメーション（バケツリレーおよびクロスフェード）を、左下の `TrackInfoWidget` だけでなく背景アート（`DrawBackground`）とも完全に同期させる。
    - 背景描画の責務を `Widget_TrackInfo` に統合・吸収するか、あるいは `Renderer` のコンテキスト（`WidgetContext` 等）でトラックドラムの進行度合い（現在のアニメーション状態・ブレンド率・OLD/NOWのアート情報）を一括管理し、Renderer側で背景を同期描画する設計に修正する。

### 要件4: 非同期画像ロードの責務厳守
    - `Widget_TrackInfo` などのUIコンポーネント内で独自にスレッド（`std::thread`, `std::async`）を立てて画像ファイルI/Oを行うことは絶対に行わない。
    - 画像のロード処理は `Application` 層の先読み機構、または `TrackDatabase` / `TrackAnalyzer` へと委譲する。
    - Widget側は「Application層から画像が渡されるまでガラス板を描画して待つ」という受動的な設計とする。画像のロード指示やデータ連携が必要な場合は `Application_*.cpp` 側でハンドリングする機構を追加・調整する。

### 要件5: シャッフル時および既存操作の安全設計
    - 静止時にシャッフル状態が切り替わった場合は表示（ドラム）を動かさず、裏のリストのみ更新する。
    - アニメーション中にシャッフル状態が変更された場合は、OLD/NOWを現在のアニメーションのまま一旦着地させ、完了後に新リストの情報を引き込む安全な状態遷移を行う。
    - `EnableTrackDrum == 0` の場合、全アニメーションロジックをバイパスし、即座に次の曲を描画する従来の挙動を完全に維持する。

## 3. 実装タスクリスト
[x] タスク1: ConfigManagerの拡張
    - `ConfigManager.h`, `ConfigManager.cpp`, `ConfigManager_DefaultIni.h` に `[TrackDrum]` セクションと関連パラメータ群を追加する。
[x] タスク2: Renderer と Widget_TrackInfo への状態変数・物理演算の追加 (同期管理設計)
    - `Widget_TrackInfo.h/cpp` または `Renderer_Context.cpp` に、ドラムの進行状態、速度、OLD/NOWのアート・テキスト情報を保持する機構を実装する。
    - バケツリレー方式での情報更新と、`EnableTrackDrum` に従うフォールバックロジック（即時切り替え）を実装する。
[x] タスク3: 遠距離ジャンプのスキップ処理の実装
    - `MaxDuration` に基づいて、遠距離移動時に中間のインデックスを間引くスキップ処理ロジックを実装する。
[x] タスク4: 非同期ロードのApplication層への委譲とフォールバック描画の実装
    - `Application_Playback.cpp` (または関連ファイル) にて、トラックドラムの目標着地（または先読み）に合わせて画像のロードを指示・通知する処理を追加/連携させる。
    - `Widget_TrackInfo` および背景描画において、画像未ロード時や高速移動中は半透明のガラス板を描画し、着地時に画像が利用可能になり次第クロスフェードさせる描画処理を実装する。
    - UIコンポーネント内からの一切の独自スレッド起動を排除する。
[x] タスク5: 背景アートとの完全同期描画の実装
    - `Renderer_Draw.cpp` などの背景描画処理にて、トラックドラムのブレンド率・OLD/NOWアート情報に完全同期して背景をフェード・スライドさせる処理を実装する（または描画責務を再編する）。
[ ] タスク6: シャッフル状態変更時の安全な状態遷移の実装
    - 静止中・アニメーション中のシャッフル状態変更イベントを適切にハンドリングし、表示の破綻を防ぐロジックを追加する。
[ ] タスク7: PROJECT_ARCHITECTURE.md の更新
    - アーキテクチャ図や説明におけるトラックドラム機能に関する概要、および背景アートの同期や非同期ロードの責務（Application層との連携）について追記する。

## 4. 詳細作業内容
### タスク1: ConfigManagerの拡張
    - `ConfigManager_DefaultIni.h` に `[TrackDrum]` セクションのデフォルト設定を追加。
    - `ConfigManager.h` に関連パラメータのゲッターとメンバ変数を追加。
    - `ConfigManager_Playback.cpp` の `LoadPlaybackSettings` メソッド内に `[TrackDrum]` 設定の読み込み処理を実装。
    - 既存の `Widget_TrackInfo` や `Renderer` には一切変更を加えずにスコープを遵守。
### タスク2: Renderer と Widget_TrackInfo への状態変数・物理演算の追加 (同期管理設計)
    - `Renderer.h` に `m_oldTrackTitle`, `m_oldTrackArtist`, `m_oldArtBitmap`, `m_isDrumAnimating`, `m_drumPosition`, `m_drumVelocity`, `m_drumTargetIndex` の状態変数を追加。
    - `WidgetContext.h` にも同様にアニメーション状態やOLD情報を伝播させるための変数を追加し、`Renderer_Context.cpp` の各種ビルド関数で引き渡すよう修正。
    - `Renderer::SetTrackInfo` にて、新しい曲がセットされた際に現在の情報をOLDに退避し、`m_isDrumAnimating` を有効にするバケツリレー処理を実装（初回起動時は除く）。
    - `Renderer::UpdateAnimation` にて、`ConfigManager` の `TrackDrumAcceleration`, `TrackDrumDeceleration`, `TrackDrumMaxSpeed` を用いた速度と位置の更新（物理演算ロジック）を実装。
    - `EnableTrackDrum` が 0（無効）の場合はアニメーション状態を強制解除し、即時にNOWのインデックスへ位置をスナップするフォールバックロジックを実装。
### タスク3: 遠距離ジャンプのスキップ処理の実装
    - `Renderer::UpdateAnimation` にて、目標となる `m_drumTargetIndex` と現在の `m_drumPosition` の距離（スロット数）を算出する処理を追加。
    - `ConfigManager` の `TrackDrumMaxSpeed` と `TrackDrumMaxDuration` を掛け合わせ、「物理的に表示可能な最大スロット数」を計算。
    - 算出した距離が最大スロット数を超える場合、目標座標から最大スロット数分離れた位置に `m_drumPosition` をワープさせることで仮想的に中間をスキップし、必ず `MaxDuration` 秒以内にアニメーションが完了する補正ロジックを実装。
### タスク4: 非同期ロードのApplication層への委譲とフォールバック描画の実装
    - `Application.h`/`Application_Playback.cpp` に `LoadCurrentTrackArtAsync` を実装し、非同期で画像をロードする機構を追加。
    - `Application_Render.cpp` の `ForceRender` 内でトラックドラムの着地（`m_isDrumAnimating` が `true` から `false` に変わった瞬間）を検知し、`LoadCurrentTrackArtAsync` をトリガーするように修正。
    - `Widget_TrackInfo.cpp` にて、アニメーション中はガラス板を描画し、画像がロードされたらクロスフェードするロジックを実装。
    - `m_drumStartIndex` の管理を `Renderer` および `WidgetContext` に追加し、OLDとNOWのオフセット座標を正確に計算するロジックを実装。
### タスク5: 背景アートとの完全同期描画の実装
    - `Renderer_Draw.cpp` の `DrawBackground` を改修し、トラックドラムのアニメーション中 (`m_isDrumAnimating` が true) に、ドラムの進行度に基づいて OLD 画像と NOW 画像の不透明度を計算し、クロスフェード描画を行うよう実装した。
    - NOW画像が未ロード (`nullptr`) の場合は、一瞬だけプレースホルダーが表示されてチラつくのを防ぐため、プレースホルダーではなく「OLD画像を維持して徐々に黒へ暗転フェードアウトさせる」処理にフォールバックさせ、視覚的破綻を防止した。
### タスク6: シャッフル状態変更時の安全な状態遷移の実装
    - 未着手
### タスク7: PROJECT_ARCHITECTURE.md の更新
    - 未着手

### Hotfix (Phase 21-8: トラックドラム描画の完全ブラッシュアップ)
    - **要件1 (クリッピング)**: `LayoutCalculator::CalculateTrackInfoLayout` で算出した `clipRect` を用いて、`Widget_TrackInfo::Draw` 全体を `PushAxisAlignedClip` / `PopAxisAlignedClip` でマスクし、ドラムの上下はみ出しを防止。
    - **要件2 (CD帯の追従とバケツリレー)**: `Renderer`, `WidgetContext`, `TrackInfoWidget` に `m_oldTrackIndex` を追加。NOW/OLDの両方のCD帯(トラックナンバー)テキストレイアウトを構築し、アルバムアート・テキストと一緒にドラムのオフセット座標系 (`drawDrumItem` 内) に合わせてスライド・回転するよう処理を移行。
    - **要件3 & 要件4 (ガラス板と先読み画像の維持)**: `Widget_TrackInfo.cpp` の描画ロジックをリファクタリング。アニメーション中であっても画像が利用可能な場合はガラス板に差し替えずに画像を描画し、利用不可の場合のみガラス板を描画するように修正。クロスフェードは静止時の遅延ロード完了時のみ適用するよう整理。

### Hotfix 2 (Phase 21-8: 仮想スロット描画への完全改修)
    - **要件1 (クリッピング領域の厳格化)**: `Widget_TrackInfo.cpp` にて、`PushAxisAlignedClip` に渡すクリッピング矩形の高さを、テキスト領域を含まない「アルバムアートの高さ」に限定するよう修正し、ドラムの上下はみ出しを完全に切り落とした。
    - **要件2 (仮想スロットループ描画の導入)**: `m_drumPosition` を基準に画面内に表示されるインデックスをループして描画するロジックに変更。OLD/NOW以外の中間スロットについては、一時的な `IDWriteTextLayout` を生成してCD帯(本来のトラックナンバー)とガラス板のみを描画して流すように修正した。
    - **要件3 (フォールバック判定の純化)**: アニメーション中かどうかに関わらず、「画像データが `nullptr` かどうか」のみでガラス板か正規画像かを判定するように純化。これにより、OLDが突然消えたり先読みされた画像がスクロール完了まで表示されないバグを解消した。

### Hotfix 2-2 (Phase 21-8: ドラムスクロール距離の固定化)
    - **要件1 (スクロール単位の画像サイズ依存排除)**: `Widget_TrackInfo.cpp` にて、クリッピング矩形およびスロット描画時のオフセット計算の基準を、対象画像の比率に依存する `layout.artDestRect` ではなく、常に一定の枠サイズを持つ `layout.fallbackArtRect` を使用するように修正。さらに各スロット描画時に、画像自身の縦横比に合わせて `itemArtDestRect` を枠内で動的に再計算することで、長方形のアートが描画された際でもスクロール位置や表示位置がズレることなく正しく中央揃えされるように修正した。

### Hotfix 2-3 (Phase 21-8: 中間スロットのテキスト情報補完)
    - **要件1 (中間スロットへのメタデータテキスト描画の追加)**: `Widget_TrackInfo.cpp` にて、OLDおよびNOW以外の中間スロットを描画する際、`WidgetContext` が保持する `shuffleMetadataList` から対象インデックスの `TrackMetadata` を参照して曲名およびアーティスト名を取得するよう修正した。取得した文字列を用いて一時的な `IDWriteTextLayout` を生成し、OLD/NOW描画時と同じフォント・マージン・ドロップシャドウのスタイルでガラス板の上にテキストを描画して流す処理を追加した。

### Hotfix 2-4 (Phase 21-8: フェードインとガラス板判定の純化)
    - **要件1 (ガラス板フォールバック判定の純化)**: `Widget_TrackInfo.cpp` にて、OLD、NOW、および中間スロットを描画する際、「アニメーション中かどうか」という判定を排除し、「画像データ（`ID2D1Bitmap*`）が `nullptr` かどうか」のみでガラス板か正規画像かを判定するように条件を純化した。これにより、隣の曲など先読みされて既に画像がメモリ上にある場合は、スクロール中であっても最初から正規のアルバムアートを描画してスライドさせることが可能となった。
    - **要件2 (クロスフェード適用条件の限定)**: `TrackInfoWidget::UpdateAnimation` にて、クロスフェードの開始条件を「直前の画像が `nullptr` であり、かつ現在の画像が提供された瞬間（画像が存在せずガラス板で待機していた状態から画像が到着した瞬間）」に限定するよう修正。既に画像が手元にありガラス板を経由せずに描画されていた場合は、着地時であってもフェードイン処理（アルファ値の0からの加算）を行わず、常に不透明度 1.0f でそのまま正規画像を描画し続ける挙動を実現した。

### Hotfix 2-5 (Phase 21-8: 先読み画像引継ぎと背景アート同期)
    - **要件1 (先読み画像の引継ぎ)**: `Application_Playback.cpp` にて、手動切り替え時に `m_isPrefetchReady` が true の場合や `m_tagManager` から新たに読み込んだ場合、`EnableTrackDrum` が有効であっても画像を `nullptr` に破棄せず、`m_renderer.SetAlbumArt()` へ適切に先読み画像を引き継ぐよう修正。これにより、NOW画像が速やかにドラムアニメーションや背景に適用されるようになった。
    - **要件2 (背景アートの同期とクロスフェード)**: `Renderer_Draw.cpp` の `DrawBackground` を改修し、トラックドラムのアニメーション中 (`m_isDrumAnimating` が true) の場合は、ドラムの進行度に基づいて OLD 画像と NOW 画像の不透明度を計算し、クロスフェード描画を行うよう実装。
    - **要件3 (NOW画像未ロード時のフォールバック)**: 同背景アート描画において、NOW画像が未ロード (`nullptr`) の場合は、デフォルトのプレースホルダーをフェードインさせるのではなく、「NOW画像を一切描画せずOLD画像を単に暗くフェードアウトさせる（黒への暗転）」処理へフォールバックさせることで、画像不在時の一瞬の視覚的破綻（プレースホルダーのチラつき）を完全に防止した。

### Hotfix 3 (Phase 21-8: OLD背景アートのフレーミング引継ぎ)
    - **要件1 (OLDフレーミング情報の退避)**: `Renderer` の `SetTrackInfo` において、トラックが変更されドラムアニメーションが開始される際に、現在のフレーミング設定 (`m_bgOffsetX`, `m_bgOffsetY`, `m_bgScale`) を OLD 用の変数 (`m_oldBgOffsetX`, `m_oldBgOffsetY`, `m_oldBgScale`) に退避して保持するよう実装。
    - **要件2 (OLD背景描画へのフレーミング適用)**: `Renderer_Draw.cpp` の `DrawBackground` において、OLD背景アートを描画する際の `LayoutCalculator::CalculateBackgroundLayout` の呼び出しに退避しておいたOLDフレーミング情報を適用。これにより、ユーザーが設定した「最高のアングル」を維持したまま、OLD背景がクロスフェードで消えていくように修正した。

### Hotfix 4 (Phase 21-8: ドラムスロットサイズとクリッピングのシャドウ考慮)
    - **要件1 (スロットサイズの再定義)**: `Widget_TrackInfo.cpp` にて、ドラムの1スロット分の物理的な高さを単なる `layout.artSize` から、「`layout.artSize` ＋ `ShadowOffsetY` ＋ `1.0f`（マージン）」に再定義し、オフセット計算の基礎値を修正した。
    - **要件2 (クリッピング領域の拡張)**: スロットサイズの再定義に合わせ、`PushAxisAlignedClip` に渡すクリッピング矩形（`clipRect.bottom`）の高さも拡張し、下側に落ちるシャドウが途切れずに完全に表示されるように修正した。
    - **要件3 (シャドウのスライド追従)**: シャドウを描画する際のY座標計算において、再定義した新しいスロット高さに基づくスライド用オフセット（`offsetY`）が適用されるよう修正。これにより、アルバムアートと一緒にシャドウも完全に画面外へスクロールアウトし、元の位置に影だけが残留するバグを解消した。

### Hotfix 5 (Phase 21-8: 初回ブランク防止とスクロール距離の最適化)
    - **要件1 (初回起動時のアニメーションスキップ)**: `Renderer` に `m_isFirstTrackLoaded` フラグを追加。`SetTrackInfo` 内で「まだ一度もトラック情報がセットされていない初期状態」を判定し、初回起動時は `m_isDrumAnimating` を `false` に設定してアニメーションをスキップし、NOWの情報を即座に表示させるよう修正した。
    - **要件2 (ループ境界での順方向スクロール補正)**: `Renderer_Update.cpp` にて、プレイリストの周回・切り替え時に `m_drumTargetIndex` が変化した際の初期位置（`m_drumPosition`）を補正するロジックを追加。「最後の曲から最初の曲へ」移る際は移動距離が `+1.0f`、「最初の曲から最後の曲へ」移る際は `-1.0f` となるよう `m_drumPosition` を目標インデックスの前後へ強制配置し、逆スクロールを防ぎつつ自然な1スロット分のスクロールが実行されるよう最適化した。

### Hotfix 5-1 (Phase 21-8: 初回起動時のドラム位置完全同期)
    - **要件1 (初回起動時のドラム位置の完全強制同期)**: `Renderer::SetTrackInfo` にて初回（`m_isFirstTrackLoaded` == false）と判定された際、新しい目標インデックスとして `m_lastCurrentTrackIndex` が有効な値の場合のみ `m_drumTargetIndex` に代入し、直後に `m_drumPosition = static_cast<double>(m_drumTargetIndex);` を実行するよう修正。不正なインデックス（-1, -2）の代入による位置のオーバーフローを防止し、初期座標の同期を確立した。
    - **要件2 (表示スロットの優先順位修正)**: `Widget_TrackInfo.cpp` にて、`ctx.drumStartIndex` と `ctx.drumTargetIndex` が一致している場合（初回起動時など）に、誤って空の `old` 情報が描画されてブランクになる不具合を修正。`trackIndex == ctx.drumTargetIndex` の判定順位を `ctx.drumStartIndex` より上に引き上げることで、同一インデックス時は常に最新のターゲット情報が優先描画される正常なロジックへと改修した。

### 2026-07-18 Phase 21-8 Hotfix 5-2: 再生ストリームフラグとOLDスナップショット化
- `Application` に対して `m_isContinuousStream` と `m_streamBreakDirection` フラグを導入し、連続再生と非連続のトラックスキップを区別するロジックを実装。
- `Renderer` 側に `ResetDrumPosition` を実装し、非連続スキップ時にドラム位置を正確な開始スロット（仮想インデックス）へリセットする機能を導入。
- `Widget_TrackInfo.cpp` 内におけるOLDメタデータの描画を、従来の `ctx.shuffleMetadataList` 参照から、完全スナップショット化された文字列のみで描画されるように純化。これにより、プレイリスト切り替え時の逆スクロールバグと、OLDメタデータの消失バグを解決。
- 古いループ境界補正ロジックを完全に削除。

### Hotfix 5-3 (Phase 21-8: バケツリレーの空打ち防止とスナップショット完全化)
    - **要件1 (トラックナンバーの完全スナップショット退避)**: `Renderer::SetTrackInfo` にて、OLDのCD帯（トラックナンバー）を文字列として `m_oldTrackNoString` にスナップショット退避するロジックを実装。`Widget_TrackInfo.cpp` では、OLDのトラックナンバー描画時に新しいプレイリストのインデックス配列(`shuffleIndices`)を参照せず、この退避された文字列を直接描画するように純化し、切り替え時のインデックスすり替えバグを解消した。
    - **要件2 (UIリセット時のバケツリレー保護)**: `Renderer::SetTrackInfo` にて、曲名が空または「NO TRACK」のような無効な状態のときにはOLDへの退避（スナップショット）を行わないガード処理を追加。これにより、プレイリスト切り替え時の「一時的なUIクリア処理」による「NO TRACK」や「ガラス板」がOLDとしてドラムに描画されてしまう空打ち現象（玉突き事故）を完全に防止した。

### Hotfix 5-4 (Phase 21-8: 仮想OLDインデックスの同期)
    - **要件1 (仮想OLDインデックスの同期・偽装)**: `Renderer::ResetDrumPosition` にて、ストリーム切断に伴う `m_drumPosition` の強制リセット処理時に、同時に `m_oldTrackIndex` にも同じターゲットの前後（`+/- 1`）の仮想インデックスを代入するよう修正。これにより、ワープ直後の手前（奥）のスロットが「中間スロット」ではなく確実に「OLDスロット」として判定され、退避済みのスナップショットが正しく表示される状態を確立した。

### Hotfix 5-5 (Phase 21-8: インデックス判定の純化と型安全の確保)
    - **要件1 (バケツリレー保護のマジックストリング脱却)**: `Renderer::SetTrackInfo` に `currentTrackIndex` 引数を追加し、無効な値（`-1`）の場合はOLDへのスナップショット退避を行わないガード条件へと純化。これに伴い、`Application` 層においてプレイリスト切り替えなどのUIクリア目的で「NO TRACK」をセットする際も `-1` を渡すように統一し、文字列判定による意図しない退避を防ぐロジックに変更した。
    - **要件2 (仮想インデックスの符号付き型安全比較)**: `Widget_TrackInfo.cpp` の仮想スロット描画ループ内において、ループ変数（`int`）と `ctx.oldTrackIndex`（`size_t`）の比較を行う際、双方を明示的に `int` 型にキャストして比較するように改修。`Renderer::ResetDrumPosition` の仮想インデックス代入時などで発生した `size_t` のアンダーフロー値が、描画時に正しく負のインデックス（例: `-1`）として安全に評価・比較される状態を保証した。
### Hotfix 5-6 (Phase 21-8: OLDフラグの導入とマジックナンバー完全排除)
    - **要件1 (別リストOLDフラグの導入)**: Renderer と WidgetContext に、退避したOLDが連続ストリームか別リストかを識別するフラグ m_oldIsCrossPlaylist および m_streamBreakDirection を導入。ストリーム切断時の ResetDrumPosition で「仮想インデックスの偽装代入」を完全に廃止し、本来のインデックスを維持するように修正。
    - **要件2 (相対位置によるOLD描画判定)**: Widget_TrackInfo.cpp の描画ループにおいて、別リストからの移行時 (ctx.oldIsCrossPlaylist == true) にはインデックス番号の直接比較を行わず、現在位置と streamBreakDirection から算出した相対位置の枠を無条件でOLDスロットとして扱う判定ロジックへ純化。これによりインデックス番号のマジックナンバーによるハック判定を全廃した。
    - **要件3 (UIリセットフラグによる空打ちガード)**: Renderer::SetTrackInfo の引数から -1 によるインデックス判定を廃止し、明示的なフラグ ool isResetUI を導入。Application 側でUIをクリアする目的にのみ isResetUI = true を渡すことで、通常の曲切り替え（同一リスト内ループ再生など）では確実にスナップショット退避が行われ、OLD消失バグを根本解決した。
