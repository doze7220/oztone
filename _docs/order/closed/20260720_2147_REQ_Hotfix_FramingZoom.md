### 作業指示書 REQ: Phase 23-7 Hotfix 3 : 画像拡縮クランプとズーム中心補正 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  参考資料: D:\ozlab\oztone\_docs\logs\20260720_2144_RES_Hotfix_ClampArtFraming.md

#### 【作業手順（厳守事項）】
本プロンプトは背景アートフレーミングの拡縮機能不備に関する Hotfix である。
1. ルールおよび開発資料を熟読・把握すること。
2. 参考資料（20260720_2144_RES_Hotfix_ClampArtFraming.md）を読み、不具合の真の原因と修正方針を確認すること。
3. 上記を確認した後、以下の【実装要件】に従ってソースコードの修正を実行すること。
4. コード修正完了後、作業レポート（D:\ozlab\oztone\_docs\logs\YYMMDD_HHMM_RES_Hotfix_FramingZoom.md）を新規作成し、対応内容を記録すること。
5. チャットにて「画像拡縮クランプのHotfix実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   **要件1: スクロールコールバックへのマウス座標の受け渡し**
    *   `src/Window.h` の `SetArtFramingScrollCallback` のシグネチャを、スクロール量に加えてマウス座標を受け取れる形（例: `std::function<void(float delta, int x, int y)>`）に変更する。
    *   `src/Window_Mouse.cpp` 内でこのコールバックを呼び出している箇所（`HandleMouseWheel` 等）を修正し、現在のマウス座標（必要に応じてDPIスケーリング済みの論理座標）を正しく渡す。
*   **要件2: ズーム中心を維持するオフセット補正計算の実装**
    *   `src/Application_Initialize.cpp` 内の `SetArtFramingScrollCallback` ラムダ式を改修する。
    *   引数で受け取ったマウス座標を「ズームの原点」として扱い、スケール（`artScale`）が変動した際に、その中心点が画面上でズレないように `artX` および `artY` を再計算・補正するロジックを実装する。
*   **要件3: ClampArtFraming 内でのスケール参照の書き戻し**
    *   `src/Renderer.cpp` の `ClampArtFraming` メソッド内において、引数の `scale` を `std::max(1.0f, scale)` 等を用いて制限し、その値が確実に呼び出し元（Application側）の変数へ書き戻されるように修正する。

#### 【絶対遵守ルール (Constraints)】
*   **移動クランプの破壊禁止** : `ClampArtFraming` 内に存在する `offsetX`, `offsetY` に対する既存の制限ロジックは正常に機能しているため、これらを破壊・改変しないこと。
