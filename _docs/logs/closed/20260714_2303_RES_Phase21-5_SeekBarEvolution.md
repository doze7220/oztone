# RES:実装計画・作業レポート Phase 21-5: シークバーのオーバーレイ化と完全データ駆動化

## 1. 実装目的
現在シークバー右に表示されている時間表示が10分以上になると改行してしまう問題、および100分以上のMP3などに対応できない問題を根本的に解決する。
時間表示領域を固定幅で確保するのではなく、シークバー本体を最大幅まで拡張し、時間表示テキストをその上にオーバーレイ（右寄せ）して描画する構造へとレイアウト計算を改修する。
また、シークバーの色や不透明度を `ConfigManager` (INIファイル) から完全に制御できるようにし、ステルスUIとしての背景透過や明度調整をデータ駆動で実現する。

## 2. アーキテクチャ設計
### 要件1: タイムカウントのオーバーレイ化とシークバーの最大拡張
- `src/LayoutCalculator.cpp` におけるシークバーレイアウト計算（`CalculateSeekBarLayout`等）を改修し、シークバー本体の幅計算から時間表示領域の確保（`TimeAreaWidth`分の引き算）を廃止する。これによりシークバーは左右マージンいっぱいに拡張される。
- 時間テキストの描画矩形は、拡張されたシークバーの領域内に右寄せで配置されるように計算ロジックを変更する。
- `src/ConfigManager.h` および関連実装に `TimeMarginRight` (デフォルト: `5.0f`等) を追加し、シークバー右端とテキスト間に適切な余白を設定できるようにする。

### 要件2: シークバーのカラー＆アルファの完全データ駆動化
- `src/ConfigManager.h` に設定項目を追加する: `SeekBarFgColor` (デフォルト: `"#E0E0E0"` 等), `SeekBarBgColor` (デフォルト: `"#FFFFFF"` 等), `FgOpacity` (デフォルト: `0.8f` 等)。既存の `BgOpacity` は維持。
- `src/ConfigManager_Playback.cpp` にて、INIファイルの `[Layout_SeekBar]` セクションとの入出力ロジック（Getter追加、`LoadFromIni`、`SaveToIni`修正）を実装する。
- `src/ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` にこれらのデフォルト設定を追記し、Single Source of Truth を維持する。
- `src/Widget_SeekBar.cpp` でのブラシ生成（`CreateResources` 又は `Draw` 内の処理）において、ハードコードされた白系色から `ConfigManager` を経由した色取得へ変更する。色は `WidgetCommon::HexToColorF` で変換し、描画時に `FgOpacity` や `BgOpacity` を適用して不透明度を制御する構造とする。

## 3. 実装タスクリスト
[x] タスク1: ConfigManagerへの設定値追加とINIデフォルト更新
    - `ConfigManager.h` に `SeekBarFgColor`, `SeekBarBgColor`, `FgOpacity`, `TimeMarginRight` を追加。
    - `ConfigManager_Playback.cpp` でINIの読み書きロジックを実装。
    - `ConfigManager_DefaultIni.h` の `DEFAULT_INI_CONTENT` を更新。
[x] タスク2: LayoutCalculator レイアウト計算の改修 (最大拡張・オーバーレイ化)
    - `LayoutCalculator.cpp` でシークバー幅を最大化するように修正。
    - 時間テキスト描画用矩形を右寄せオーバーレイに変更し、`TimeMarginRight` を適用する。
[x] タスク3: SeekBarWidget カラー描画のデータ駆動化
    - `Widget_SeekBar.cpp` の描画処理にて、ハードコード色を排除。
    - INIから取得した色（`HexToColorF`で変換）とAlpha値を適用してブラシを生成・描画する。

## 4. 詳細作業内容
### タスク1: ConfigManagerへの設定値追加とINIデフォルト更新
    - `ConfigManager.h` に `GetSeekBarFgColor`, `GetSeekBarBgColor`, `GetSeekBarFgOpacity`, `GetSeekBarTimeMarginRight` のgetterメソッドおよび対応するメンバ変数を追加しました。
    - `ConfigManager_Playback.cpp` の `LoadPlaybackSettings` にて、INIファイルの `[Layout_SeekBar]` セクションから新規に追加された項目 (`FgColor`, `BgColor`, `FgOpacity`, `TimeMarginRight`) を読み込むように修正しました。
    - `ConfigManager_DefaultIni.h` のデフォルトINI定義に上記設定のデフォルト値を追加しました。

