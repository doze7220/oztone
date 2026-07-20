# RES:実装計画・作業レポート Phase 23-2: FileManagerの新設とTagManagerの吸収

## 1. 実装目的
ファイルI/Oおよびメタデータ抽出を単一の窓口として引き受ける門番クラス `FileManager` を新規に構築する。さらに、既存のメタデータ抽出処理を担っていた `TagManager` を解体・吸収し、外部に対してTagLibの存在を完全に隠蔽するAdapterパターンとしてのアーキテクチャへと移行する。これにより、将来的なファイル読み込みフロー（バイナリローディング等）の集約に向けた基盤を整える。

## 2. アーキテクチャ設計
### 要件1: FileManager クラスの新設とTagLibの統合
    - `src/FileManager.h` / `src/FileManager.cpp` を新規作成する。
    - ファイルパス (`std::wstring`) を受け取り、曲名、アーティスト名、曲の長さなどのテキスト情報およびAPIC画像バイナリを抽出して専用の構造体（例: `AudioMetadata`）として返すメソッドを実装する。
    - 処理においてTagLibの `MPEG::File` 等を用い、必要なメタデータのみをオンデマンドで取得する（全ファイルインメモリ化は行わない）。
    - 外部にはTagLibの型を一切露出させない（`FileManager.h` で `<taglib/tag.h>` 等をインクルードしない）。実装側 (`.cpp`) のみで参照するカプセル化（Pimpl/Adapter）を徹底する。

### 要件2: TagManager の解体と吸収
    - `src/TagManager.h` / `src/TagManager.cpp` を削除（または無効化・ビルド除外）する。
    - 既存の `TagManager::ExtractMetadata` 等のロジックを `FileManager` に移植・リファクタリングする。
    - 抽出した情報（メタデータ・画像バイナリ）の返却インターフェースを整理し、`Application` や `TrackAnalyzer` が扱いやすい構造に純化する。

### 要件3: Applicationクラス等への配線と置き換え
    - `Application`, `TrackAnalyzer` など、現在 `TagManager` に依存している箇所を `FileManager` を用いるように置換する。
    - `CMakeLists.txt` を更新し、`src/FileManager.cpp` を追加、`src/TagManager.cpp` を削除（除外）する。
    - これらを通してプロジェクト全体が正常にビルドでき、メタデータ抽出機能が旧来通り動作することを確認する。

## 3. 実装タスクリスト
[x] タスク1: FileManagerのインターフェース設計と作成
    - `src/FileManager.h` を新規作成し、TagLib依存を隠蔽する窓口を定義する。
[x] タスク2: FileManagerの実装（TagManagerロジックの移植）
    - `src/FileManager.cpp` を新規作成し、既存のTagLibを用いたメタデータ抽出ロジックをここに移植・カプセル化する。
[x] タスク3: 外部クラスの配線の付け替え
    - `Application` クラスや `TrackAnalyzer` クラスなどを開き、既存の `TagManager` へのアクセスを新設した `FileManager` へと綺麗に結線し直す。
[x] タスク4: TagManagerの完全パージとビルド環境の更新
    - 不要になった `src/TagManager.h` および `src/TagManager.cpp` をファイルシステムから物理削除する。
    - `CMakeLists.txt` を更新してビルドを通す。
