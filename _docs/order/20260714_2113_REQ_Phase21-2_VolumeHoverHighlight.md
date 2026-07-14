### 作業指示書 REQ: Phase 21-2: 音量アイコンのホバーハイライト対応 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-2_VolumeHoverHighlight.md）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

#### 【実装要件】
現在、再生コントロールやロゴメニューで実装されている「ホバー時の非対称カラーフェードアニメーション」を、音量コントロールのスピーカーアイコンにも適用し、UIの操作感（アフォーダンス）を統一する。

*   **要件1: アニメーション状態の追加 (Widget_VolumeControl)**
    *   `src/Widget_VolumeControl.h` の `VolumeControlWidget` クラスに、スピーカーアイコンのホバーフェード状態をトラッキングするための変数（例: `float m_hoverAlpha = 0.0f;`）を追加する。
*   **要件2: 非対称フェードの更新 (UpdateAnimation)**
    *   `src/Widget_VolumeControl.cpp` の `UpdateAnimation` メソッドを改修する。
    *   現在マウスが音量コントロール領域にあるか（例えば `ctx.isVolumeHovered` などの既存フラグ、あるいはそれに準ずるコンテキスト情報）を判定し、対象であれば `m_hoverAlpha` をフェードイン（例: 即座に 1.0f または高速加算）、外れていれば `ConfigManager::GetHoverFadeOutSpeed()` などを利用してフェードアウト（減算）させる非対称フェードロジックを実装する。
*   **要件3: カラーブレンド（Lerp）の適用 (Draw)**
    *   `src/Widget_VolumeControl.cpp` の `Draw` メソッドを改修する。
    *   スピーカーアイコンを描画する際、ベースカラー（白など）と `ConfigManager::GetHoverIconColor()` の色を `m_hoverAlpha` を用いて線形補間（Lerp）する。
    *   ブレンドした色をスピーカー描画用のブラシにセット（`SetColor`）し、描画に反映させる。

#### 【絶対遵守ルール (Constraints)】
*   **カプセル化の徹底**: アニメーションの計算（Lerp）やブラシカラーの変更は `Widget_VolumeControl` 内部に留め、`Renderer` や `LayoutCalculator` に影響を与えないこと。
*   **既存機能の維持**: 既に実装されている音量のツールチップ表示やシークバーとの連動フェードなどの機能は一切壊さないこと。
-----------------------------------------------------------------------------------
### 作業指示書 REQ: Phase 21-2: 音量アイコンのホバーハイライト対応 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260714_2113_RES_Phase21-2_VolumeHoverHighlight.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはPhase 21-2の「実装実行」である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、既存の作業レポート（20260714_2113_RES_Phase21-2_VolumeHoverHighlight.md）の「タスク1〜3」のチェックボックスを完了 `[x]` にし、詳細作業内容を追記すること。
3. チャットにて「Phase 21-2の実装がすべて完了しました！ビルド・動作確認をお願いします」と報告し、待機すること。

#### 【実装要件】
*   **タスク1: VolumeControlWidgetへの変数追加**
    *   `src/Widget_VolumeControl.h` の `VolumeControlWidget` クラスの private メンバに `float m_hoverAlpha = 0.0f;` を追加する。
*   **タスク2: UpdateAnimationの改修 (非対称フェード)**
    *   `src/Widget_VolumeControl.cpp` の `UpdateAnimation` メソッドを改修する。
    *   `ctx.isVolumeHovered` が true の場合は `m_hoverAlpha` をフェードイン（例: 即座に `1.0f` に設定、または高速加算）させる。
    *   false の場合は、`deltaTime` と `ctx.config->GetHoverFadeOutSpeed()` などを利用して `0.0f` に向かって滑らかに減算し、`std::clamp` 等で `0.0f` 〜 `1.0f` に収める非対称フェードロジックを実装する。
*   **タスク3: Drawメソッドの改修 (Lerpによる色合成)**
    *   `src/Widget_VolumeControl.cpp` の `Draw` メソッドを改修する。
    *   スピーカーアイコン等を描画する直前に、ベースカラー（例: `D2D1::ColorF(D2D1::ColorF::White)`）とホバーカラー（`ctx.config->GetHoverIconColor()`）を `m_hoverAlpha` を用いて線形補間（Lerp）する処理を追加する（必要であれば `WidgetCommon` 等の既存のLerpロジック、または自前のラムダ関数を利用）。
    *   算出された補間色を `m_controlBrush->SetColor(...)` でブラシに適用し、ジオメトリ（`m_speakerIconGeometry` や音量バー等）の描画に反映させる。

#### 【絶対遵守ルール (Constraints)】
*   **カプセル化の徹底**: アニメーションの計算（Lerp）やブラシカラーの変更は `Widget_VolumeControl.cpp` の内部（`Draw` メソッド内等）に留め、既存のツールチップ表示やシークバーの減光ロジックなどの他機能は一切破壊しないこと。

-----------------------------------------------------------------------------------