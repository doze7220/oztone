# 実装レポート: Phase 8-8 ステップ2 プレイリストのインタラクション

## 1. 目的
プレイリストのインタラクション機能（マウスホイールによるスクロールと、クリックによる対象曲へのジャンプ再生）を実装する。

## 2. 変更予定のファイル
- `src/PlaylistManager.h` / `src/PlaylistManager.cpp`
- `src/Renderer.h` / `src/Renderer.cpp`
- `src/Window.h` / `src/Window.cpp`
- `src/Application.cpp`

## 3. 実装要件・方針

### PlaylistManager の拡張
- `void JumpToIndex(size_t index);` を追加し、`m_shuffleIndex` を指定された値に更新するよう実装する。
- 範囲外アクセスのガード処理を入れる。

### スクロール状態の管理と描画への反映
- `Renderer` クラスに `float m_playlistManualScrollY` をメンバ変数として追加する。
- プレイリストが閉じられた際（`Renderer::Render` の引数 `isPlaylistHovered` が false になった時）に `m_playlistManualScrollY` を 0.0f にリセットする。
- 描画時のY座標計算で、手動オフセットを加算し、一番上（Y座標が下がりすぎない）・一番下（Y座標が上がりすぎない）で余白ができすぎないように `std::clamp` を使用して制限をかける。同時に蓄積された手動オフセットそのものもクランプして補正する。
- 外部からスクロール量を加算するための `AddPlaylistScroll(float delta)` および、現在のスクロールオフセットを取得する `GetPlaylistManualScrollY()` を用意する。

### Window の入力イベント処理（スクロールとクリック）
- `Window` クラスに `SetPlaylistScrollCallback(std::function<void(int)>)` と `SetPlaylistClickCallback(std::function<void(int, int)>)` を追加する。
- `WM_MOUSEWHEEL`: `m_isPlaylistHovered` が true の場合、プレイリスト用スクロールコールバックを呼び出す。
- `WM_LBUTTONDOWN`: `m_isPlaylistHovered` が true の場合、プレイリスト用クリックコールバックを呼び出して `x, y` 座標を渡す（ここでは `Play/Pause` 等のコントロール判定より優先または排他制御として扱う）。

### Application での統合
- `Application::Initialize` にて、プレイリストのスクロールコールバックで `Renderer::AddPlaylistScroll(delta)` を呼び出す。
- クリックコールバックにて、`x, y`（物理座標）から論理座標に変換（またはWindow内で論理座標として渡す）し、`ConfigManager::GetPlaylistItemOffsetY()` や現在の `PlaylistManager::GetCurrentIndex()`、`Renderer` のスクロール状態からクリックされたインデックスを算出する。
- 算出されたインデックスが範囲内であれば、`PlaylistManager::JumpToIndex` を呼び出し、`AudioPlayer::Stop` → 対象曲の `Play` → `PrefetchNextTrack` の一連のジャンプ再生処理を実行する。

## 4. 懸念点・確認事項
- プレイリストの一番上および一番下の `clamp` における計算式について、表示領域の中央配置を維持したまま自然なスクロール制限ができるように慎重に計算します。

---

## タスクリスト
- [x] 1. PlaylistManager に `JumpToIndex` メソッドを追加する。
- [x] 2. Renderer に手動スクロール用変数とメソッドを追加し、描画ロジックに反映する。
- [x] 3. Window にプレイリスト用ホイール・クリックイベントのコールバックを追加する。
- [x] 4. Application にコールバックの実装を追加し、インデックス計算と曲のジャンプ再生ロジックを統合する。

### タスク1 詳細作業内容
- `src/PlaylistManager.h`: 指定したシャッフルインデックスへ直接ジャンプするメソッド `void JumpToIndex(size_t index);` の宣言を追加しました。
- `src/PlaylistManager.cpp`: 指定された `index` が範囲内かをチェックし、`m_shuffleIndex` を更新する実装を追加しました。

