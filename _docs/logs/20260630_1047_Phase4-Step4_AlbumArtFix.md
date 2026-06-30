# Phase 4-4: アルバムアート表示不具合の修正 (Implementation Log)

## 実施内容
1. **不具合の調査**
   - ユーザー報告：「アルバムアートが変わらない。設定されたMP3を使ってるはずだが。」
   - `mutagen`（Python）を用いて `assets/test.mp3` に `APIC` フレーム（PNG画像）が正しく埋め込まれていることを確認。
   - 各種デバッグ手法（ファイルログ、MessageBox、OutputDebugString）を試行し、ファイルI/Oの制約からログファイルが生成されない状態も調査。
   - コマンドラインから直接EXEを実行した際の stderr に `TagLib: Could not open file assets/test.mp3` というエラーメッセージを発見。

2. **根本原因の特定**
   - `TagManager::Load()` 内で、同一ファイルに対して `TagLib::FileRef` と `TagLib::MPEG::File` を**二重にオープン**していた。
   - Windows環境では `FileRef` がファイルを排他ロックするため、直後の `MPEG::File` によるオープンが失敗し、`mpegFile.isValid()` が `false` を返していた。
   - その結果、ID3v2タグからの APIC フレーム取得処理に到達せず、`m_albumArtBytes` が常に空のままだった。
   - 一方、タイトルとアーティスト名は先に開いた `FileRef` 経由で取得できていたため、テキスト情報だけは正しく表示されていた。

3. **修正内容**
   - `TagManager::Load()` を `TagLib::MPEG::File` のみで統一。一度のファイルオープンでタイトル・アーティスト名（`mpegFile.tag()`）と APIC フレーム（`mpegFile.ID3v2Tag()`）を全て取得する構造に変更。
   - 不要になった `#include <taglib/fileref.h>` を削除。
   - デバッグ用に追加していた `#include <fstream>` や各種ログ出力コードを `Application.cpp` からクリーンアップ。

## 変更ファイル一覧
| ファイル | 変更内容 |
|---|---|
| `src/TagManager.cpp` | `FileRef` + `MPEG::File` の二重オープンを `MPEG::File` 単独に統一。不要インクルード削除。 |
| `src/Application.cpp` | デバッグ用の `#include <fstream>` とログ出力コードを削除。 |

## 確認状況
- `build.bat` によるビルドが正常に終了することを確認。
- EXE実行時に `TagLib: Could not open file` エラーが出力されないことを確認。
- ユーザーによりアルバムアートが正しく表示されることを確認済み。

## 教訓
- TagLib において、同一ファイルを複数の `File` オブジェクトで同時にオープンすると、Windows のファイルロック機構により2回目以降のオープンが暗黙的に失敗する。メタデータの取得はファイル形式固有のクラス（`MPEG::File` 等）で一本化すべき。
