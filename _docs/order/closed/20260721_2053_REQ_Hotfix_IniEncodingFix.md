##### 作業指示書 REQ: INIファイルの文字コード崩壊（UTF-16 LE BOM化）修正 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
###### 【作業手順（厳守事項）】
本プロンプトはINIファイルの文字コード崩壊（UTF-16 LE BOM化）修正のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
3. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_IniEncodingFix.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
4. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  対象ファイル: `src/ConfigManager.cpp`
*  現在、`ConfigManager::SaveDefaultSettings` 内で `DEFAULT_INI_CONTENT` を `WideCharToMultiByte` を用いて UTF-8 に変換して書き出している処理を **完全廃止** する。
*  Win32 API (`WritePrivateProfileStringW`) が正しくワイド文字で追記・更新できるようにするため、以下の手順で UTF-16 LE 形式（BOM付き）でファイルを出力するようにロジックを改修すること。
    1. `std::ofstream` をバイナリモード (`std::ios::binary`) で開く。
    2. ファイルの先頭に UTF-16 LE の BOM である 2バイト (`0xFF`, `0xFE`) を書き込む。
    3. 既にワイド文字列（`const wchar_t*` または `std::wstring`）である `DEFAULT_INI_CONTENT` の内容を、バイト配列としてそのまま（`sizeof(wchar_t) * length`分）ファイルへ書き込む。
*  これにより、INIファイルが生成された時点で OS が UTF-16 LE と認識するため、以降の `WritePrivateProfileStringW` による上書き保存で Shift-JIS が混入するバグが完全に解消される。

#### 【絶対遵守ルール (Constraints)】
*   **エンコーディングの厳守** : UTF-8での出力やマルチバイト変換は行わず、必ずUTF-16 LE (BOM付き) としてバイナリ書き込みを行うこと。既存の `std::wofstream` などを安易に使うと環境依存の文字コード変換が発生する可能性があるため、必ず `std::ofstream` + `std::ios::binary` + `reinterpret_cast<const char*>` のアプローチで純粋なバイト列として書き出すこと。
