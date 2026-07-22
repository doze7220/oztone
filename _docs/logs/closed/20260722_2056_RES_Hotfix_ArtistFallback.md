# RES:HOTFIX作業レポート: アーティストメタ情報なしの表示変更

## 1. 実装目的
OZtoneのミニマルなUI美学を向上させるため、曲のメタデータからアーティスト名が取得できなかった場合のフォールバック表示を、従来の「---」から「」（空文字）へと変更する。

## 2. 調査内容
ソースコード全体から、アーティスト名が空の場合のフォールバックとして `L"---"` を代入している箇所を検索し特定した。
対象は以下のファイル群の該当箇所である。
*   `src/Application_Playback.cpp`
*   `src/Application_Render.cpp`
*   `src/TrackAnalyzer.cpp`

## 3. 対象ファイル
*   `src/Application_Playback.cpp`
*   `src/Application_Render.cpp`
*   `src/TrackAnalyzer.cpp`

## 4. 実装タスクリスト
[x] タスク1: アーティスト名のフォールバックを空文字に変更 - 各ファイル内で `L"---"` を `L""` に置換する

## 5. 詳細作業内容
* タスク1: アーティスト名のフォールバックを空文字に変更
    - `src/Application_Playback.cpp` における3箇所の `L"---"` 代入を `L""` に修正した。
    - `src/Application_Render.cpp` における1箇所の `L"---"` 代入を `L""` に修正した（`timeString` に関する箇所はそのまま維持した）。
    - `src/TrackAnalyzer.cpp` における3箇所の `L"---"` 代入を `L""` に修正した。
