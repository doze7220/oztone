# Unicodeファイルパス対応 (Hotfix)

指定された要件に基づき、Unicode文字を含むファイルパス（'ø' などShift-JIS非対応の文字を含むパス）を正常に読み込めるようにするため、内部のファイルパス管理をワイド文字列（`std::wstring`）へと移行します。

## Proposed Changes

---

### PlaylistManager
ファイルパスを管理するデータ構造およびAPIを `std::wstring` に変更し、プレイリストファイル(.lst)の読み書き時には UTF-8 に変換して保存・読み込みを行います。

#### [MODIFY] [PlaylistManager.h](file:///D:/ozlab/oztone/src/PlaylistManager.h)
- 引数と戻り値の `std::string` を `std::wstring` に変更します (`Add`, `GetCurrentTrack`, `GetNextTrack`, `SaveToFile`, `LoadFromFile`)。
- メンバ変数の `m_playlist` と `m_playlistSet` の型を `std::wstring` を用いるように変更します。

#### [MODIFY] [PlaylistManager.cpp](file:///D:/ozlab/oztone/src/PlaylistManager.cpp)
- 各メソッドの実装を `std::wstring` 対応に変更します。
- `SaveToFile` では、`WideCharToMultiByte` (CP_UTF8) を使用して `std::wstring` のパスを UTF-8 に変換してファイルに書き込みます。
- `LoadFromFile` では、ファイルから読み込んだ UTF-8 の行を `MultiByteToWideChar` (CP_UTF8) を使用して `std::wstring` に変換してからリストに追加します。

---

### AudioPlayer
miniaudioエンジンに対してワイド文字列のファイルパスを渡せるようにします。

#### [MODIFY] [AudioPlayer.h](file:///D:/ozlab/oztone/src/AudioPlayer.h)
- `Play` メソッドの引数を `std::wstring` に変更します。

#### [MODIFY] [AudioPlayer.cpp](file:///D:/ozlab/oztone/src/AudioPlayer.cpp)
- `Play` メソッド内で `ma_sound_init_from_file_w` を呼び出し、ワイド文字列のパス (`filepath.c_str()`) を渡すように修正します。

---

### TagManager
TagLibに対してワイド文字列のファイルパスを渡せるようにします。

#### [MODIFY] [TagManager.h](file:///D:/ozlab/oztone/src/TagManager.h)
- `Load` メソッドの引数を `std::wstring` に変更します。

#### [MODIFY] [TagManager.cpp](file:///D:/ozlab/oztone/src/TagManager.cpp)
- `Load` メソッド内で、`TagLib::MPEG::File` のコンストラクタに `filepath.c_str()` を渡すように修正します。（Windows版 TagLib では `const wchar_t*` を受け取るコンストラクタが用意されています）。

---

### Application
各マネージャ間の橋渡しを行っている部分で、マルチバイト変換を行わずにワイド文字列のまま処理を行うように修正します。

#### [MODIFY] [Application.cpp](file:///D:/ozlab/oztone/src/Application.cpp)
- `Initialize`, `OnFilesDropped`, `Run`, `PrefetchNextTrack`, `ClearPlaylist` 等において、`PlaylistManager` や `AudioPlayer`、`TagManager` とファイルパスをやり取りする箇所で `.string()` への変換（マルチバイト変換）を行っている処理を取り除き、`.wstring()` のまま引き渡すように修正します。
- 例：`std::string defaultPath = std::filesystem::path(m_config.GetDefaultPlaylistPath()).string();` を `.wstring()` に変更。
- `std::string track = m_playlistManager.GetCurrentTrack();` などを `std::wstring` に変更し、付随する `std::filesystem::path(track).filename().wstring()` などの処理を調整します。

## Verification Plan

### Automated Tests
- CMakeを用いたビルド（`build.bat`）を実行し、コンパイルエラー・リンクエラーが発生しないことを確認します。

### Manual Verification
- Unicode文字（例：'ø'、'♥' など）を含むファイル名のオーディオファイルをアプリケーションにドラッグ＆ドロップし、正常にプレイリストに追加され、再生・タグ取得（タイトル、アーティスト、アルバムアート）ができることを確認します。
- 次の曲、前の曲への移動、およびプレイリストの再ロード（アプリ再起動時のプレイリスト読み込み）が正常に行われることを確認します。