### タスク2 詳細作業内容
- `src/Renderer.h`: 手動スクロール量を保持する `float m_playlistManualScrollY = 0.0f;` を追加し、操作メソッド `AddPlaylistScroll` と取得メソッド `GetPlaylistManualScrollY` の宣言を追加しました。
- `src/Renderer.cpp`: `AddPlaylistScroll` と `GetPlaylistManualScrollY` を実装しました。また、`Renderer::Render` 内部で `!isPlaylistHovered` 時に `m_playlistManualScrollY` を 0.0f にリセットする処理を追加し、プレイリストの描画時に `m_playlistManualScrollY` をベースのスクロール量に加算し、`std::clamp` によって限界を設けた上で、そのクランプ結果から逆算して `m_playlistManualScrollY` を補正することで、スクロールが制限を超えて蓄積しないように実装しました。

### タスク3 詳細作業内容
- `src/Window.h`: `SetPlaylistScrollCallback` と `SetPlaylistClickCallback` を追加し、コールバック保持用の `std::function` 変数 `m_onPlaylistScroll`, `m_onPlaylistClick` をメンバに追加しました。
- `src/Window.cpp`: 
  - `WM_MOUSEWHEEL` にて、`m_isPlaylistHovered` が true の場合に `m_onPlaylistScroll` へホイール回転量(`zDelta`)を渡すように変更しました。
  - `WM_LBUTTONDOWN` にて、`m_isPlaylistHovered` が true の場合に `m_onPlaylistClick` へクリック座標(`x, y`)を渡すように変更しました（プレイリストクリック時は他のUI判定より優先して早期リターンします）。

### タスク4 詳細作業内容
- `src/Application.cpp`: 
  - `Initialize` メソッド内にて、`m_window.SetPlaylistScrollCallback` と `m_window.SetPlaylistClickCallback` を実装しました。
  - スクロール時は、ホイール回転量(`delta`) から移動量を計算し（ホイール1クリックにつき項目高さの2倍）、`m_renderer.AddPlaylistScroll` に渡しています。
  - クリック時は、渡された物理座標のY値をDPIスケーリングで論理座標に変換し、`Renderer::Render` と同様の計算式で現在のベーススクロール位置や手動スクロール位置を算出した上で、クリックされたY座標に該当するインデックスを導出しています。
  - 計算されたインデックスが範囲内であった場合は、`PlaylistManager::JumpToIndex` でインデックスを更新し、対象の曲を `AudioPlayer::Play` で再生しつつ、曲情報のUI更新と次曲の先読み(`PrefetchNextTrack`)を実施するように統合しました。

---

## Phase 8-8 Hotfix: プレイリストジャンプ再生時のスクロールバグ修正

### 作業1 詳細作業内容
- `src/Renderer.h`: 手動スクロールオフセットを0にリセットする `void ResetPlaylistScroll();` の宣言を追加しました。
- `src/Renderer.cpp`: `ResetPlaylistScroll()` の実装を追加し、内部で `m_playlistManualScrollY = 0.0f;` とするようにしました。

### 作業2 詳細作業内容
- `src/Application.cpp`: プレイリストクリックのコールバック内にて、`m_playlistManager.JumpToIndex(index);` を呼び出した直後に `m_renderer.ResetPlaylistScroll();` を呼び出す処理を追加しました。これにより、クリックして再生曲が切り替わった瞬間に手動スクロール量がリセットされ、対象曲がリストの中央に収まるようになります。

---

## Phase 8-8 Hotfix 2: プレイリストジャンプ再生時のスクロール完全維持
- `src/Application.cpp`: プレイリストのクリックコールバック処理にて、`m_renderer.ResetPlaylistScroll()` を呼び出す代わりに、クリックによる曲の切り替え前後のインデックス差分から補正値を算出し (`(newIndex - oldIndex) * itemHeight`)、それを `m_renderer.AddPlaylistScroll(補正値)` で手動スクロール量に加算する処理に変更しました。これにより、再生曲（中央の基準点）が変わったことによる描画座標のズレが相殺され、クリックしてもリストが視覚的にジャンプせず現在のスクロール位置を完全に維持する極上のUXを実現しました。
- `src/Renderer.h`, `src/Renderer.cpp`: Hotfix 1で追加したものの、Hotfix 2の実装により完全に不要（Dead Code）となった `ResetPlaylistScroll()` メソッドを削除し、コードをクリーンアップしました。
