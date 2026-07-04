# Phase 8-8: Ninja Playlist (Step 1)

## Goal Description
「忍者プレイリスト」機能を実装します。
シークバー右上の「TRACK XXX/XXX」という表示にマウスホバーすることで、ウィンドウ右端から現在のシャッフル再生順リストがスライドインするUIを構築します。すべてのレイアウトパラメータはINIファイルで管理可能とします。

## Proposed Changes

---

### ConfigManager
設定ファイル(`OZtone.ini`)にプレイリストUI関連のレイアウト設定を追加します。

#### [MODIFY] src/ConfigManager.h
- `[Layout_Playlist]` セクション用のパラメータおよびゲッターメソッドを追加します。
  - トラック数表示: `TrackCountFontFamily`, `TrackCountFontSize`, `TrackCountRightOffset`, `TrackCountBottomOffset`, `TrackCountLetterSpacing`, `TrackCountShadowOffsetX`, `TrackCountShadowOffsetY`, `TrackCountShadowOpacity`
  - プレイリストリスト項目: `PlaylistWidth`, `PlaylistItemOffsetY`, `PlaylistTitleFontSize`, `PlaylistArtistTimeFontSize`

#### [MODIFY] src/ConfigManager.cpp
- コンストラクタでの初期値設定、`LoadSettings()` でのINI読み込み処理、`SaveDefaultSettings()` での書き出し処理に、追加したパラメータを反映します。

---

### PlaylistManager
プレイリストの現在の状況（インデックス、シャッフル後のリスト）を取得する機能を追加します。

#### [MODIFY] src/PlaylistManager.h
#### [MODIFY] src/PlaylistManager.cpp
- `size_t GetCurrentIndex() const;` : 現在再生中の曲がシャッフルリスト内の何番目かを取得するメソッドを追加。
- `std::vector<std::wstring> GetShuffleList() const;` : 現在のシャッフルリスト全体のファイルパス一覧を取得するメソッドを追加。

---

### Window
プレイリスト表示トリガー領域（トラック数表示部分）のホバー判定を追加し、既存のコントロールより優先して処理します。

#### [MODIFY] src/Window.h
#### [MODIFY] src/Window.cpp
- `bool IsInPlaylistRegion(int x, int y) const;` を追加し、「TRACK XXX/XXX」表示領域の矩形判定を実装。
- `m_isPlaylistHovered` フラグを追加。
- `WindowProc` での `WM_MOUSEMOVE` 等によるホバー状態の更新処理において、`IsInPlaylistRegion` の判定を `IsInPlaybackControlRegion` よりも先に行うよう順序を調整。

---

### Renderer
「TRACK XXX/XXX」の描画、プレイリストのスライドイン・アニメーション、およびプレイリスト項目の描画を実装します。

#### [MODIFY] src/Renderer.h
- トラック数表示用、プレイリストタイトル用、プレイリスト時間・アーティスト用の `IDWriteTextFormat` を追加。
- スライドイン・アニメーション状態を保持する変数（例：`float m_playlistSlideX`）を追加。
- `Render` メソッドの引数に、プレイリストのホバー状態、現在のインデックス、総曲数、シャッフルリストのファイルパス配列等を渡せるように拡張。

#### [MODIFY] src/Renderer.cpp
- 初期化時(`Initialize`)に各テキストフォーマット(`CreateTextFormat`)を作成。
- `Render` の最後（UIの最前面）に以下を追加：
  1. トラック数表示（"TRACK XXX/XXX"）の描画（影、トリミング、文字間隔の適用）。
  2. 毎フレームの `m_playlistSlideX` の更新（`isPlaylistHovered` に応じたイージングまたは定速スライド）。
  3. スライドインしたX座標に基づき、プレイリスト背景となる半透明の板を描画。
  4. シャッフルリストの項目をループで描画。現在再生中の曲がリスト中央に来るようにオフセット計算を行い、両端の余白制限を `std::clamp` で適用する。
  5. 各項目は `DirectWrite` を用いて、1行目に曲名、2行目にアーティスト名と時間（右寄せ）として描画する。

---

### Application
各モジュールの変更をつなぎ合わせ、描画エンジンに情報を渡します。

#### [MODIFY] src/Application.h
#### [MODIFY] src/Application.cpp
- `ForceRender` と `Run` のメインループ内で、`PlaylistManager` から「現在のインデックス」「総曲数」「シャッフルリスト」を取得。
- `Window` から `isPlaylistHovered` 状態を取得。
- これらを `Renderer::Render` に渡して毎フレーム描画させる。

## Verification Plan
1. アプリをビルド・起動し、設定ファイルに `[Layout_Playlist]` セクションが作成されることを確認。
2. プレイリストに曲を追加した際、シークバー右上に「TRACK 1/5」のように現在の位置と総数が正しく表示されることを確認。
3. トラック数表示部分にマウスホバーすると、右からプレイリストの板がスライドインし、マウスを外すとスライドアウトすることを確認。
4. プレイリスト内の曲名、アーティスト名が適切に表示され、現在の再生曲が中央に来るようスクロールされることを確認。
5. 作業完了後、`_docs/logs/20260701_HHMM_Phase8-8_Playlist.md` へログを出力し、`PROJECT_ARCHITECTURE.md` を更新。
