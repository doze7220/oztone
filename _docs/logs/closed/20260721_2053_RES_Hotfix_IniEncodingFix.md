# RES:HOTFIX作業レポート: INIファイルの文字コード崩壊（UTF-16 LE BOM化）修正

## 1. 実装目的
`ConfigManager` による INI ファイル（`OZtone.ini`）生成時、これまでは UTF-8 で出力されていたため、後続処理である Win32 API (`WritePrivateProfileStringW`) がワイド文字列で追記・更新を行う際に文字コードが崩れ、Shift-JIS などの意図しないエンコーディングが混入する問題（文字コード崩壊）が発生していた。これを解決するため、INI ファイル生成段階から確実に OS と API に適合する形式で書き出せるように修正する。

## 2. 調査内容
* `ConfigManager::SaveDefaultSettings` において、`WideCharToMultiByte` を用いて UTF-8 に変換してファイル出力する処理が文字コード崩壊の原因となっていた。
* `WritePrivateProfileStringW` は BOM なしの UTF-8 ファイルに対して追記を行った際、環境依存のエンコーディング（Shift-JIS など）を適用してしまい、ファイルが破壊される。
* このため、`std::ofstream` をバイナリモード (`std::ios::binary`) で開き、先頭に UTF-16 LE の BOM (`0xFF`, `0xFE`) を直接書き込んだ後、ワイド文字列 (`DEFAULT_INI_CONTENT`) のバイト列をそのまま追記するアプローチへ変更することで、文字コード崩壊を未然に防ぐことができる。

## 3. 対象ファイル
* `src/ConfigManager.cpp`
* `PROJECT_ARCHITECTURE.md` (アーキテクチャ資料追記)

## 4. 実装タスクリスト
- [x] タスク1: `src/ConfigManager.cpp` の修正 - `SaveDefaultSettings` メソッド内の出力ロジックを UTF-16 LE (BOM付き) のバイナリ出力へ変更。
- [x] タスク2: `PROJECT_ARCHITECTURE.md` の修正 - `ConfigManager` の項に、ファイル生成時の文字コード（UTF-16 LE BOM付き）に関する仕様を追記。

## 5. 詳細作業内容
* タスク1: `src/ConfigManager.cpp` の修正
    - `WideCharToMultiByte` を用いた UTF-8 出力ロジックを完全廃止。
    - `std::ofstream` に `std::ios::binary` オプションを指定し、BOM (`0xFF`, `0xFE`) を2バイト書き込む処理を追加。
    - `DEFAULT_INI_CONTENT` の文字列を `std::wstring` とし、`reinterpret_cast<const char*>` を用いて直接バイト列としてバイナリ書き込みを行うように修正。
* タスク2: `PROJECT_ARCHITECTURE.md` の修正
    - `ConfigManager` クラスの説明文に、「初期ファイル生成時は `WritePrivateProfileStringW` による後続の書き込みとの整合性を保つため、UTF-16 LE (BOM付き) 形式でバイナリ出力される」旨の仕様を追記。
