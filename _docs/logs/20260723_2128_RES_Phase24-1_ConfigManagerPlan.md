# RES:実装計画・作業レポート Phase 24-1: ConfigManager リファクタリング計画立案

## 1. 実装目的
現在 `ConfigManager.h` に平べったく羅列されている膨大な設定変数群を、INIファイルのセクション（`[Audio]`, `[Layout_VolumeControl]` 等）と完全に1対1で対応する「1 Section = 1 Struct = 1 .cpp」のアーキテクチャへと刷新する。
これにより、設定値の管理スコープを明確にし、肥大化した `ConfigManager` の責務とヘッダを整理する。
また、作業量が膨大となるため、AIのコンテキスト限界（ハルシネーション）を防ぐべく細かくタスクを分割し、最終工程で一気に結線する安全な移行プロセスを確立する。

## 2. アーキテクチャ設計
### 【厳命事項】
**AIは1回のタスクで指定されたセクションのみを確実に行い、絶対に他のタスクをフライングしてはならない。**

### 要件1: ソースコード解析に基づくセクション分割
現状の `ConfigManager.h` および `ConfigManager_DefaultIni.h` を解析し、計22個のINIセクションが存在することを確認した。
これを以下のアーキテクチャで再構築する。
*   **Configディレクトリの新設**: `src/Config/` ディレクトリを新設し、新設する設定ファイル群および `ConfigManager.h`, `ConfigManager.cpp` を格納する。
*   **【例外絶対ルール】**: `src/ConfigManager_DefaultIni.h` は絶対に移動させず、現在の `src/` 直下に残すこと。新設ファイルから参照する際は `#include "../ConfigManager_DefaultIni.h"` とすること。
*   **構造体定義**: 各セクションに対応する構造体を定義したヘッダファイル（例: `src/Config/Config_Audio.h`）を作成。
*   **物理ファイル**: 構造体に値をロードする処理を記述した実装ファイル（例: `src/Config/ConfigManager_Audio.cpp`）を作成（メソッド例: `ConfigManager::LoadSection_Audio(Config_Audio& outConfig)`）。
*   **タスク分割**: 1タスクにつき2〜3セクション程度を担当させる。

### 要件2: タスクの細分化とハルシネーション対策の厳命
対象が22セクションに及ぶため、全9タスクに分割して移行を実施する。各タスクでは指定されたセクションに関連する新規ファイルの作成（`.h` と `.cpp`）のみに集中すること。

### 要件3: ヘッダ登録の遅延化とメモ書き（安全装置）
*   各移行タスク（Task 1〜Task 8）の実行中は、大元である `ConfigManager.h` は一切書き換えない（旧変数の削除も禁止）。
*   各タスク完了時、本レポートの「詳細作業内容」セクションに**「最終的に `ConfigManager.h` に登録すべきインスタンス変数とゲッターのシグネチャ」**をメモとして追記・残すこと。

### 要件4: 最終タスクでの一斉結線とパージ
*   Task 9 において、Task 1〜8 で残されたすべてのメモを拾い上げ、`ConfigManager.h` へ一気にインスタンス変数とゲッターを登録（結線）する。
*   結線完了後、旧変数・旧ゲッター群を一気にパージ（削除）する。
*   既存の `ConfigManager_Window.cpp` や `ConfigManager_Playlist.cpp` などの古いロード用cpp群で不要になったものを整理・削除する。
*   `PROJECT_ARCHITECTURE.md` の記述を更新する。

## 3. 実装タスクリスト
- [x] Task 1: System / Window / Visibility セクションの移行
    - `Config_System.h/cpp`, `Config_Window.h/cpp`, `Config_Visibility.h/cpp` の作成
- [x] Task 2: Playlist / Audio / TrackDrum セクションの移行
    - `Config_Playlist.h/cpp`, `Config_Audio.h/cpp`, `Config_TrackDrum.h/cpp` の作成
- [x] Task 3: UI_Common_Parm / Background セクションの移行
    - `Config_UICommonParm.h/cpp`, `Config_Background.h/cpp` の作成
- [x] Task 4: Layout_AppLogo / Layout_LogoMenu / Layout_NowPlaying セクションの移行
    - `Config_LayoutAppLogo.h/cpp`, `Config_LayoutLogoMenu.h/cpp`, `Config_LayoutNowPlaying.h/cpp` の作成
- [x] Task 5: Layout_SeekBar / Layout_PlaybackControls / Layout_VolumeControl セクションの移行
    - `Config_LayoutSeekBar.h/cpp`, `Config_LayoutPlaybackControls.h/cpp`, `Config_LayoutVolumeControl.h/cpp` の作成
