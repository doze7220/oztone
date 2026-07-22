# RES:HOTFIX作業レポート: PlayCurrentTrackのロック優先度最適化

## 1. 実装目的
ファイルロック競合による初回再生スキップバグを解決するため、メインスレッドの音声再生エンジン（AudioManager）が確実に一番最初にファイルロックを取得し、後から発動する非同期ワーカーたち（ThumbnailManager, BackgroundManager）が安全に待機できるよう、`PlayCurrentTrack` 内の処理のロック優先順位を適正化する。

## 2. 調査内容
`src/Application_Playback.cpp` 内の `Application::PlayCurrentTrack` を調査した結果、すでに `m_thumbnailManager.GetOrGenerateThumbId` や `m_backgroundManager.RequestLoad` などの非同期タスク発注処理は `m_audioManager.Play(track)` の直後に配置されていることが確認された。
しかし、同期処理である `UpdateTrackMetadataIfNeeded(track)` がアニメーション完了時の `onComplete` コールバック内に配置されていたため、`Play` によってファイルがロックされた後に同期的なメタデータ抽出が走り、競合が発生する状態であった。
修正方針として、`UpdateTrackMetadataIfNeeded` を `Play` 呼び出しの直前に移動し、確実にファイルロックの競合を回避しつつ、音声再生後に非同期処理が発注される理想的な優先順位を確立する。

## 3. 対象ファイル
* `src/Application_Playback.cpp`

## 4. 実装タスクリスト
- [x] タスク1: `Application::PlayCurrentTrack` 内の処理順序の適正化 - 同期処理である `UpdateTrackMetadataIfNeeded` を `Play` の前に移動し、非同期発注処理が `Play` 直後に行われる構成を担保した。

## 5. 詳細作業内容
* タスク1: `Application::PlayCurrentTrack` 内の処理順序の適正化
    - `UpdateTrackMetadataIfNeeded` を `onComplete` ラムダ式内から削除し、関数先頭の `m_audioManager.Play(track)` の直前に移動した。
    - `m_thumbnailManager.GetOrGenerateThumbId` や `m_backgroundManager.RequestLoad` については、既に `m_audioManager.Play(track)` 実行ブロック直後に記述されていたため、現在の配置を維持し、要件にある非同期ワーカーへの発注優先順位を担保した。
