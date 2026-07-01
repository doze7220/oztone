# 実装レポート：Unicodeファイルパス対応 (Hotfix)

## 目的
Unicode文字（'ø'など、Shift-JISにマッピングできない文字）を含むファイルパスが読み込めないバグを修正するため、アプリ内部のファイルパス管理をマルチバイト（ACP）からワイド文字列（UTF-16 / `std::wstring`）へ完全移行する。

## 実装内容（作業中）

### タスク1: PlaylistManager のワイド文字列化とUTF-8対応
**対象ファイル:**
- `src/PlaylistManager.h`
- `src/PlaylistManager.cpp`

**変更内容:**
- ファイルパスを保持するリスト(`m_playlist`)やセット(`m_playlistSet`)の型を `std::string` から `std::wstring` へ変更。
- `Add`, `GetCurrentTrack`, `GetNextTrack`, `SaveToFile`, `LoadFromFile` 等の引数と戻り値を `std::wstring` に変更。
- プレイリストファイル（.lst）の読み書きにおいて、Win32 APIの `WideCharToMultiByte` および `MultiByteToWideChar` を用いて、内部の `std::wstring` とファイル上の UTF-8 文字列を相互変換する処理を実装し、安全なファイルI/Oを実現。

### タスク2: AudioPlayer のワイド文字列対応
**対象ファイル:**
- `src/AudioPlayer.h`
- `src/AudioPlayer.cpp`

**変更内容:**
- `Play` メソッドのファイルパス引数を `std::wstring` に変更。
- 内部の音声ファイル読み込みで呼び出している関数を、マルチバイト用の `ma_sound_init_from_file` からワイド文字列対応版の `ma_sound_init_from_file_w` へ変更し、`filepath.c_str()` を直接渡すように修正。

### タスク3: TagManager のワイド文字列化
**対象ファイル:**
- `src/TagManager.h`
- `src/TagManager.cpp`

**変更内容:**
- `Load` メソッドのファイルパス引数を `std::wstring` に変更。
- TagLib が Windows 上で `const wchar_t*` を受け取るコンストラクタを持っていることを活用し、`TagLib::MPEG::File` の生成時に `filepath.c_str()` を直接渡すように修正。

### タスク4: Application の連携修正
**対象ファイル:**
- `src/Application.cpp`

**変更内容:**
- 各マネージャ間の橋渡しを行っている部分で、マルチバイト変換（`.string()`）を行わずにワイド文字列のまま処理を行うように修正した。
- `std::filesystem::path` を用いたパスの取り出しや結合において、`.wstring()` メソッドを利用するように統一した。

### タスク5: ビルド確認とドキュメント更新
**対象ファイル:**
- `PROJECT_ARCHITECTURE.md`

**作業内容・検証結果:**
- CMakeによるビルド（`build.bat`）を実行し、コンパイルエラー・リンクエラーが発生せず、`OZtone.exe` が正常に出力されることを確認した。
- `PROJECT_ARCHITECTURE.md` 内の内部パス処理に関する記述を「ACPエンコード（`.string()`）」から「ワイド文字列による Unicodeファイルパスへの完全対応、および UTF-8 への相互変換」という内容に更新し、現在のアーキテクチャ仕様と整合性を保った。

## 結論
以上の修正により、アプリケーション内部のファイルパスが全てワイド文字列（`std::wstring`）で管理されるようになり、Unicode文字を含むファイルパスの再生やタグ取得、プレイリスト保存が正常に行える構造へと移行が完了した。
