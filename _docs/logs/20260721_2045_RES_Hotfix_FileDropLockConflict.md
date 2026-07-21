# RES:HOTFIX作業レポート: ファイルドロップ直後の再生スキップ（ファイルロック競合）修正

## 1. 実装目的
ファイルドロップ直後、ミリ秒単位のファイルロック競合により即時再生が失敗し、意図しない曲スキップが発生する不具合を修正する。

## 2. 調査内容
`Application_FileDrop.cpp` の `OnFilesDropped` メソッドにおいて、ファイルのドロップ直後にバックグラウンドの解析スレッドを起床させるエンキュー処理が、メインスレッドの即時再生（`PlayCurrentTrack()`）よりも先に行われていた。これにより、解析スレッドとメインスレッドの間でファイルロック競合が発生していた。
この処理順序を適正化し、メインスレッドが優先してファイルをオープンできるように、解析スレッドへのキューイング処理を即時再生処理の「後」へ移動する方針で修正を行った。

## 3. 対象ファイル
* `src/Application_FileDrop.cpp`

## 4. 実装タスクリスト
[x] タスク1: `OnFilesDropped` 処理順序の入れ替え - 解析スレッドへのキューイングと起床処理全体を、`PlayCurrentTrack()` 呼び出し後に移動する。

## 5. 詳細作業内容
* タスク1: `OnFilesDropped` 処理順序の入れ替え
    - `src/Application_FileDrop.cpp` 内において、`m_playlistManager.GetShuffleList()` を用いて未解析トラックを取得し、`m_trackAnalyzer.AddTrackToQueue()` および `m_thumbnailManager` へのエンキューを行うブロックを抽出。
    - 抽出したブロックを、`PlayCurrentTrack(-1)` 等を行う即時再生用のブロック（`if (!isShiftPressed || (wasEmpty && !m_audioManager.IsPlaying())) { ... }`）の直後に移動させた。
