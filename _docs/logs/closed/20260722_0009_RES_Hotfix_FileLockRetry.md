# RES:HOTFIX作業レポート: ファイルロックリトライ機構の実装

## 1. 実装目的
他プロセスのファイルアクセスや一時的なロック競合に起因して、曲のメタデータ抽出や波形スキャン、サムネイル生成が失敗し、そのままデータが欠落したりキューが詰まったりする問題を防ぐため。

## 2. 調査内容
`TrackAnalyzer` および `ThumbCacher` において、`FileManager` や `AudioManager` の読み出し処理がエラーまたは例外となった場合、一度の失敗で直ちに諦める実装になっていた。
これを改善するため、エラーや例外発生時に `Sleep(50)` を挟んで最大10回（計500ms）リトライするループを各ワーカーの読み出し処理部分に導入した。

## 3. 対象ファイル
* `src/TrackAnalyzer.cpp`
* `src/ThumbCacher.cpp`

## 4. 実装タスクリスト
[x] タスク1: `TrackAnalyzer` へのファイルロック・リトライ機構 - `ParseThreadFunc` 内のメタデータ抽出・波形スキャン処理にリトライループを追加。
[x] タスク2: `ThumbCacher` へのファイルロック・リトライ機構 - `WorkerLoop` 内のサムネイルバイナリ抽出および変換処理にリトライループを追加し、最終的に失敗した場合のみロールバックするように堅牢化。

## 5. 詳細作業内容
* タスク1: `TrackAnalyzer` へのファイルロック・リトライ機構
    - `FileManager::ExtractTextMetadata` 呼び出し部分を `try-catch` と `for` ループで囲み、例外発生時は `Sleep(50)` を挟んで最大10回再試行するよう実装。
    - `AudioManager::ScanAudioData` 呼び出し部分も同様に囲み、関数が `false` を返した場合や例外発生時にリトライを実行するよう実装。
* タスク2: `ThumbCacher` へのファイルロック・リトライ機構
    - `FileManager::ExtractAlbumArtBinary` から `CookThumbnailImage`、および `StoreCookedData` に至る抽出〜保存のフロー全体をリトライループで囲むよう実装。
    - いずれかの段階で失敗または例外が発生した場合は `Sleep(50)` を挟んで最大10回再試行し、最終的に処理が完了しなかった場合のみ `m_db->RollbackThumbId(filepath)` を呼び出して発番を取り消すよう堅牢化。
