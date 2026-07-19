# RES:HOTFIX作業レポート: トラックインフォのCD帯（トラックナンバー）機能の削除

## 1. 実装目的
不要となったトラックインフォのCD帯（トラックナンバー）機能を削除し、ソースコードおよび設定ファイルをスリム化・クリーンアップする。

## 2. 調査内容
指示書に沿って、`ConfigManager`、`LayoutCalculator`、`Widget_TrackInfo` から TrackCount（CD帯UI）に関連する設定、計算ロジック、リソース、および描画処理を完全に削除する。

## 3. 対象ファイル
- src/ConfigManager.h
- src/ConfigManager_DefaultIni.h
- src/ConfigManager_Playback.cpp
- src/LayoutCalculator.h
- src/LayoutCalculator.cpp
- src/Widget_TrackInfo.h
- src/Widget_TrackInfo.cpp
- PROJECT_ARCHITECTURE.md

## 4. 実装タスクリスト
[x] タスク1: ConfigManagerからのパラメータ完全パージ
[x] タスク2: LayoutCalculatorからの計算ロジック削除
[x] タスク3: Widget_TrackInfoからの描画・リソース・計算ロジックの削除
[x] タスク4: PROJECT_ARCHITECTURE.mdの更新

## 5. 詳細作業内容
* タスク1: ConfigManagerからのパラメータ完全パージ
    - `src/ConfigManager.h` からTrackCountに関するゲッター関数およびメンバ変数を削除した。
    - `src/ConfigManager_DefaultIni.h` からTrackCount設定の初期値を削除した。
    - `src/ConfigManager_Playback.cpp` からTrackCount関連のLoadOrWrite呼び出しを削除した。
* タスク2: LayoutCalculatorからの計算ロジック削除
    - `src/LayoutCalculator.h` の `TrackInfoLayout` 構造体から TrackCount 関連のメンバ変数を削除した。
    - `src/LayoutCalculator.cpp` の `CalculateTrackInfoLayout` から TrackCount（CD帯UI）の座標・サイズ計算ロジックを削除した。
* タスク3: Widget_TrackInfoからの描画・リソース・計算ロジックの削除
    - `src/Widget_TrackInfo.h` から TrackCount 関連の TextFormat、TextLayout、Brush を削除した。
    - `src/Widget_TrackInfo.cpp` の `CreateResources`, `ReleaseResources`, `UpdateLayout`, `Draw` から TrackCount に関するリソース生成・解放、テキストレイアウト構築、および-90度回転を伴う描画処理を完全に削除した。
* タスク4: PROJECT_ARCHITECTURE.mdの更新
    - `TrackInfoWidget` の説明から「トラック番号の描画」という文言を削除した。
