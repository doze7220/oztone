# RES:実装計画・作業レポート Phase 18-9: 全ホットキー操作のOSD対応

## 1. 実装目的
Phase 18-8で構築したフライテキスト（`OsdWidget`）の基盤を活用し、すべてのグローバルホットキーおよびメディアキー操作に対して、OSDリアクション（画面中央へのフェードテキスト表示）を表示する極上のUXを完成させる。

既に音量調整（`TriggerVolumeOsd`）、Z-Order変更（`TriggerFlyText`）、シャッフル切替（`TriggerFlyText`）のOSD表示は実装済みである。本フェーズでは、これら以外の残りすべてのアクションに対して `TriggerFlyText` 呼び出しを追加する。

## 2. アーキテクチャ設計
### 要件1: OSDトリガーの追加対象アクション
現在のホットキー・メディアキーコールバック処理（`Application.cpp` L25-259）を精査し、OSD未対応のアクションを特定した。

**対象となるアクション一覧と呼び出し箇所:**

| アクション | 呼び出し元 | 現在のOSD | 追加するFlyText |
|---|---|---|---|
| 再生/一時停止 | `HandleMediaCommand` (L26-27) / `HK_PLAY_PAUSE` (L173-174) | なし | `L"PLAY / PAUSE"` |
| 停止 | `HandleMediaCommand` (L28-29) / `HK_STOP` (L176-177) | なし | `L"STOP"` |
| 次の曲 | `HandleMediaCommand` (L30-33) / `HK_NEXT_TRACK` (L167-168) | なし | `L"NEXT TRACK"` |
| 前の曲 | `HandleMediaCommand` (L30,34-36) / `HK_PREV_TRACK` (L170-171) | なし | `L"PREVIOUS TRACK"` |
| 次のプレイリスト | `HK_NEXT_PLAYLIST` (L229-235) | なし | `L"NEXT PLAYLIST"` |
| 前のプレイリスト | `HK_PREV_PLAYLIST` (L231-234) | なし | `L"PREVIOUS PLAYLIST"` |
| アプリ終了 | `HK_EXIT_APP` (L254-256) | なし | `L"EXIT"` |
| シャッフル切替 | `SetShuffleCallback` (L144-160) | **実装済み** | ― |
| 音量調整 (±5%, ±25%) | `HK_VOL_UP/DOWN_*` (L179-213) | **実装済み (VolumeOsd)** | ― |
| Z-Order: 最前面固定 | `HK_ACTIVE_TOPMOST` (L239-244) | **実装済み** | ― |
| Z-Order: 最背面固定 | `HK_ACTIVE_BOTTOM` (L247-251) | **実装済み** | ― |

### 要件2: 既存OSD基盤の完全再利用
- `Renderer::TriggerFlyText(const std::wstring& text)` をそのまま呼び出すのみ。
- `OsdWidget` やアーキテクチャへの変更は一切行わない。
- 新たなWidgetの追加も行わない。

### 設計方針
- メディアキーコールバック `HandleMediaCommand` は、グローバルホットキー（`HK_PLAY_PAUSE`, `HK_STOP`, `HK_NEXT_TRACK`, `HK_PREV_TRACK`）からも呼び出される共通関数であるため、`HandleMediaCommand` 内に `TriggerFlyText` を追加することで、メディアキー・ホットキーの両方のルートを1箇所でカバーできる。
- プレイリスト切替（`HK_NEXT_PLAYLIST` / `HK_PREV_PLAYLIST`）とアプリ終了（`HK_EXIT_APP`）は `SetHotkeyCallback` ラムダ内のcaseブロックに直接追加する。

## 3. 実装タスクリスト
- [x] タスク1: `HandleMediaCommand` への OSD トリガー追加
    - `Application::HandleMediaCommand()` 内の各アクション分岐に `m_renderer.TriggerFlyText()` 呼び出しを追加する。
    - 再生/一時停止: `L"PLAY / PAUSE"`（L27の直後）
    - 停止: `L"STOP"`（L29の直後）
    - 次の曲: `L"NEXT TRACK"`（`APPCOMMAND_MEDIA_NEXTTRACK` 分岐の冒頭、L32付近）
    - 前の曲: `L"PREVIOUS TRACK"`（`APPCOMMAND_MEDIA_PREVIOUSTRACK` 分岐の冒頭、L34付近）

- [x] タスク2: `SetHotkeyCallback` ラムダへの OSD トリガー追加
    - `HK_NEXT_PLAYLIST` ケース: `SwitchPlaylist` 呼び出し後に `m_renderer.TriggerFlyText(L"NEXT PLAYLIST")` を追加（L235付近）。
    - `HK_PREV_PLAYLIST` ケース: `SwitchPlaylist` 呼び出し後に `m_renderer.TriggerFlyText(L"PREVIOUS PLAYLIST")` を追加（L235付近）。
      - 注: 両ケースは同一caseブロック（L215-237）で処理されており、`hotkeyId` の値で分岐するため、条件分岐でテキストを変える。
    - `HK_EXIT_APP` ケース: `PostMessage` の前に `m_renderer.TriggerFlyText(L"EXIT")` を追加（L255付近）。

- [x] タスク3: `PROJECT_ARCHITECTURE.md` の更新
    - `OsdWidget` の説明（L206付近）を更新し、対応アクションの完全なリストを反映する。

## 4. 詳細作業内容
*   **Application.cppの修正**
    *   `HandleMediaCommand` にて `APPCOMMAND_MEDIA_PLAY_PAUSE`, `APPCOMMAND_MEDIA_STOP`, `APPCOMMAND_MEDIA_NEXTTRACK`, `APPCOMMAND_MEDIA_PREVIOUSTRACK` 発生時に、対応する `m_renderer.TriggerFlyText` を追加。
    *   `SetHotkeyCallback` 内の `HK_NEXT_PLAYLIST`, `HK_PREV_PLAYLIST` の分岐にて、`SwitchPlaylist` の実行後に該当する `TriggerFlyText` を呼び出すように追加。
    *   `SetHotkeyCallback` 内の `HK_EXIT_APP` 時に `PostMessage` の直前で `m_renderer.TriggerFlyText(L"EXIT")` を呼び出すように追加。
*   **ドキュメント更新**
    *   `PROJECT_ARCHITECTURE.md` の `OsdWidget` (L206付近) の説明文に、本フェーズで追加したアクション（再生/一時停止、停止、次/前の曲、プレイリスト切替、アプリ終了など）の完全なリストを反映し、すべてのホットキー・メディアキー操作時の状態表示に対応した旨を追記しました。
