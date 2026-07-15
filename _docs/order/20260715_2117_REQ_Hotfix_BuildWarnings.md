### 作業指示書 REQ: Hotfix ビルドWarningの一掃と型安全性の確保 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`YYYYMMDD_HHMM_RES_Hotfix_BuildWarnings.md`）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. チャットにて「Warningの修正が完了しました。ビルドの確認をお願いします」と報告すること。

#### 【実装要件】
現在ビルド時に発生している大量の C4244 警告（暗黙の型変換によるデータ損失の可能性）を解消し、型安全性を確保する。

*   **要件1: ConfigManager.h のゲッター/セッターの型不一致修正**
    *   `ConfigManager.h` を精査し、`int` 型のメンバ変数に対して `float` を返しているゲッター、またはその逆のゲッターを特定し、戻り値および引数の型をメンバ変数と完全に一致させること。

*   **要件2: `GetDefaultIniValue` のワイド文字列（Unicode）対応**
    *   `src/ConfigManager.cpp` の `GetDefaultIniValue` メソッド内において、`std::string` や `std::istringstream` を使用している箇所を、`DEFAULT_INI_CONTENT` のワイド文字列（`const wchar_t*`）に合わせて `std::wstring` および `std::wistringstream` へ修正し、`char` への暗黙の変換が発生しないよう型を統一すること。

*   **要件3: ConfigManager_Playback.cpp 等での代入時の型不一致修正**
    *   `src/ConfigManager_Playback.cpp` をはじめとする各設定読み込みファイルにて、`LoadOrWriteFloat` から取得した値を `int` 型変数へ代入している箇所、またはその逆の箇所を特定する。
    *   変数の意図（本来 int であるべきか float であるべきか）に合わせて読み込みメソッド（`LoadOrWriteInt` に変える等）を修正するか、明示的な `static_cast` を用いて警告を解消すること。

*   **要件4: 外部ライブラリ (`miniaudio.h`) の警告抑制**
    *   `src/AudioPlayer.cpp` における `#include "miniaudio.h"` のインクルード部分を `#pragma warning` で囲み、外部ライブラリ内部の C4244 警告をプロジェクトのビルドログに出力させないようにする。
    *   実装例:
        ```cpp
        #pragma warning(push)
        #pragma warning(disable: 4244)
        #include "miniaudio.h"
        #pragma warning(pop)
        ```

#### 【絶対遵守ルール (Constraints)】
*   **安全なキャスト**: 値の切り捨てが発生する `static_cast` を行う場合は、それがUI描画上の意図的な丸め処理等であることを確認し、予期せぬデータの破壊を起こさないこと。
