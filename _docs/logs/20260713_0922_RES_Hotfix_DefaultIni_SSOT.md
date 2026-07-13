# RES:実装計画・作業レポート Phase 19-Hotfix: DefaultIniのSSOT化と最新パラメータ同期

## 1. 実装目的
現在、新規INI生成用のマスターデータである `ConfigManager_DefaultIni.h` 内の `DEFAULT_INI_CONTENT` と、C++側（`ConfigManager` 各種メソッド）にハードコードされた初期値との間でパラメータの二重管理状態が発生している。また、直近のアップデート（Phase 19周辺）で追加された各種パラメータがマスターデータに追記されていない。
本Hotfixでは、最新パラメータをマスターデータへ完全同期させた上で、C++側のハードコードされた初期値を撤廃し、`DEFAULT_INI_CONTENT` を Single Source of Truth (SSOT：真実の単一情報源) として機能させるよう `ConfigManager` のアーキテクチャをリファクタリングする。

## 2. アーキテクチャ設計
### 要件1: 最新パラメータ群の `DEFAULT_INI_CONTENT` への完全同期
    - `ConfigManager_DefaultIni.h` 内の `DEFAULT_INI_CONTENT` に、不足している最新のパラメータ群（Visualizer関連やその他設定値など）を漏れなく追記する。
    - 主な対象パラメータ:
      - `[Visualizer]` セクション: `EnablePreScan`, `HighFreqNoiseThreshold`, `BandGain0`〜`BandGain100` 等
      - `[Visualizer_PrismBeat]` セクション: `PrismLineThickness` 等
      - `[Visualizer_HaloDust]` セクション: `HaloParticleSizeRatio`, `HaloLaserSpawnRate`, `HaloParticleLifeTime` 等
      - その他、最新の実装で追加されたがINIに含まれていないもの

### 要件2: 自前パース関数の新設
    - `ConfigManager.h` および `ConfigManager.cpp` に、`DEFAULT_INI_CONTENT` の文字列テキストから指定セクションとキーに対応する値を抽出するプライベートヘルパー関数 `std::wstring GetDefaultIniValue(const std::wstring& section, const std::wstring& key)` を新設する。
    - この関数は、メモリ上の `DEFAULT_INI_CONTENT` 文字列を行単位で走査し、`[Section]` 宣言をトリガーにフラグを立て、直下の `Key=Value` の行から `Value` 文字列を抽出して返す設計とする。見つからない場合は空文字などを返す。

### 要件3: 初期値のSSOT化（二重管理のパージ）
    - `ConfigManager` 内の `LoadOrWriteInt` / `LoadOrWriteFloat` / `LoadOrWriteString` メソッドのシグネチャを変更し、第3引数であった「ハードコードのデフォルト値」を削除する。
    - 代わりに各 `LoadOrWrite*` 関数内部において、INIファイルに値が存在しなかった場合のフォールバック値として新設した `GetDefaultIniValue(section, key)` を呼び出し、文字列から各型（int/float/wstring）へ変換して返すようロジックを一元化する。
    - コンストラクタやヘッダファイルにおけるメンバ変数の直接初期化（ハードコード）を可能な限りパージし、読み込みは完全に SSOT に依存する形へ統一する。

### 1. タスク一覧と進捗
- [x] **タスク1: `DEFAULT_INI_CONTENT` の完全同期**
  - **内容**: `ConfigManager_DefaultIni.h` 内の `DEFAULT_INI_CONTENT` に、Phase 19周辺で追加された最新のパラメータ群（Visualizer_PrismBeat, Visualizer_HaloDust, HighFreqNoiseThreshold 等）をハードコードされた値と完全に一致するように追記・同期しました。
- [x] **タスク2: パース関数の実装とフェイルファスト**
  - **内容**: `ConfigManager::GetDefaultIniValue(section, key)` を実装し、`DEFAULT_INI_CONTENT` の文字列を `std::istringstream` で動的にパースする仕組みを構築しました。値が存在しない場合は `assert(false && "Missing key in DEFAULT_INI_CONTENT");` によってフェイルファスト（意図的なクラッシュ）を引き起こす安全設計としました。
- [x] **タスク3: ハードコードのパージと委譲**
  - **内容**: `LoadOrWriteInt`, `LoadOrWriteFloat`, `LoadOrWriteString` のシグネチャからデフォルト値の引数を削除し、内部で `GetDefaultIniValue` を呼ぶように変更しました。また、各 `ConfigManager_*.cpp` からハードコードされたデフォルト値の引数を PowerShell スクリプトを用いた一括置換により完全に削除しました。
- [x] **タスク4: ドキュメントの更新**
  - **内容**: `PROJECT_ARCHITECTURE.md` の ConfigManager の説明部分に、設定初期値が `DEFAULT_INI_CONTENT` にSSOT化されていることと、フェイルファスト設計の仕様を追記しました。本ログドキュメントも更新しました。

## 4. 詳細作業内容
### タスク1: `ConfigManager_DefaultIni.h` の更新
    - （完了）
### タスク2: 自前パース関数の実装
    - （完了）
### タスク3: ハードコードのパージ
    - （未実施）
### タスク4: ドキュメント更新
    - （完了）

### 追加の Hotfix (キー欠落に伴うアサーションエラーの修正)
- **問題**: 実装後に `OZtone.exe` を実行した際、`ConfigManager.cpp` の 105行目（`assert(false && "Missing key in DEFAULT_INI_CONTENT");`）にてアサーションエラーが発生し、クラッシュする問題が報告されました。
- **原因**: 抽出スクリプトによる突き合わせの結果、`[Audio]` セクションのパラメータ群 (`DefaultVolume`, `ShuffleMode`, `SkipSeconds`) が `DEFAULT_INI_CONTENT` に追記されていなかったため、フェイルファストが正しく発動して起動をブロックしていました。
- **対応**: `ConfigManager_DefaultIni.h` 内の `DEFAULT_INI_CONTENT` に不足していた `[Audio]` セクションを追記し、マスターデータを完全に補完しました。これによりビルド後の正常起動を確認しました。
