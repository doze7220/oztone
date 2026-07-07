### 作業指示書 REQ: Phase 18-9: 全ホットキー操作のOSD対応 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための実装計画、兼作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-9_AllHotkeyOSD.md`）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
Phase 18-8で構築したフライテキスト（OsdWidget）の基盤を活用し、すべてのグローバルホットキーおよびメディアキー操作に対してOSDリアクションを表示する極上のUXを完成させる。

* **要件1: Application クラス等における OSD トリガーの追加**
  * `Application` クラス内でホットキーイベントを処理している箇所（メディアキーのコールバックやグローバルホットキーの処理部分）を拡張する。
  * 既存の音量調整および Z-Order 変更以外のすべてのアクションに対して、対応する文字列を引数として `m_renderer.TriggerFlyText()` を呼び出す。
  * 対象と文字列の例（デザインに合わせて大文字表記などで調整可）:
    * 再生/一時停止: `L"PLAY / PAUSE"`
    * 停止: `L"STOP"`
    * 次の曲: `L"NEXT TRACK"`
    * 前の曲: `L"PREVIOUS TRACK"`
    * 次のプレイリスト: `L"NEXT PLAYLIST"`
    * 前のプレイリスト: `L"PREVIOUS PLAYLIST"`
    * アプリ終了: `L"EXIT"` など
* **要件2: 既存のOSD基盤の完全再利用**
  * 新たなWidgetの追加やアーキテクチャの変更は行わない。Phase 18-8 で実装済みの `TriggerFlyText` と `OsdWidget` をそのまま活用すること。

#### 【作業終了後】
1. 作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-9_AllHotkeyOSD.md`）に、詳細作業内容を記載する（タスクリストに含める）こと。
2. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

#### 【絶対遵守ルール (Constraints)】
* **アーキテクチャの厳守**: `OsdWidget` や `Renderer` の状態管理の仕組みは一切変更せず、トリガーの呼び出し追加のみに留めること。
