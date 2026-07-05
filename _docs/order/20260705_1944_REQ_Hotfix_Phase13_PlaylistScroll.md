### 作業指示書 REQ: Hotfix_Phase13: プレイリストのスクロール計算一元化 (実装実行)

以下のプロジェクトルールと開発資料を熟読すること。
* D:\ozlab\oztone\PROJECT_CONSTITUTION.md
* D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは過去の技術的負債を解消するためのHotfixの実装実行である。事前の計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Hotfix_Phase13_PlaylistScroll.md`）を新規作成し、対応内容を記録すること。
3. その際、本レポート内には「本件は `20260704_2314_RES_Phase13_PlaylistHierarchy.md` で混入したスクロール計算の二重管理バグに対するHotfixである」と明記し、不具合の発生元と修正の文脈を明確に紐付けること。
4. `D:\ozlab\oztone\PROJECT_ARCHITECTURE.md` を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
5. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

---

#### 【実装要件】
Phase 13（プレイリスト階層化とツールバー追加）で混入した、プレイリストの「クリック判定」と「描画」におけるスクロール計算の二重管理を解消し、ウィンドウ高さが低い場合にクリック判定が大きくズレる不具合をアーキテクチャレベルで根本解決する。

* **修正1: `LayoutCalculator` へのインデックス逆算メソッド新設 (`src/LayoutCalculator.h/cpp`)**
  * `LayoutCalculator` に、論理Y座標からクリックされたプレイリストのインデックスを数学的に逆算するメソッド（例: `int GetPlaylistItemIndexAt(float logicalY, const PlaylistLayout& layout, size_t totalItems, float itemHeight, float manualScrollY)` 等）を追加する。
  * `CalculatePlaylistLayout` 内で使用しているものと全く同じ計算式（ベーススクロールの算出、クランプ処理、手動スクロール量の反映）を用いて、実際の描画Y座標と完全に一致するインデックスを算出するロジックを構築すること。
  * その際、スクロール計算やクランプにおける「リスト表示領域の高さ」の基準を、画面全体の高さからツールバーの高さ等を適切に差し引いた正確な値へ修正・統一すること。

* **修正2: `Application.cpp` の判定ロジックの浄化**
  * `Application.cpp` 内で設定している `SetPlaylistClickCallback` 等の内部において、独自に行っていた「Y座標からの引き算や割り算、クランプ処理によるインデックス算出」をすべて削除する。
  * 代わりに、現在適用されている `PlaylistLayout` と上記で新設した `LayoutCalculator` の逆算メソッドを呼び出す形へ置き換え、インデックス計算の責務を `LayoutCalculator` に完全委譲（一元化）する。
  * 曲一覧モード時・プレイリスト一覧モード時の両方のクリック判定において、この一元化された計算ロジックを通すこと。
