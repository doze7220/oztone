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
[ ] タスク2: FileManagerの実装（TagManagerロジックの移植）
    - `src/FileManager.cpp` を新規作成し、既存のTagLibを用いたメタデータ抽出ロジックをここに移植・カプセル化する。
[ ] タスク3: 外部クラスの配線の付け替え
    - `Application` クラスや `TrackAnalyzer` クラスなどを開き、既存の `TagManager` へのアクセスを新設した `FileManager` へと綺麗に結線し直す。
[ ] タスク4: TagManagerの完全パージとビルド環境の更新
    - 不要になった `src/TagManager.h` および `src/TagManager.cpp` をファイルシステムから物理削除する。
    - `CMakeLists.txt` を更新してビルドを通す。
[ ] タスク5: ドキュメントの更新
    - `PROJECT_ARCHITECTURE.md` 等を更新する。

## 4. 詳細作業内容
### タスク1: FileManagerのインターフェース設計と作成
    - `src/FileManager.h` を新規作成し、TagLib依存を隠蔽する窓口を定義した。抽出したメタデータをまとめる専用構造体 `AudioMetadata` を定義し、静的メソッド `ExtractMetadata` を宣言した。
### タスク2: FileManagerの実装（TagManagerロジックの移植）
    - （未着手）
### タスク3: 外部クラスの配線の付け替え
    - （未着手）
### タスク4: TagManagerの完全パージとビルド環境の更新
    - （未着手）
### タスク5: ドキュメントの更新
    - （未着手）

### HOTFIX1
#### 原因・理由:Phase 23-2 タスク1実装
    - FileManagerのインターフェース設計と作成の要求に対応するため。

#### 対象ファイル: 
    - src/FileManager.h (新規作成)

#### 対応:FileManagerインターフェースの設計と作成
    - `src/FileManager.h` を新規作成した。
    - TagLibへの依存を隠蔽するため、ヘッダ内でTagLib関連のインクルードを行わず、`std::wstring`, `std::vector<BYTE>` などの標準型および `windows.h` のみを使用。
    - 抽出したメタデータをまとめる専用の構造体 `AudioMetadata` を定義した。
    - `FileManager` クラスを静的ユーティリティクラスとして設計し、`ExtractMetadata` メソッドを宣言した。