[ ] タスク5: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` 等を更新する。

## 4. 詳細作業内容
### タスク1: FileManagerのインターフェース設計と作成
    - `src/FileManager.h` を新規作成し、TagLib依存を隠蔽する窓口を定義した。抽出したメタデータをまとめる専用構造体 `AudioMetadata` を定義し、静的メソッド `ExtractMetadata` を宣言した。

    #### 原因・理由:Phase 23-2 タスク1実装
        - FileManagerのインターフェース設計と作成の要求に対応するため。

    #### 対象ファイル: 
        - src/FileManager.h (新規作成)

    #### 対応:FileManagerインターフェースの設計と作成
        - `src/FileManager.h` を新規作成した。
        - TagLibへの依存を隠蔽するため、ヘッダ内でTagLib関連のインクルードを行わず、`std::wstring`, `std::vector<BYTE>` などの標準型および `windows.h` のみを使用。
        - 抽出したメタデータをまとめる専用の構造体 `AudioMetadata` を定義した。
        - `FileManager` クラスを静的ユーティリティクラスとして設計し、`ExtractMetadata` メソッドを宣言した。

    #### HOTFIX
    ##### 原因・理由:FileManagerインターフェースの責務分離
        - 解析スレッドが多数の曲を処理する際、重い画像バイナリを全てメモリにロードしてしまうとパフォーマンス低下を引き起こすため。
        - 画像バイナリは必要な時にのみオンデマンドで取得すべきであるため、テキスト抽出と画像抽出の責務を分離した。

    ##### 対象ファイル: 
        - src/FileManager.h

    ##### 対応:テキスト抽出と画像抽出のメソッド分割
        - `AudioMetadata` 構造体から `coverArt` フィールドを削除し、軽量なテキスト情報のみを保持するように変更した。
        - 既存の `ExtractMetadata` メソッドを `ExtractTextMetadata` にリネームした。
        - 画像バイナリのみを抽出する専用メソッド `ExtractAlbumArtBinary` を新たに追加した。
        - クラス外からの TagLib 依存隠蔽（Adapterパターン）は維持している。

### タスク2: FileManagerの実装（TagManagerロジックの移植）
    - `src/FileManager.cpp` を新規作成し、`ExtractTextMetadata` および `ExtractAlbumArtBinary` メソッドの実装を行った。

    #### 原因・理由:Phase 23-2 タスク2実装
        - 旧TagManagerの責務（テキストメタデータ・画像バイナリ抽出）をFileManagerへ移植し、カプセル化するため。

    #### 対象ファイル:
        - src/FileManager.cpp (新規作成)

    #### 対応:TagManagerロジックの移植
        - `src/FileManager.cpp` を新規作成した。
        - `<taglib/mpegfile.h>`, `<taglib/id3v2tag.h>`, `<taglib/attachedpictureframe.h>` 等のTagLibインクルードをこのファイル内でのみ行い、ヘッダ側への依存漏出を防ぐことでAdapterパターンを完成させた。
        - `ExtractTextMetadata` で曲名、アーティスト名、曲の長さのテキスト情報を効率的に取得するように実装した。
        - `ExtractAlbumArtBinary` で `TagLib::ID3v2::AttachedPictureFrame` からバイナリデータを取得し、`std::vector<BYTE>` として返却するように実装した。それぞれ必要な情報だけを抽出するストイックな処理とした。

### タスク3: 外部クラスの配線の付け替え
    #### 原因・理由:Phase 23-2 タスク3実装
        - `TagManager` の廃止に向け、`Application`, `TrackAnalyzer`, `ThumbCacher` 等での `TagManager` 依存を `FileManager` に置き換えるため。

    #### 対象ファイル: 
        - src/Application.h
        - src/Application_Playback.cpp
        - src/TrackAnalyzer.cpp
        - src/ThumbCacher.cpp

    #### 対応:外部クラスの配線の付け替え
        - 各ファイルの `#include "TagManager.h"` を `#include "FileManager.h"` に変更した。
        - `Application.h` のメンバ変数 `m_tagManager` を削除した。
        - `Application_Playback.cpp` および `TrackAnalyzer.cpp` のメタデータ抽出処理を、`TagManager` のインスタンスメソッドによる処理から `FileManager::ExtractTextMetadata` の静的メソッド呼び出しに変更し、`FileManager::TextMetadata` 構造体を受け取るように修正した。

### タスク4: TagManagerの完全パージとビルド環境の更新
    - `src/TagManager.h` および `src/TagManager.cpp` を物理的に削除した。
    - `CMakeLists.txt` において `TagManager` の記述を削除し、`FileManager` に差し替え、ビルドが通るようにした。

    #### 原因・理由:Phase 23-2 タスク4実装
        - 旧クラスの完全除去と新しいクラスのビルドへの統合を行い、プロジェクト構成を浄化するため。

    #### 対象ファイル: 
        - src/TagManager.h (削除)
        - src/TagManager.cpp (削除)
        - CMakeLists.txt

    #### 対応:不要ファイルのパージとビルド環境更新
        - `src/TagManager.h` と `src/TagManager.cpp` をファイルシステムから削除。
        - `CMakeLists.txt` 内のソースファイルリストおよびヘッダファイルリストから上記2ファイルを削除し、代わりに `src/FileManager.cpp` と `src/FileManager.h` を追加した。
### タスク5: ドキュメントの更新
    - （未着手）
