# 20260705_0944_RES_Phase15_IconMenu_RemoveTrack

## 実装目的
Phase 13にてプレイリストUIのツールバーに「再生曲の削除」機能が統合されたため、役割が重複するロゴ拡張メニューの「曲削除（🗑️）」項目を完全に廃止し、ストイックなUIを保つ。

## 変更ファイル
* `src/Window.h`
  * `ID_LOGO_CLEAR` 定数を削除。
* `src/Window.cpp`
  * `m_logoMenuItems` の初期化リストから該当アイコン項目を削除。
  * `WindowProc` 内のクリックイベント（`WM_LBUTTONDOWN`）における該当コマンドに対する分岐を削除。
* `PROJECT_ARCHITECTURE.md`
  * 「ロゴ拡張メニューの機能」から「再生中の曲削除」の記述を削除。

## 懸念点・特記事項
* `src/Widgets.cpp` のホバーテキスト表示については、配列ベースの動的参照 (`hoveredItem.labelText`) に依存する設計となっており、コマンドIDによる静的な分岐が存在しなかったため、コードの修正は不要でした。
