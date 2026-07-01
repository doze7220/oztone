【目的】
Unicode文字（'ø'など、Shift-JISにマッピングできない文字）を含むファイルパスが読み込めないバグを修正するため、アプリ内部のファイルパス管理をマルチバイト（ACP）からワイド文字列（UTF-16 / std::wstring）へ完全移行します。

【実装要件】
1. PlaylistManager のワイド文字列化
対象: src/PlaylistManager.h, src/PlaylistManager.cpp
- ファイルパスを保持するリストやセットの型を `std::string` から `std::wstring` へ変更してください。
- `Add`, `GetCurrentTrack`, `GetNextTrack` 等の引数と戻り値を `std::wstring` に変更してください。
- `SaveToFile`, `LoadFromFile` において、プレイリストファイル（.lst）の読み書きを UTF-8 で行うように修正してください（Win32APIの `WideCharToMultiByte` / `MultiByteToWideChar` を用いて `std::wstring` と UTF-8 の `std::string` を相互変換してファイルI/Oを行うのが最も安全です）。

2. AudioPlayer のワイド文字列対応
対象: src/AudioPlayer.h, src/AudioPlayer.cpp
- `Play` メソッドの引数を `std::wstring` に変更してください。
- 内部で呼び出している `ma_sound_init_from_file` を、ワイド文字列対応版の `ma_sound_init_from_file_w` に変更し、ファイルパスを渡してください。

3. TagManager のワイド文字列対応
対象: src/TagManager.h, src/TagManager.cpp
- `Load` メソッドの引数を `std::wstring` に変更してください。
- `TagLib::MPEG::File` はWindows環境において `const wchar_t*` を受け取るコンストラクタを持っています。引数として `filepath.c_str()` を渡すように修正してください。

4. Application の橋渡し修正
対象: src/Application.cpp
- `OnFilesDropped` やプレイリスト初期化などのパス処理において、`std::filesystem::path::string()` によるマルチバイト化（ACP変換）を行っていた箇所を修正し、`.wstring()` のまま `PlaylistManager` 等の各クラスへ渡すように変更してください。

プロジェクト憲法に従い、既存のアーキテクチャを崩さず、実装完了後は変更点をまとめた実装レポートを出力し、PROJECT_ARCHITECTURE.md の該当の仕様記述を更新してください。
