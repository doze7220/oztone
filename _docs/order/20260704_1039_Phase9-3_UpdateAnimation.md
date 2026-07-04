Phase 9-3: アニメーション状態更新の分離（作業指示書）
以下のテキストをコピーして、次回のAIへ投下してください。
### Phase 9-3: アニメーション状態更新の分離（作業指示書）

#### 【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（_docs/logs/YYYYMMDD_HHMM_Phase9-3_UpdateAnimation.md）として新規作成する。
2. 作業計画をタスクリストに分割し、作業レポート末尾に記載する。
3. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、ユーザーの承認を待つこと。

---

#### 【実装要件】

**1. 目的**
現在 `Renderer::Render` メソッド内に混在している、アニメーション用の状態更新処理（UIのフェードイン・フェードアウトやスライドアニメーションの計算）を、新しい `Renderer::UpdateAnimation` メソッドへと完全に切り出す。
これにより、`Render` メソッドを「渡された最新のレイアウト座標と状態を用いて、純粋に画面へピクセルを描画するだけの関数」へと昇華させる。

**2. 対象ファイル**
*   `src/Renderer.h`
*   `src/Renderer.cpp`
*   `src/Application.cpp`

**3. アーキテクチャ設計（インターフェース）**
*   将来的なフレームレート非依存のアニメーション実装を見据え、`UpdateAnimation` メソッドは必ず「時間差分 (`deltaTime`)」を受け取る設計とすること。
    *   例: `void UpdateAnimation(float deltaTime, bool isControlHovered, bool isPlaylistHovered);`
    *   ※ または `AnimationInput` のような構造体を定義して渡してもよい。

**4. 段階的な実装タスクリスト**
安全性と動作確認のため、以下の順序でタスクを分割して実行すること。

*   **Task 1: `UpdateAnimation` メソッドの追加と `Application` からの呼び出し**
    *   `src/Renderer.h` に `UpdateAnimation(float deltaTime, ...)` を追加する。
    *   `src/Renderer.cpp` に空の実装を追加。
    *   `src/Application.cpp` の描画フローにおいて、`UpdateAnimation(...)` を呼び出した直後に `Render(...)` を呼び出すよう構築し、ビルド確認。
*   **Task 2: 再生・音量コントロールのフェード状態 (`m_controlAlpha`) の分離**
    *   `Renderer::Render` 内にある `m_controlAlpha` の加算・減算処理を `UpdateAnimation` へ移動する。
    *   ビルドし、マウスホバー時のフェードアニメーションが正常に動作することを確認。
*   **Task 3: プレイリストのスライド状態 (`m_playlistSlideX`) の分離**
    *   `Renderer::Render` 内にある `m_playlistSlideX` のイージング計算処理を `UpdateAnimation` へ移動する。
    *   ビルドし、プレイリストの展開・格納アニメーションが正常に動作することを確認。
*   **Task 4: 状態分離の最終確認**
    *   `Renderer::Render` 内部を精査し、メンバ変数の更新（状態の書き換え）処理が**一切残っていないこと**を確認する。
    * Render() 内に状態更新処理が残っていないことを確認する。
    * Render() がメンバ変数を読み取るだけであることを確認する。
    * UpdateAnimation() が状態更新を一元管理していることを確認する。


#### 【本フェーズ完了条件】
以下をすべて満たした時点で完了とする。
- Render() 内に状態更新処理が存在しない。
- UpdateAnimation() 内に描画API呼び出しが存在しない。
- Application の呼び出し順が UpdateAnimation() → Render() となっている。
- アニメーションの見た目・速度・手触りが変更されていない。
- ビルド成功および実行確認済み。

**5. 懸念点・禁止事項**
*   **`Render()` における状態更新の完全禁止:** `Render()` は現在の状態を読み取り、描画のみを担当すること。`Render()` 内でメンバ変数を書き換えることは絶対に許可しない。
*   **呼び出し順序の固定:** `Application` 側の描画フローは必ず `UpdateAnimation() → Render()` の順序を維持すること。AIの判断でこの順番を変更してはならない。
*   **描画への非依存:** `UpdateAnimation` 内ではDirect2Dなどの描画API（`Draw...` や `Fill...` 等）を絶対に呼び出さないこと。純粋な変数の更新のみを担当する。
*   **既存の手触りの維持:** アニメーションの計算式（加減算のステップ値やイージングの係数）は既存のものをそのまま流用し、今回は `deltaTime` を使わずに従来の固定値のままでよい（将来の拡張のためのインターフェース変更であるため）。動作の見た目やスピードを変更しないこと。
* **責務の限定:** `UpdateAnimation()` が担当するのは既存メンバ変数のアニメーション状態更新のみとする。
  新たなゲームロジック・UI状態判定・入力処理・ConfigManager参照などを追加してはならない。
  