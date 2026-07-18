# RES: 実装計画・作業レポート Phase 21-9: 完全独立型トラックドラムエンジンの構築

## 1. 実装目的
プレイリストのデータ構造からトラックドラムの描画データを完全に切り離し、ドラム自身が独立したスナップショットデータと操作意図を持つ「最強のバケツリレーアーキテクチャ」を構築する。これにより、プレイリスト再構築時などのデータ喪失を防ぎ、安全かつ堅牢なアニメーションを実現する。
本計画書は単体で Single Source of Truth (SSOT) となるよう、削除対象の旧ハック実装についても明記する。

## 2. アーキテクチャ設計
### 要件1: ドラム専用データ構造と操作意図の定義
    - `Renderer.h` に描画用データをカプセル化した `DrumSlotData` 構造体を定義する（曲名、アーティスト、トラックNo文字列、画像ビットマップを含む）。
    - ドラムの動作を示す `DrumMoveType` 列挙型（`Next`, `Prev`, `Jump`, `CrossPlaylist`, `Reset`）を定義する。
### 要件2: Application層からの操作意図伝達
    - `Application` 層から `SetTrackInfo` を呼び出す際、操作意図に応じた `DrumMoveType` と表示用文字列（TrackNo）を渡すようにインターフェースを改修する。
### 要件3: 独立したバケツリレーと位置リセットの構築
    - `Renderer` 内部に `m_oldDrumSlot` と `m_nowDrumSlot` を実体コピーとして保持する。
    - `DrumMoveType` に応じた完全なバケツリレー（`m_oldDrumSlot = m_nowDrumSlot`等）と、目標インデックスに対する `m_drumPosition` のワープ処理を実装する。
### 要件4: Widget_TrackInfo の描画純化
    - 仮想スロット描画ループから `ctx.shuffleMetadataList` などのプレイリストデータ参照を完全に排除する（削除対象ハック）。
    - 描画は `oldDrumSlot` と `nowDrumSlot` の実体のみに依存させる。

## 3. 実装タスクリスト
- [x] タスク1: `Renderer.h` へのデータ構造定義と `SetTrackInfo` シグネチャ変更
- [x] タスク2: `Renderer.cpp` および `Renderer_Update.cpp` でのバケツリレーと位置ワープ処理の実装
- [x] タスク3: `Application` 層の全 `SetTrackInfo` 呼び出し箇所への `DrumMoveType` 適用
- [x] タスク4: `WidgetContext.h` への `DrumSlotData` 追加と古い変数のパージ
- [x] タスク5: `Widget_TrackInfo.cpp` での描画処理の純化と古いハックのパージ

## 4. 詳細作業内容
### タスク1: `Renderer.h` へのデータ構造定義と `SetTrackInfo` シグネチャ変更
    - `Renderer.h` に以下を定義:
      ```cpp
      enum class DrumMoveType { Next, Prev, Jump, CrossPlaylist, Reset };
      struct DrumSlotData {
          std::wstring title;
          std::wstring artist;
          std::wstring trackNoString; // "---" や "1" などの表示文字列
          Microsoft::WRL::ComPtr<ID2D1Bitmap> artBitmap;
      };
      ```
    - `Renderer` のメンバ変数整理:
      - 追加: `DrumSlotData m_nowDrumSlot;`, `DrumSlotData m_oldDrumSlot;`
      - 削除（過去のハックパージ対象）: `m_oldTrackTitle`, `m_oldTrackArtist`, `m_oldTrackIndex`, `m_oldArtBitmap`
      - 維持: `m_drumStartIndex`, `m_drumTargetIndex` は絶対インデックスとして保持する。
    - `SetTrackInfo` シグネチャを `void SetTrackInfo(const std::wstring& title, const std::wstring& artist, const std::wstring& trackNoString, DrumMoveType moveType, size_t targetIndex);` へ変更。

