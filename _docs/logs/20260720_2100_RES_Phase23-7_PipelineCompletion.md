# RES:実装計画・作業レポート Phase 23-7: パイプラインの完全開通と司令塔の整理

## 1. 実装目的
Phase 23の総仕上げとして、`Application` クラスにおける曲ロードフロー（パイプライン）を整理・純化する。新設された各Manager（`AudioManager`, `ThumbnailManager`, `BackgroundManager`）への適切なデータ分配と発注を確立し、一時的に失われていた背景表示モードの切り替え機能を復活させる。さらに、旧アーキテクチャ解体時に残された不要な工事マーカーコメントを徹底的に削除し、コードベースをクリーンアップする。

## 2. アーキテクチャ設計
### 要件1: 司令塔 (Application) のデータ分配フローの完成
- `Application::PlayCurrentTrack` 内の `dataProvider` および完了コールバック（`onComplete`）において、楽曲パスを元に `ThumbnailManager` へのサムネイル生成依頼（`GetOrGenerateThumbId` と `EnqueueTrack` の呼び出し）が漏れなく的確に発注されるようパイプラインを結線する。
- `BackgroundManager` と `ThumbnailManager` が同時に適切なパスを受け取り、非同期処理を円滑に開始できるようにする。

### 要件2: バックグラウンド表示モードの復活 (Hotfix)
- `Renderer::DrawBackground` の描画処理において、`ConfigManager::GetBackgroundArtMode()` の値（0:再生中, 1:非表示, 2:デフォルト固定）を評価する分岐を追加する。
- `1:非表示` の場合は背景画像の描画を完全にスキップする。
- `2:デフォルト固定` の場合は、現在や過去のWIC画像を無視し、`m_placeholderArtBitmap` を用いてデフォルトの背景を固定描画する。
- フェード管理やWIC画像抽出の非同期処理は `BackgroundManager` が担当し続け、Rendererは単純に「どう描画するか」という表示の分岐のみを担うことで責務を完全に分離する。

### 要件3: コードの大掃除（工事残骸のパージ）
- `src/Renderer.h` および `src/Renderer.cpp` に残存している「Phase23-1 背景アートパージに伴い削除」に関連する不要なコメントやマーカーを検索し、行ごと削除してコードをクリーンアップする。

## 3. 実装タスクリスト
- [x] タスク1: 司令塔 (Application) のデータ分配フローの整理
    - `src/Application_Playback.cpp` の `PlayCurrentTrack` を修正し、`ThumbnailManager::EnqueueTrack` を用いたサムネイル発注処理を整備する。
- [x] タスク2: バックグラウンド表示モードの復活 (Hotfix)
    - `src/Renderer_Draw.cpp` の `Renderer::DrawBackground` を修正し、`GetBackgroundArtMode()` に基づく背景描画の分岐（通常・非表示・固定）を実装する。
- [ ] タスク3: 工事残骸マーカーのパージ
    - `src/Renderer.h` および `src/Renderer.cpp` から不要になった結線待ちコメント・削除済みメソッドの痕跡を完全に除去する。

## 4. 詳細作業内容
### タスク1: 司令塔 (Application) のデータ分配フローの整理
**【対象ファイル】**
- `src/Application_Playback.cpp` (編集)

**【作業内容】**
- `Application::PlayCurrentTrack` において、アニメーション完了時（`onComplete`コールバック内）に遅れて実行されていたサムネイル発注処理（`ThumbnailManager::GetOrGenerateThumbId`）を、関数内の先頭（曲決定直後）に引き上げました。
- これにより、目標トラックが決定した瞬間に `ThumbnailManager` および `BackgroundManager` への非同期ロードの発注が一括して行われるようになり、司令塔としてのデータ分配フロー（パイプライン）の結線が整理・純化されました。

### タスク2: バックグラウンド表示モードの復活 (Hotfix)
**【対象ファイル】**
- `src/Renderer_Draw.cpp` (編集)

**【作業内容】**
- `Renderer::DrawBackground` 内に、`m_config->GetBackgroundArtMode()` を用いた分岐を追加しました。
- モードが `0` (再生中) の場合は、通常通りWIC画像を用いたクロスフェード描画を行うようにしました。
- モードが `1` (非表示) の場合は、背景画像描画を完全にスキップするようにしました。
- モードが `2` (デフォルト固定) の場合は、現在・過去のWIC画像を無視し、`m_placeholderArtBitmap` が存在すればそれを用いて固定描画を行うようにしました。
- ダークオーバーレイの描画処理は、これらの背景画像描画モードに関わらず従来通り後続で実行される構造を維持しました。

### タスク3: 工事残骸マーカーのパージ
**【対象ファイル】**
- 

**【作業内容】**
- 
