# RES:実装計画・作業レポート Phase 22-6: プレイリストサムネイル描画の修復とINI制御化

## 1. 実装目的
プレイリストのサムネイル表示において、「アスペクト比が崩れて正方形に潰れる」「レイアウトがハードコードされている」という不具合が存在している。これを解消するため、サムネイルのサイズや位置を INI ファイルから制御できるように設定管理を拡張し、指定領域に対してアスペクト比を維持したままフィット描画されるように UI レイアウト計算と描画ロジックを改修する。

## 2. アーキテクチャ設計
### 要件1: サムネイルレイアウトのINI制御化 (ConfigManager の拡張)
- `ConfigManager.h` および `ConfigManager_Playlist.cpp` を拡張し、`[Layout_Playlist]` セクションに `PlaylistThumbSize`, `PlaylistThumbOffsetX`, `PlaylistThumbOffsetY` の3つのパラメータを追加する。これらに対する読み込み処理 (`LoadPlaylistSettings`)、書き込み処理、およびゲッターを実装する。

### 要件2: LayoutCalculator の改修 (動的レイアウトと回避ロジック)
- `LayoutCalculator.cpp` の `CalculatePlaylistItemLayout` を改修し、ConfigManager の設定値から `thumbRect` を動的に算出する。
- 算出したサムネイル領域の幅（`PlaylistThumbSize`）とオフセット（`PlaylistThumbOffsetX`）を考慮し、タイトルやアーティスト名などのテキスト描画開始 X 座標を右へシフトし、サムネイル画像との重なりを回避する計算ロジックを実装する。

### 要件3: アスペクト比を維持した描画 (Widget_Playlist_DrawItems.cpp)
- `Widget_Playlist_DrawItems.cpp` のアイテム描画ループ内において、サムネイル描画時に `thumbRect` に直接 `DrawBitmap` する既存実装を変更する。
- 取得したビットマップの元サイズ（`bitmap->GetSize()`）からアスペクト比を計算し、`thumbRect` 領域内にアスペクト比を維持したまま長辺基準で内接（または中央配置）する描画用矩形を算出してから `DrawBitmap` を行うように修正する。

## 3. 実装タスクリスト
- [x] タスク1: ConfigManager の拡張 (サムネイルレイアウトのINI制御化)
- [x] タスク2: LayoutCalculator の改修 (動的レイアウト計算とテキスト回避ロジックの実装)
- [x] タスク3: Widget_Playlist_DrawItems の改修 (アスペクト比を維持したフィット描画の実装)
- [x] タスク4: PROJECT_ARCHITECTURE.md の更新 (追加された設定パラメータなどの反映)

## 4. 詳細作業内容
### タスク1: ConfigManager の拡張
    - `ConfigManager.h`, `ConfigManager_Playlist.cpp`, `ConfigManager_DefaultIni.h` に対して、`PlaylistThumbSize` (32.0f), `PlaylistThumbOffsetX` (5.0f), `PlaylistThumbOffsetY` (5.0f) を追加実装した。
    - 各種ゲッターと `LoadPlaylistSettings` での読み込み処理、および `DEFAULT_INI_CONTENT` への初期値設定を完了した。

### タスク2: LayoutCalculator の改修
    - `CalculatePlaylistItemLayout` 内で、追加した設定パラメータを利用して `thumbRect` を動的に算出するように変更した。
    - 描画するテキスト（タイトル、アーティスト、再生時間）がサムネイルと重ならないよう、`PlaylistThumbSize` と `PlaylistThumbOffsetX` を元にシフト量 (`textShiftX`) を算出して回避するロジックを実装した。

### タスク3: Widget_Playlist_DrawItems の改修
    - `src/Widget_Playlist_DrawItems.cpp` の `DrawTrackList` メソッド内におけるサムネイル描画部分を改修した。
    - サムネイル元画像のサイズ (`bitmapSize`) を取得し、描画先矩形 (`itemLayout.thumbRect`) にアスペクト比を維持したまま長辺基準で内接するよう、スケール値とオフセット（中央配置用）を算出するロジックを実装した。
    - 算出した `destRect` を用いて `DrawBitmap` を呼び出すように変更し、画像が正方形に潰れる問題を解消した。

### タスク4: PROJECT_ARCHITECTURE.md の更新
    - `PROJECT_ARCHITECTURE.md` の `ConfigManager` の項において、新たに追加されたプレイリスト上のサムネイルレイアウト制御パラメータ (`PlaylistThumbSize`, `PlaylistThumbOffsetX`, `PlaylistThumbOffsetY`) の説明を追記した。
    - `Widget` コンポーネントの `PlaylistWidget` の項において、`Widget_Playlist_DrawItems.cpp` がアスペクト比を維持したサムネイルのフィット描画を担当している旨を追記した。