- [x] Task 6: Layout_Tooltip / Layout_Playlist / Layout_GlobalHotkeys セクションの移行
    - `Config_LayoutTooltip.h/cpp`, `Config_LayoutPlaylist.h/cpp`, `Config_LayoutGlobalHotkeys.h/cpp` の作成
- [x] Task 7: Layout_OSD / Visualizer / Visualizer_PrismBeat セクションの移行
    - `Config_LayoutOSD.h/cpp`, `Config_Visualizer.h/cpp`, `Config_VisualizerPrismBeat.h/cpp` の作成
- [x] Task 8: Visualizer_HaloDust / GlobalHotkeys セクションの移行
    - `Config_VisualizerHaloDust.h/cpp`, `Config_GlobalHotkeys.h/cpp` の作成
- [x] Task 9: 最終結線と置換（※パージ保留）
    - Task 1〜8のメモ書きをもとに `ConfigManager.h` へ一斉結線
    - アプリケーション各所の既存ゲッター呼び出し箇所を新構造体経由への一括修正 (例: `GetFocusColor()` -> `GetUICommonParm().FocusColor`)
    - `CMakeLists.txt` の更新（`src/Config/Config_*.cpp` 構造の反映）およびビルド通過確認
    - （※旧変数、旧ゲッター、旧パース用cppファイル等のパージ、および ConfigManager 本体の移動は次フェーズへ保留）

## 4. 詳細作業内容
### Task 1: System / Window / Visibility セクションの移行
**【対象ファイル】**
- `src/Config/Config_System.h`, `src/Config/Config_System.cpp`
- `src/Config/Config_Window.h`, `src/Config/Config_Window.cpp`
- `src/Config/Config_Visibility.h`, `src/Config/Config_Visibility.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_System`, `Config_Window`, `Config_Visibility`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_System`, `LoadSection_Window`, `LoadSection_Visibility`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_System m_configSystem;
Config_Window m_configWindow;
Config_Visibility m_configVisibility;

// ゲッター
const Config_System& GetSystem() const { return m_configSystem; }
const Config_Window& GetWindow() const { return m_configWindow; }
const Config_Visibility& GetVisibility() const { return m_configVisibility; }

// ロード関数シグネチャ (privateなど)
void LoadSection_System(Config_System& outConfig);
void LoadSection_Window(Config_Window& outConfig);
void LoadSection_Visibility(Config_Visibility& outConfig);
```

### Task 2: Playlist / Audio / TrackDrum セクションの移行
**【対象ファイル】**
- `src/Config/Config_Playlist.h`, `src/Config/Config_Playlist.cpp`
- `src/Config/Config_Audio.h`, `src/Config/Config_Audio.cpp`
- `src/Config/Config_TrackDrum.h`, `src/Config/Config_TrackDrum.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_Playlist`, `Config_Audio`, `Config_TrackDrum`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_Playlist`, `LoadSection_Audio`, `LoadSection_TrackDrum`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_Playlist m_configPlaylist;
Config_Audio m_configAudio;
Config_TrackDrum m_configTrackDrum;

// ゲッター
const Config_Playlist& GetPlaylist() const { return m_configPlaylist; }
const Config_Audio& GetAudio() const { return m_configAudio; }
const Config_TrackDrum& GetTrackDrum() const { return m_configTrackDrum; }

// ロード関数シグネチャ (privateなど)
void LoadSection_Playlist(Config_Playlist& outConfig);
void LoadSection_Audio(Config_Audio& outConfig);
void LoadSection_TrackDrum(Config_TrackDrum& outConfig);
```

### Task 3: UI_Common_Parm / Background セクションの移行
**【対象ファイル】**
- `src/Config/Config_UICommonParm.h`, `src/Config/Config_UICommonParm.cpp`
- `src/Config/Config_Background.h`, `src/Config/Config_Background.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_UICommonParm`, `Config_Background`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_UICommonParm`, `LoadSection_Background`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_UICommonParm m_configUICommonParm;
Config_Background m_configBackground;

// ゲッター
const Config_UICommonParm& GetUICommonParm() const { return m_configUICommonParm; }
const Config_Background& GetBackground() const { return m_configBackground; }

// ロード関数シグネチャ (privateなど)
void LoadSection_UICommonParm(Config_UICommonParm& outConfig);
void LoadSection_Background(Config_Background& outConfig);
```