### タスク2: LayoutCalculator レイアウト計算の改修 (最大拡張・オーバーレイ化)
    - `LayoutCalculator.cpp` の `CalculateSeekBarLayout` において、`barAreaWidth` の計算から `TimeAreaWidth` の減算を廃止し、シークバー本体が余白を除いた最大幅に拡張されるように改修しました。
    - 拡張された領域上で時間テキストが右寄せで描画されるよう、`layout.textOrigin` の計算を `startX + totalWidth - config->GetSeekBarTimeMarginRight() - layout.textMaxWidth` とし、描画矩形を適切に配置するよう修正しました。

### タスク3: SeekBarWidget カラー描画のデータ駆動化
    - `Widget_SeekBar.cpp` の `Draw` メソッド内で使用するブラシ (`m_seekBarBgBrush`, `m_seekBarFgBrush`) に対し、`WidgetCommon::HexToColorF` を用いて ConfigManager から取得したカラー情報をセットする処理を追加しました。
    - また、不透明度に関しても `FgOpacity` と `BgOpacity` をそれぞれ適用し、UI全体のトーン＆マナーに沿ったデータ駆動描画を実現しました。

<!-- 以下は実装フェーズで不具合や追加修正が発生した場合のみ追記すること -->
特に問題なく予定通り実装を完了しました。

### 追加修正: Hot-reload時の設定反映不具合の修正
- ユーザーからの報告により「INIを直接編集しても設定が反映されない」問題を確認しました。
- 原因として、Windows API (`GetPrivateProfileStringW`) がINIファイルの内容を内部的にキャッシュしているため、外部エディタの変更を読み込めていないことが判明しました。
- `ConfigManager.cpp` の `LoadSettings()` 実行時に `WritePrivateProfileStringW(NULL, NULL, NULL, m_iniFilePath.c_str())` を呼び出してキャッシュを強制的にフラッシュする処理を追加し、即座に設定値が反映されるよう修正しました。

### HOTFIX: 時間テキストカラーの独立化 (Phase 21-5 Hotfix)
- **原因と背景**: Phase 21-5 の実装により、時間テキストの描画時にシークバー本体の色 (`FgColor` / `FgOpacity`) を流用してしまっていたため、シークバー本体の設定を変更すると時間テキストの視認性が低下する（または意図しない色になる）問題がありました。
- **対応内容**:
  - `ConfigManager` に時間テキスト専用の設定値 `TextColor` と `TextOpacity` (デフォルト: `#FFFFFF` / `1.0`) を追加し、INIファイルからの読み書きに対応しました。
  - `Widget_SeekBar.cpp` の描画処理 (`Draw` メソッド) にて、時間テキスト描画用のブラシ (`m_textBrush`) に対して `TextColor` と `TextOpacity` を適用するように変更し、シークバー本体とテキストの色設定を完全に独立化しました。

### HOTFIX: TimeAreaWidthの完全パージ (Phase 21-5 Hotfix)
- **原因と背景**: 時間テキストのシークバー上へのオーバーレイ化に伴い、時間表示領域を事前に確保するためのパラメータ `TimeAreaWidth` が不要となりました。
- **対応内容**:
  - `ConfigManager.h`, `ConfigManager_Playback.cpp`, `ConfigManager_DefaultIni.h` から `TimeAreaWidth` の変数、パース処理、INIデフォルト値を完全に削除しました。
  - `LayoutCalculator.cpp` における時間テキスト描画矩形 (`timeRect`) の計算において、`GetSeekBarTimeAreaWidth()` を用いていた部分を削除し、右寄せ描画（`DWRITE_TEXT_ALIGNMENT_TRAILING`）がシークバー全体の幅で機能するように、矩形の幅を `totalWidth - margin` として計算するようクリーンアップしました。

