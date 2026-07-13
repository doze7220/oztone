### 作業指示書 REQ: Hotfix: VolumeControlWidget の Transform 戻し忘れ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレート（D:\ozlab\oztone\_docs\RES(Hotfix)_template.md）を元に、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_VolumeTooltipTransform.md）として新規作成すること。作業レポートに原因と対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: `VolumeControlWidget::Draw` 内の Transform 状態の保存と復元**
    *   `src/Widget_VolumeControl.cpp` を調査し、`Draw` メソッド内においてツールチップを描画しているブロック（`if (m_tooltipAlpha > 0.0f)` 等の内部）を特定する。
    *   ツールチップやそのドロップシャドウの描画のために `context->SetTransform(...)` が呼ばれている場合、その変更が後続の Widget（`PlaylistWidget` など）の描画座標系を破壊している（画面右下に謎の黒い矩形が現れるバグの原因）。
    *   ツールチップ描画の処理ブロックに入る直前で `D2D1::Matrix3x2F oldTransform; context->GetTransform(&oldTransform);` を用いて元の変換行列を保存し、ツールチップ描画ブロックを抜ける直前に `context->SetTransform(oldTransform);` で必ず元の状態に復元する安全処理（副作用の排除）を追加すること。

#### 【絶対遵守ルール (Constraints)】
