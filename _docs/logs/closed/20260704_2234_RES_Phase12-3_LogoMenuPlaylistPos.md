# 作業レポート: Phase 12-3 ロゴ拡張メニューへの「プレイリスト左右配置トグル」追加計画

## 概要
ロゴ拡張メニュー（LogoMenuWidget）に第6のボタンを追加し、プレイリストの表示位置（左・右）を動的に切り替える機能（PlaylistPositionトグル）を実装するための計画書。
既存の配列ベースのメニュー管理機能（`m_logoMenuItems`）を利用することで、UI描画と状態管理の責務を分離したまま容易に拡張可能である。

## アーキテクチャ設計・実装方針

1. **メニュー項目の拡張 (Window)**
   - `Window.h` に新しいコマンドID `ID_LOGO_PLAYLIST_POS` (例: 3006) を定義。
   - `Window::Window` コンストラクタにおいて、`m_logoMenuItems` の末尾に6つ目の要素として追加。
     - アイコン: `↔️` または `⇄` (UNICODE)
     - ツールチップ(デフォルト): `プレイリストの配置場所切り替え`
     - トグルフラグは `false` とし、値の反転管理自体は `ConfigManager` に委譲。

2. **ホバーテキストの動的表示 (LogoMenuWidget)**
   - `LogoMenuWidget::Draw` 内のホバー処理（`textToDraw` の決定ロジック）を修正。
   - `hoveredItem.commandId == Window::ID_LOGO_PLAYLIST_POS` の場合、`config->GetPlaylistPosition()` の戻り値に応じて表示文字列を動的に設定。
     - 0 (左) の場合: `PLAYLIST POS: LEFT`
     - 1 (右) の場合: `PLAYLIST POS: RIGHT`

3. **設定の動的変更と保存 (ConfigManager / Window)**
   - **設定管理 (ConfigManager)**: 現在の `ConfigManager` には `GetPlaylistPosition()` は存在するが、セッターがないため、`SetPlaylistPosition(int position)` メソッドを新設する。
   - 内部で `m_playlistPosition` を更新し、同時に `WritePrivateProfileStringW` で `OZtone.ini` の `[Layout_Playlist]` セクションにある `PlaylistPosition` へ即時保存する。
   - **イベント処理 (Window)**: `Window::WindowProc` の `WM_LBUTTONDOWN` 内（ロゴメニュークリック処理）に `ID_LOGO_PLAYLIST_POS` のハンドラを追加。
   - 現在の `GetPlaylistPosition()` の値を反転（0なら1、1なら0）させ、`SetPlaylistPosition()` を呼び出して状態を適用する。

## タスクリスト

- [x] `Window.h` の修正
  - `Window` クラス内に `static constexpr UINT ID_LOGO_PLAYLIST_POS = 3006;` を追加する。
- [x] `ConfigManager.h` の修正
  - `void SetPlaylistPosition(int position);` の宣言を追加する。
- [x] `ConfigManager.cpp` の修正
  - `ConfigManager::SetPlaylistPosition(int position)` の実装を追加し、`m_playlistPosition` の更新と `WritePrivateProfileStringW` による保存処理を記述する。
- [x] `Window.cpp` (コンストラクタ) の修正
  - `Window::Window` 内の `m_logoMenuItems` 初期化リスト末尾に `{ID_LOGO_PLAYLIST_POS, L"↔️", false, false, L"プレイリストの配置場所切り替え"}` を追加する。
- [x] `Window.cpp` (WM_LBUTTONDOWN) の修正
  - `WindowProc` 内の `WM_LBUTTONDOWN` におけるロゴメニュークリック処理に、`else if (item.commandId == ID_LOGO_PLAYLIST_POS)` の分岐を追加。
  - 値を取得して反転させ、`m_config->SetPlaylistPosition(newPos)` を呼び出すロジックを実装する。
- [x] `Widgets.cpp` の修正
  - `LogoMenuWidget::Draw` において、`hoveredItem.commandId == Window::ID_LOGO_VISUALIZER` の `if` 文に `else if (hoveredItem.commandId == Window::ID_LOGO_PLAYLIST_POS)` の分岐を追加。
  - `config->GetPlaylistPosition()` の値に基づき、`textToDraw` に `L"PLAYLIST POS: LEFT"` または `L"PLAYLIST POS: RIGHT"` を代入する処理を実装する。

## 4. 詳細作業内容
* **ConfigManagerの拡張**:
  * `ConfigManager.h` に `void SetPlaylistPosition(int position);` を追加しました。
  * `ConfigManager.cpp` に同メソッドの実装を追加し、`m_playlistPosition` メンバー変数の更新と、`WritePrivateProfileStringW` を使用して `OZtone.ini` の `[Layout_Playlist]` セクションにある `PlaylistPosition` への永続化を実装しました。
* **Windowクラスの拡張 (UI統合)**:
  * `Window.h` に新しいコマンドID `ID_LOGO_PLAYLIST_POS = 3006;` を定義しました。
  * `Window.cpp` のコンストラクタにおいて、`m_logoMenuItems` リストの末尾に設定を切り替えるためのトグル項目 `{ID_LOGO_PLAYLIST_POS, L"↔️", false, false, L"プレイリストの配置場所切り替え"}` を追加しました。
  * `Window.cpp` の `WM_LBUTTONDOWN` ハンドラ内に `ID_LOGO_PLAYLIST_POS` 用の分岐を追加し、クリックされるたびに現在位置設定 (0 or 1) を反転して `SetPlaylistPosition()` を呼び出すロジックを実装しました。
* **LogoMenuWidgetの描画拡張 (ホバーテキスト)**:
  * `Widgets.cpp` のホバーテキスト決定ロジックに分岐を追加し、`GetPlaylistPosition()` の値が 0 なら `PLAYLIST POS: LEFT`、それ以外(1) なら `PLAYLIST POS: RIGHT` の文字列を動的に設定するように修正しました。
