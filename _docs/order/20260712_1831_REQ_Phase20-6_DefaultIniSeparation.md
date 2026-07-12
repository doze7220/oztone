### 作業指示書 REQ: Phase 20-6 Task 1 : DEFAULT_INI_CONTENT の分離
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは DEFAULT_INI_CONTENT の外部ファイル化の「実装実行」である。直ちに以下の【実装要件】に従ってコードとドキュメントの修正を実行すること。
2. 作業完了後、新規の作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase20-6_DefaultIniSeparation.md）を作成し、詳細作業内容を記録すること。
3. チャットにて「DEFAULT_INI_CONTENTの分離(Phase 20-6)がすべて完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
`ConfigManager.cpp` にハードコードされている約500行の `DEFAULT_INI_CONTENT` を、AI-IDEのコンテキスト圧迫を防ぐため専用のヘッダファイルへ完全に分離する。

*   **要件1: 新規ヘッダファイルの作成 (`src/ConfigManager_DefaultIni.h`)**
    *   `src/ConfigManager_DefaultIni.h` を新規作成し、インクルードガード (`#pragma once`) を記述する。
    *   `ConfigManager.cpp` にある `constexpr const char* DEFAULT_INI_CONTENT = R"( ... )";` の定義ブロックを丸ごとこのヘッダファイルへ移動する。

*   **要件2: `ConfigManager.cpp` の修正**
    *   `ConfigManager.cpp` から `DEFAULT_INI_CONTENT` の定義を削除し、ファイルの先頭付近に `#include "ConfigManager_DefaultIni.h"` を追加してコンパイルが通るようにする。

#### 【絶対遵守ルール (Constraints)】
*   **内容の維持**: `DEFAULT_INI_CONTENT` の文字列の中身は1文字たりとも変更してはならない。単純なファイルの移動（分離）のみを行うこと。
