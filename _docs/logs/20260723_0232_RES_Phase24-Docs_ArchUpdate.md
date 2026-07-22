# RES: Phase 24-Docs / アーキテクチャ資料の更新とノイズパージ

## 1. 実装目的
Phase 23以降のアーキテクチャ変更の反映と、`PROJECT_ARCHITECTURE.md` 内に蓄積した「コードを読めば分かるノイズ情報」の徹底削除を行う。資料の目的である「AIの道標・索引」としての純度を高める。

## 2. 作業内容
*   **「ジョグダイヤリング」概念の更新**:
    *   UI/ビジュアル仕様方針の項目を「ジョグダイヤリング (Jog Dialing)」に更新。
    *   ジョグダイヤリングやNext/Prev等の能動的な操作時のみプレイリストが自動センタリングされ、自動再生時はスクロール位置を維持する設計方針を明記。
*   **FileManagerへのI/O責務集約**:
    *   FileManagerに一時的なファイルロック競合を回避するリトライ機構がカプセル化されている旨を追記。
    *   TrackAnalyzerおよびThumbCacherから個別のリトライ処理に関する記述を削除。
*   **ノイズ情報の徹底パージ**:
    *   ConfigManagerから `ThumbnailJpegQuality`, `PlaylistThumbSize`, `PlaylistThumbOffsetX` 等のパラメータキー名の羅列を全削除。
    *   ThumbnailDatabaseから過去のバグ対応（UTF-8 ⇔ UTF-16変換）の記述を削除。
    *   Widgetコンポーネント（TrackInfoWidget）から細かすぎる実装レベルの記述（WidgetCommonの共通ユーティリティ等）を削除。

## 3. 対象ファイル
* `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md`

## 4. 実行結果
すべての指示事項に沿って `PROJECT_ARCHITECTURE.md` の更新およびノイズパージを完了した。
