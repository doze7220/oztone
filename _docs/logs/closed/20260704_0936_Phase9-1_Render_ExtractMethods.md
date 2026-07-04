# Phase 9-1: Renderer::Render() の大掃除（関数分割）

## 目的
現在の1200行を超える `Renderer::Render()` メソッドの中身を、機能ごとにプライベートメソッドとして切り出し、可読性とメンテナンス性を向上させます。
※「関数の抽出」のみを行い、ロジックの変更やクラスの分割は行いません。

## 変更対象ファイル
- `src/Renderer.h`
- `src/Renderer.cpp`

## 懸念点
- 各描画メソッド間で共有される状態（`m_controlAlpha` やレンダーターゲットのサイズ計算など）の取り扱いに注意が必要です。
- DPIスケールの適用（`m_dpiScale`）は描画の根元で行われているため、各メソッド内での座標計算は論理ピクセルを前提としてそのまま移植可能ですが、D2Dコンテキストのサイズ取得等に関する記述を漏らさず移行する必要があります。

## 作業計画（タスクリスト）

- [x] **タスク1: 背景・ビジュアル系の抽出**
  - `DrawBackground()` の定義と実装
  - `DrawVisualizer(const std::vector<float>& spectrum)` の定義と実装
- [x] **タスク2: 基本UI系の抽出**
  - `DrawAppLogo(bool isHovered)` の定義と実装
  - `DrawTrackInfo()` の定義と実装
  - `DrawNextTrack()` の定義と実装
- [x] **タスク3: 下部コントロール系の抽出**
  - `DrawSeekBar(float progress, const std::wstring& timeString)` の定義と実装
  - `DrawPlaybackControls(bool isPlaying)` の定義と実装
  - `DrawVolumeControl(float volume)` の定義と実装
  - ※ `m_controlAlpha` の計算処理自体は `Render()` 内に残す方針とします。
- [x] **タスク4: 特殊UI系の抽出**
  - `DrawPlaylist(bool isPlaylistHovered, size_t currentTrackIndex, size_t totalTracks, const std::vector<std::wstring>& shuffleList)` の定義と実装
  - `DrawResizeGrip()` の定義と実装

---

## 作業報告（タスク1）
- `Renderer::DrawBackground()` を新設し、`Render()` 内の「背景アルバムアートの描画」と「ダークオーバーレイの描画」の処理を移動しました。引数は不要とし、内部のメンバ変数のみで完結しています。
- `Renderer::DrawVisualizer(const std::vector<float>& spectrum)` を新設し、ビジュアライザの描画処理を移動しました。引数として `spectrum` を受け取ります。
- `src/Renderer.h` の private セクションにこれら2つのメソッド宣言を追加しました。
- 既存のロジックをそのまま移行しており、動作の変更はありません。

## 作業報告（タスク2）
- `Renderer::DrawAppLogo(bool isHovered)` を新設し、アプリアイコンの描画処理を移動しました。
- `Renderer::DrawTrackInfo()` を新設し、「左下アルバムアートの描画」および「曲情報テキストの描画」の処理を統合して移動しました。引数は不要とし、内部のメンバ変数を参照しています。
- `Renderer::DrawNextTrack()` を新設し、「次の曲」情報表示の処理を移動しました。引数不要でメンバ変数のみに依存しています。
- `src/Renderer.h` にメソッド宣言を追加しました。
- 描画順・条件分岐・計算式・変数・処理内容は変更せず、安全にメソッド抽出のみを行いました。

## 作業報告（タスク3）
- `Renderer::DrawSeekBar(float progress, const std::wstring& timeString)` を新設し、シークバーと時間テキストの描画処理を移動しました。
- `Renderer::DrawPlaybackControls(bool isPlaying)` を新設し、再生コントロールの描画処理を移動しました。
- `Renderer::DrawVolumeControl(float volume)` を新設し、音量UIの描画処理を移動しました。
- `src/Renderer.h` の private セクションにこれら3つのメソッド宣言を追加しました。
- `Render()` 内で行われている `m_controlAlpha` の計算・更新処理は `Render()` 側にそのまま残し、抽出した各描画メソッドはその値を参照して描画を行うようにすることで、既存の処理内容や描画順、条件分岐を一切変更せずに安全に抽出を行いました。

## 作業報告（タスク4）
- `Renderer::DrawPlaylist(bool isPlaylistHovered, size_t currentTrackIndex, size_t totalTracks, const std::vector<std::wstring>& shuffleList)` を新設し、プレイリストUIの描画処理を移動しました。
- `Renderer::DrawResizeGrip()` を新設し、ウィンドウリサイズ用グリップの描画処理を移動しました。
- `src/Renderer.h` の private セクションにこれら2つのメソッド宣言を追加しました。
- `Render()` メソッドに渡されている引数（`isPlaylistHovered`, `currentTrackIndex`, `totalTracks`, `shuffleList`）をそのまま `DrawPlaylist()` に渡す設計とし、既存の描画順・条件分岐・計算式・変数・処理内容は一切変更せずに安全に抽出のみを行いました。
- これにより、1200行超に及んでいた巨大な `Render()` メソッドは、各描画メソッドを順次呼び出すだけの非常にクリーンで見通しの良い形にリファクタリングされました。