### タスク2: `Renderer.cpp` および `Renderer_Update.cpp` でのバケツリレーと位置ワープ処理の実装
    - `Renderer::SetTrackInfo` 内での処理:
      - バケツリレー: `m_oldDrumSlot = m_nowDrumSlot;`
      - 新規データのセット: `m_nowDrumSlot.title = title; m_nowDrumSlot.artist = artist; m_nowDrumSlot.trackNoString = trackNoString;` (Bitmapは `SetAlbumArt` 時に `m_nowDrumSlot.artBitmap` にもセットするよう連携)
      - `DrumMoveType` に応じた `m_drumPosition` と `m_drumStartIndex` のワープ:
        - `Next` / `CrossPlaylist`: `m_drumPosition = targetIndex - 1.0;`, `m_drumStartIndex = targetIndex - 1;`
        - `Prev`: `m_drumPosition = targetIndex + 1.0;`, `m_drumStartIndex = targetIndex + 1;`
        - `Jump`: `m_drumStartIndex = 現在の m_drumTargetIndex;` （インデックス差分による遠距離ジャンプ維持）
        - `Reset`: `m_drumPosition = targetIndex;`, `m_drumStartIndex = targetIndex;`
      - `m_drumTargetIndex = targetIndex;`
      - アニメーションフラグ `m_isDrumAnimating` と初速度設定を行う。
    - **削除対象ハック**: `Renderer_Update.cpp` 側で `m_drumTargetIndex != currentTrackIndex` を毎フレーム監視してアニメーションを自動トリガー・リセットしていた冗長な推測処理を完全にパージし、すべて `SetTrackInfo` 呼び出し時の明示的ワープに一本化する。
    - **完了**: `SetTrackInfo` 内部で `m_oldDrumSlot = m_nowDrumSlot` による絶対的なバケツリレーを実装し、指定された `DrumMoveType` に応じた `m_drumPosition` のワープ処理を実装した。また、`SetAlbumArt` 時に `m_nowDrumSlot.artBitmap` が更新されるように修正した。

### [x] タスク3: `Application` 層の全 `SetTrackInfo` 呼び出し箇所への `DrumMoveType` 適用
    - `Application_Render.cpp`, `Application_Playlist.cpp`, `Application_Playback.cpp`, `Application_Initialize.cpp`, `Application_FileDrop.cpp` を網羅的に修正。
    - 呼び出し時の引数に `DrumMoveType` (曲送りなら `Next` など) と `trackNoString` を追加する。
    - プレイリストのインデックス操作からトラック番号表示文字列を算出するロジック（`shuffleIndices` のルックアップ等）を `Application` 側に寄せる。
    - **完了**: `PlayCurrentTrack` に `DrumMoveType` 引数を追加し、すべてのアクションから意図に合わせた `DrumMoveType` (Next, Prev, Jump, CrossPlaylist, Reset) を `SetTrackInfo` および `PlayCurrentTrack` へ明示的に渡すように修正した。`Renderer.h` の `SetTrackInfo` から `moveType` などのデフォルト引数も削除し、シグネチャを純化した。

### [x] タスク4: `WidgetContext.h` への `DrumSlotData` 追加と古い変数のパージ
    - `Renderer.h` から `DrumSlotData` 定義を `WidgetContext.h` へ移動した。
    - `WidgetContext` に `DrumSlotData oldDrumSlot;` と `DrumSlotData nowDrumSlot;` を追加した。
    - 既存の `drumStartIndex` と `drumTargetIndex` は維持。
    - **完了**: 不要となったバラバラの変数（`oldTrackTitle`, `oldTrackArtist`, `oldArtBitmap`）を `WidgetContext.h`, `Renderer.h`, `Renderer_Context.cpp`, `Renderer.cpp` から完全に削除した。

### [x] タスク5: `Widget_TrackInfo.cpp` での描画処理の純化
    - **完了**:
      - 仮想スロットループ内で、描画対象のインデックスが `ctx.currentTrackIndex` と一致するかどうかだけで単純に分岐するように修正。
      - `i == ctx.currentTrackIndex` の場合は `ctx.nowDrumSlot` を使用し、それ以外の場合は一律で `ctx.oldDrumSlot` を使用。
      - `ctx.shuffleMetadataList` などのプレイリストデータへのインデックスアクセスハックをすべて排除。完全に純粋なバケツリレースナップショットのみに依存する堅牢な描画方式となった。
