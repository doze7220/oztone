【目的】
Phase 8-8: 「忍者プレイリスト」の実装。シークバー右上の「TRACK XXX/XXX」表示をトリガーとし、ホバー時にウィンドウ右端から現在のシャッフル再生順リストがスライドインするUIを構築します。すべてのレイアウトパラメータはINIファイルで管理します。

【作業手順（厳守事項）】
AIのハルシネーション（コードの破壊）を防止するため、以下の手順を必ず守ること。
1. 実装計画書を作成する。
2. 実装計画書を元に、タスクリストを作成する。
3. 作業用ログとして `_docs/logs/20260701_HHMM_Phase8-8_Playlist.md` を新規作成する。
4. タスク終了ごとに作業ログに作業内容を反映し、タスクリストを更新する。
5. ユーザーの確認を待つ。決して次のタスクに進まないこと。
※注意：今回の変更は複数ファイルにまたがるため、すべてのタスクが完了するまでビルドエラーになることは想定内です。途中のタスク完了時にビルドテストを行う必要はありません。

【実装要件】
1. ConfigManager の拡張
対象: src/ConfigManager.h, src/ConfigManager.cpp
- `[Layout_Playlist]` セクションを新設し、以下のパラメータの読み書きとゲッターを追加してください。
  - トラック数表示用: TrackCountFontFamily, TrackCountFontSize, TrackCountRightOffset, TrackCountBottomOffset, TrackCountLetterSpacing, TrackCountShadowOffsetX, TrackCountShadowOffsetY, TrackCountShadowOpacity
  - プレイリスト用: PlaylistWidth, PlaylistItemOffsetY (1項目ごとの高さ/間隔), PlaylistTitleFontSize, PlaylistArtistTimeFontSize

2. PlaylistManager の拡張
対象: src/PlaylistManager.h, src/PlaylistManager.cpp
- 現在のシャッフルインデックス（自分が今何曲目にいるか）を取得するメソッドを追加してください。
- 現在のシャッフルリスト全体（次に再生される順番）のファイルパス一覧を取得するメソッドを追加してください。

3. Window のホバー判定（優先度制御）
対象: src/Window.h, src/Window.cpp
- トラック数表示（TRACK XXX/XXX）領域の矩形を計算し、`IsInPlaylistRegion(x, y)` としてホバー判定を追加してください。
- 既存の再生コントロール領域の判定 (`IsInPlaybackControlRegion`) よりも先にこの判定を行い、「プレイリストポップアップを優先」させてください。

4. Renderer の描画とスライドイン・アニメーション
対象: src/Renderer.h, src/Renderer.cpp
- トラック数表示用のテキストフォーマット、およびプレイリスト用のテキストフォーマット（タイトル用、アーティスト＆時間用）を作成してください。
- 毎フレームの `Render` メソッド内において、以下の描画を追加してください：
  - (1) 指定されたオフセット座標に「TRACK XXX/XXX」を描画（トリミング、文字間隔、影を適用）。
  - (2) `isPlaylistHovered` が true の場合、X座標を毎フレーム減算して右端からスライドインするアニメーション状態を更新。
  - (3) 描画の一番最後（最前面）に、スライドインしたX座標を用いてプレイリストの背景（半透明の板）と項目リストを描画。
  - (4) プレイリストの項目は「現在再生中の曲」がリストの縦方向の中央に来るようにY座標のオフセットを計算し、リストの先頭・末尾で余白ができすぎないように `std::clamp` でスクロールを制限してください。
  - (5) 各項目は1行目にタイトル（少し大きめ）、2行目にアーティスト名と再生時間（右寄せ）で描画してください（DirectWriteの機能を利用）。

5. Application の連携
対象: src/Application.cpp
- `PlaylistManager` から現在のインデックス、総曲数、シャッフルリスト順を取得し、それを `Renderer` の描画に渡す橋渡し処理を追加してください。

【最終タスク】
すべてのタスク完了後、ビルドを実行し、最後に作業ログと `PROJECT_ARCHITECTURE.md` を照らし合わせ、アーキテクチャ資料の更新漏れがないかをチェックして反映させてください。
