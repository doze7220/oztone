# [RES] Hotfix SmartReset
## 実行完了レポート

指定された【実装要件】に基づき、Hotfixの実装を完了しました。

### 1. `src/Window.cpp` / `src/Window.h`
- `Window`クラスに`m_onResetAllCallback`とそのセッター（`SetResetAllCallback`）を追加しました。
- `WM_COMMAND`の`ID_TRAY_RESET_ALL`処理から、プロセス再起動（`ShellExecuteW` / `WM_CLOSE`）のロジックを削除し、代わりに`m_onResetAllCallback`を呼び出すように変更しました。

### 2. `src/ConfigManager.cpp` / `src/ConfigManager.h`
- `void ResetToDefaults();` を新設し、メモリ上の設定用メンバ変数を最新の`DEFAULT_INI_CONTENT`の初期値に強制的にリセット（同期）する処理を実装しました。
- `ConfigManager`のコンストラクタは`ResetToDefaults()`を呼び出す形にリファクタリングしました。

### 3. `src/Application.cpp` / `src/Application.h`
- `void ResetAllSettings();` を追加し、以下のシーケンスを実装しました：
  1. `m_config.SaveDefaultSettings();`
  2. `m_config.ResetToDefaults();`
  3. `m_config.LoadSettings();`
  4. `m_renderer.ReloadResources();`
  5. ウィンドウサイズを1024x512にリセットし、画面中央に配置する処理。
- `Application::Initialize`内で、`m_window.SetResetAllCallback([this]() { this->ResetAllSettings(); });` を登録しました。

---

## 【レポート】DEFAULT_INI_CONTENTとC++メンバ変数の不整合リスト

`DEFAULT_INI_CONTENT`と、既存の`ConfigManager`のコンストラクタ内で設定されていた変数の間で以下の不整合（不足・過多・値の乖離）が発見されました。今回の`ResetToDefaults()`の実装により、これらはすべて`DEFAULT_INI_CONTENT`の定義に強制同期されるように修正されています。

**1. DEFAULT_INI_CONTENT に「存在しない」が、C++側で使われている変数（過多）**
- `EnableOSD` (`[Layout_OSD]`)
  - C++側では `m_enableOSD = true` として初期化され、`LoadSettings()` でも `GetPrivateProfileIntW` で読み込まれていますが、`DEFAULT_INI_CONTENT` の文字列内には項目自体が存在しません。

**2. DEFAULT_INI_CONTENT に「存在する」が、C++の旧コンストラクタで初期化されていなかった変数（不足）**
- `OsdFadeWait` (`[Layout_OSD]`)
  - INIには `1.5` として存在していましたが、コンストラクタでは代入漏れとなっていました。
- `SkipIconPoints`, `SkipTextFontSize`, `SkipTextOffsetX`, `SkipTextOffsetY`, `SkipTextShadowColor`, `SkipTextShadowOpacity`, `SkipTextShadowShift` (`[Layout_PlaybackControls]`)
  - INIには存在していましたが、コンストラクタでの初期化記述が完全に抜け落ちていました。

**3. INI初期値とC++コンストラクタ初期値の値の乖離（抜粋）**
- `m_volFontFamily`: INIでは `"Courier New"`、旧C++では `"Meiryo"`
- `m_volFontSize`: INIでは `24.0`、旧C++では `14.0`
- `m_ghFontFamily`: INIでは `MS Gothic`、旧C++では `Meiryo`
- その他、位置やオフセットに関する多数の微細な値のズレがありました。