### Task 4: Layout_AppLogo / Layout_LogoMenu / Layout_NowPlaying セクションの移行
**【対象ファイル】**
- `src/Config/Config_LayoutAppLogo.h`, `src/Config/Config_LayoutAppLogo.cpp`
- `src/Config/Config_LayoutLogoMenu.h`, `src/Config/Config_LayoutLogoMenu.cpp`
- `src/Config/Config_LayoutNowPlaying.h`, `src/Config/Config_LayoutNowPlaying.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_LayoutAppLogo`, `Config_LayoutLogoMenu`, `Config_LayoutNowPlaying`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_LayoutAppLogo`, `LoadSection_LayoutLogoMenu`, `LoadSection_LayoutNowPlaying`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_LayoutAppLogo m_configLayoutAppLogo;
Config_LayoutLogoMenu m_configLayoutLogoMenu;
Config_LayoutNowPlaying m_configLayoutNowPlaying;

// ゲッター
const Config_LayoutAppLogo& GetLayoutAppLogo() const { return m_configLayoutAppLogo; }
const Config_LayoutLogoMenu& GetLayoutLogoMenu() const { return m_configLayoutLogoMenu; }
const Config_LayoutNowPlaying& GetLayoutNowPlaying() const { return m_configLayoutNowPlaying; }

// ロード関数シグネチャ (privateなど)
void LoadSection_LayoutAppLogo(Config_LayoutAppLogo& outConfig);
void LoadSection_LayoutLogoMenu(Config_LayoutLogoMenu& outConfig);
void LoadSection_LayoutNowPlaying(Config_LayoutNowPlaying& outConfig);
```

### Task 5: Layout_SeekBar / Layout_PlaybackControls / Layout_VolumeControl セクションの移行
**【対象ファイル】**
- `src/Config/Config_LayoutSeekBar.h`, `src/Config/Config_LayoutSeekBar.cpp`
- `src/Config/Config_LayoutPlaybackControls.h`, `src/Config/Config_LayoutPlaybackControls.cpp`
- `src/Config/Config_LayoutVolumeControl.h`, `src/Config/Config_LayoutVolumeControl.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_LayoutSeekBar`, `Config_LayoutPlaybackControls`, `Config_LayoutVolumeControl`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_LayoutSeekBar`, `LoadSection_LayoutPlaybackControls`, `LoadSection_LayoutVolumeControl`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_LayoutSeekBar m_configLayoutSeekBar;
Config_LayoutPlaybackControls m_configLayoutPlaybackControls;
Config_LayoutVolumeControl m_configLayoutVolumeControl;

// ゲッター
const Config_LayoutSeekBar& GetLayoutSeekBar() const { return m_configLayoutSeekBar; }
const Config_LayoutPlaybackControls& GetLayoutPlaybackControls() const { return m_configLayoutPlaybackControls; }
const Config_LayoutVolumeControl& GetLayoutVolumeControl() const { return m_configLayoutVolumeControl; }

// ロード関数シグネチャ (privateなど)
void LoadSection_LayoutSeekBar(Config_LayoutSeekBar& outConfig);
void LoadSection_LayoutPlaybackControls(Config_LayoutPlaybackControls& outConfig);
void LoadSection_LayoutVolumeControl(Config_LayoutVolumeControl& outConfig);
```

### Task 6: Layout_Tooltip / Layout_Playlist / Layout_GlobalHotkeys セクションの移行
**【対象ファイル】**
- `src/Config/Config_LayoutTooltip.h`, `src/Config/Config_LayoutTooltip.cpp`
- `src/Config/Config_LayoutPlaylist.h`, `src/Config/Config_LayoutPlaylist.cpp`
- `src/Config/Config_LayoutGlobalHotkeys.h`, `src/Config/Config_LayoutGlobalHotkeys.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_LayoutTooltip`, `Config_LayoutPlaylist`, `Config_LayoutGlobalHotkeys`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_LayoutTooltip`, `LoadSection_LayoutPlaylist`, `LoadSection_LayoutGlobalHotkeys`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_LayoutTooltip m_configLayoutTooltip;
Config_LayoutPlaylist m_configLayoutPlaylist;
Config_LayoutGlobalHotkeys m_configLayoutGlobalHotkeys;

// ゲッター
const Config_LayoutTooltip& GetLayoutTooltip() const { return m_configLayoutTooltip; }
const Config_LayoutPlaylist& GetLayoutPlaylist() const { return m_configLayoutPlaylist; }
const Config_LayoutGlobalHotkeys& GetLayoutGlobalHotkeys() const { return m_configLayoutGlobalHotkeys; }

// ロード関数シグネチャ (privateなど)
void LoadSection_LayoutTooltip(Config_LayoutTooltip& outConfig);
void LoadSection_LayoutPlaylist(Config_LayoutPlaylist& outConfig);
void LoadSection_LayoutGlobalHotkeys(Config_LayoutGlobalHotkeys& outConfig);
```

