# RES:HOTFIX作業レポート: FileManagerへのI/Oリトライ機構の一元化

## 1. 実装目的
ファイルアクセス時に発生する一時的なファイルロック競合（他プロセス等による占有）により、メタデータやサムネイルの抽出に失敗する問題を解決するため、ファイルI/Oの統括クラスである `FileManager` の内部にリトライ機構を一元化して実装する。

## 2. 調査内容
`FileManager::ExtractTextMetadata` および `FileManager::ExtractAlbumArtBinary` 内部における `TagLib::MPEG::File` のインスタンス化の際、ファイルが開けない（`isValid()` が false または例外スロー）場合に直ちに抽出失敗としていた。
修正方針として、例外を捕捉する `try-catch` と `Sleep(50)` を組み合わせた最大10回のリトライループを実装し、外部の呼び出し元（`TrackAnalyzer` や `ThumbCacher`、`BackgroundManager`）に対してリトライの事情を一切意識させない「完全なカプセル化」を維持する。

## 3. 対象ファイル
* `src/FileManager.cpp`

## 4. 実装タスクリスト
[x] タスク1: メタデータ抽出処理へのリトライ機構の実装 - `ExtractTextMetadata`内にリトライループを実装
[x] タスク2: アルバムアート抽出処理へのリトライ機構の実装 - `ExtractAlbumArtBinary`内にリトライループを実装

## 5. 詳細作業内容
* タスク1: メタデータ抽出処理へのリトライ機構の実装
    - `<memory>` および `<windows.h>` のインクルードを追加。
    - `ExtractTextMetadata` 内にて `std::unique_ptr<TagLib::MPEG::File>` を用いた最大10回（50ms間隔）のリトライループを追加し、例外発生時やオープン失敗時にリトライするよう変更。
    - リトライを使い果たした場合にのみ、空のメタデータを返すよう修正。

* タスク2: アルバムアート抽出処理へのリトライ機構の実装
    - `ExtractAlbumArtBinary` 内にて `ExtractTextMetadata` と同様に、最大10回（50ms間隔）のリトライループを追加。
    - 例外やオープン失敗を吸収し、上限に達した場合のみ空のバイナリ（抽出失敗時の既存フォールバック）を返すよう修正。