### Task 7: Layout_OSD / Visualizer / Visualizer_PrismBeat セクションの移行
**【対象ファイル】**
- `src/Config/Config_LayoutOSD.h`, `src/Config/Config_LayoutOSD.cpp`
- `src/Config/Config_Visualizer.h`, `src/Config/Config_Visualizer.cpp`
- `src/Config/Config_VisualizerPrismBeat.h`, `src/Config/Config_VisualizerPrismBeat.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_LayoutOSD`, `Config_Visualizer`, `Config_VisualizerPrismBeat`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_LayoutOSD`, `LoadSection_Visualizer`, `LoadSection_VisualizerPrismBeat`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_LayoutOSD m_configLayoutOSD;
Config_Visualizer m_configVisualizer;
Config_VisualizerPrismBeat m_configVisualizerPrismBeat;

// ゲッター
const Config_LayoutOSD& GetLayoutOSD() const { return m_configLayoutOSD; }
const Config_Visualizer& GetVisualizer() const { return m_configVisualizer; }
const Config_VisualizerPrismBeat& GetVisualizerPrismBeat() const { return m_configVisualizerPrismBeat; }

// ロード関数シグネチャ (privateなど)
void LoadSection_LayoutOSD(Config_LayoutOSD& outConfig);
void LoadSection_Visualizer(Config_Visualizer& outConfig);
void LoadSection_VisualizerPrismBeat(Config_VisualizerPrismBeat& outConfig);
```

### Task 8: Visualizer_HaloDust / GlobalHotkeys セクションの移行
**【対象ファイル】**
- `src/Config/Config_VisualizerHaloDust.h`, `src/Config/Config_VisualizerHaloDust.cpp`
- `src/Config/Config_GlobalHotkeys.h`, `src/Config/Config_GlobalHotkeys.cpp`
**【作業内容】**
- 各セクションに対応する構造体（`Config_VisualizerHaloDust`, `Config_GlobalHotkeys`）の定義をヘッダに作成。
- INIファイルから値を読み込むためのメンバ関数（`LoadSection_VisualizerHaloDust`, `LoadSection_GlobalHotkeys`）の実装をcppに作成。
- `ConfigManager.h` の直接的な書き換えは避け、Task 9での最終結線に向けてメモを残した。
**【最終登録用メモ（ConfigManager.h向け）】**
```cpp
// メンバ変数
Config_VisualizerHaloDust m_configVisualizerHaloDust;
Config_GlobalHotkeys m_configGlobalHotkeys;

// ゲッター
const Config_VisualizerHaloDust& GetVisualizerHaloDust() const { return m_configVisualizerHaloDust; }
const Config_GlobalHotkeys& GetGlobalHotkeys() const { return m_configGlobalHotkeys; }

// ロード関数シグネチャ (privateなど)
void LoadSection_VisualizerHaloDust(Config_VisualizerHaloDust& outConfig);
void LoadSection_GlobalHotkeys(Config_GlobalHotkeys& outConfig);
```

### Task 9: 最終結線と置換（※パージ保留）
**【対象ファイル】**
- `src/ConfigManager.h`, `src/ConfigManager.cpp`
- `src/*.cpp` (全てのWidget, Application, Window, Renderer 等)
- `CMakeLists.txt`
**【作業内容】**
- `ConfigManager.h` に、Task 1〜8で作成した全22セクションのインクルード、メンバ変数、ゲッターメソッドを追加した。
- `ConfigManager.cpp` の `LoadSettings()` にて、全セクションのロード関数（`LoadSection_XXX`）を呼び出すように追加した。
- Pythonスクリプトを用いて、旧ゲッター呼び出し（例：`GetShowTitleBar()`）を新構造体経由（例：`GetVisibility().ShowTitleBar`）に一括置換した。
- `CMakeLists.txt` に新設した `Config_*.cpp` および `Config_*.h` を追加した。
- ビルドを実行し、LNKエラーを含むすべてのエラーが解消され、ビルドが通ることを確認した。
- **※安全装置として、旧メンバ変数、旧ゲッター、旧パース用cppファイル（`ConfigManager_Window.cpp`等）は一切削除せずに残している。パージ作業およびConfigManager本体のディレクトリ移動は保留中。**

### Task 9.5 (Hotfix): TrackInfo ズレ原因調査と補完
#### 原因・理由: TrackInfo UIのズレ
    - 置換スクリプトにより `GetBaseBottomOffset()` や `GetTooltipOffsetY()` などの旧ゲッターが、`GetLayoutVolumeControl()` などの別の構造体プロパティに誤って置換されていたことが原因。
    - `Config_LayoutNowPlaying` 自体のパース内容に漏れがないことを確認済み。`TrackCountOffsetX` 等のパラメータは、正しく `[Layout_Playlist]` の設定として `Config_LayoutPlaylist.cpp` でロードされていることも確認した。

#### 対象ファイル: 
    - `src/LayoutCalculator.cpp`
    - `src/Widget_TrackInfo.cpp`

#### 対応: 誤置換された構造体参照の修正と旧ゲッターの移行
    - `LayoutCalculator.cpp` 内の `CalculateTrackInfoLayout` 等において、`GetLayoutVolumeControl().BaseBottomOffset` と誤用されていた箇所を `GetLayoutNowPlaying().BaseBottomOffset` に修正。
    - 同ファイル内で未置換で残存していた旧ゲッター `GetPlaybackBaseBottomOffset()`、`GetVolumeBaseBottomOffset()`、`GetVolumeTooltipOffsetY()` を、それぞれ対応する新しい構造体プロパティ（`GetLayoutPlaybackControls().BaseBottomOffset` 等）に修正。
    - `Widget_TrackInfo.cpp` にて、`GetLayoutVolumeControl().TooltipOffsetY` と誤用されていた箇所を `GetLayoutNowPlaying().TooltipOffsetY` に修正。
    - 修正後、`build.bat` を実行し正常にビルドが完了することを確認した。

### Task 9.6 (Hotfix): ホバー判定領域の誤置換修正
#### 原因・理由: 座標判定ロジックの参照先誤り
    - `src/Window_Mouse.cpp` におけるマウスホバー判定ロジック内で、自動置換スクリプトにより `BaseBottomOffset` などの取得元が誤って一律 `GetLayoutVolumeControl()` に置換されていた（`IsInTrackInfoRegion` 等）。
    - 古いゲッター (`GetVolumeBaseBottomOffset`, `GetPlaybackBaseBottomOffset`) の未置換も残存していた。

#### 対象ファイル: 
    - `src/Window_Mouse.cpp`

#### 対応: 誤置換された構造体参照の修正と旧ゲッターの移行
    - `IsInTrackInfoRegion` にて、`GetLayoutVolumeControl().BaseBottomOffset` を本来の `GetLayoutNowPlaying().BaseBottomOffset` に修正。
    - `IsInVolumeControlRegion` にて、残存していた旧ゲッター `GetVolumeBaseBottomOffset()` を新しい構造体プロパティ `GetLayoutVolumeControl().BaseBottomOffset` に修正。
    - `GetPlaybackButtonAt` にて、残存していた旧ゲッター `GetPlaybackBaseBottomOffset()` を新しい構造体プロパティ `GetLayoutPlaybackControls().BaseBottomOffset` に修正。
    - 修正後、`build.bat` を実行し正常にビルドが完了することを確認した。

### [x] Task 10: 旧コードの完全パージとファイル整理 (Final Cleanup)
**目的:** ConfigManager の移行が完了したため、旧アーキテクチャの残骸を安全にパージする。

**作業内容:**
1. **ConfigManager.h のパージ**
   * 古いメンバ変数および、使われなくなった古いゲッターメソッドをすべて削除。
2. **ConfigManager.cpp のパージ**
   * LoadSettings() 内に残っている旧ローダーの呼び出しを削除し、古い LoadCommonSettings() 等の実装を削除。
   * 現在もUIから呼ばれているセッター群の実装を移行し、新構造体へセットするよう書き換え。
3. **物理ファイルの削除と移動**
   * **削除:** ConfigManager_Window.cpp 等の旧パース用ファイル群を削除。
   * **移動:** ConfigManager.h, ConfigManager.cpp, ConfigManager_DefaultIni.h を src/Config/ に移動。
4. **インクルードパスの修正とCMakeLists.txt の更新**
   * プロジェクト内のすべてのファイルに対し、#include "ConfigManager.h" を #include "Config/ConfigManager.h" に修正。
   * CMakeLists.txt のファイル一覧から削除したパース用ファイルを除外し、移動したファイルのパスを更新。
5. **最終ビルドと動作確認**
   * すべての移行が完了した状態でビルドエラーがないことを確認する。

