### 作業指示書 REQ: Phase 18-12: 曲ごとの背景アート・フレーミング（位置・拡大率の保存） (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
2. 以下の【実装要件】を満たすための実装計画を作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase18-12_ArtFraming.md`）として新規作成すること。
3. レポートのフォーマットは、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠し、細かなタスクリストを含めること。
4. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
ウィンドウサイズが可変であるOZtoneにおいて、曲の背景アルバムアートの「最高のアングル（画角）」をユーザー自身がフレーミング（位置・拡大率の調整）し、保存できる神機能を実装する。

*   **要件1: バックエンドの拡張（メタデータとTSVキャッシュ）**
    *   `src/PlaylistManager.h` の `TrackMetadata` 構造体に、フレーミング情報を保持する `float artOffsetX = 0.0f;`, `float artOffsetY = 0.0f;`, `float artScale = 1.0f;` を追加する。
    *   `SaveToFile` および `LoadFromFile` メソッドにおける TSVの読み書きロジックを拡張し、既存の4項目（`filepath \t title \t artist \t timeString`）の後ろに `\t artOffsetX \t artOffsetY \t artScale` を追加して永続化する。要素不足の過去ファイルへのフォールバック（初期値適用）も安全に行うこと。

*   **要件2: 描画・レイアウト計算への反映 (LayoutCalculator / Renderer)**
    *   `LayoutCalculator::CalculateBackgroundLayout` に対して `artOffsetX`, `artOffsetY`, `artScale` のパラメータを渡し、画像の拡大・描画矩形の計算に適用する。
    *   `Renderer::DrawBackground` にて、現在の `TrackMetadata` から取得したオフセットとスケールを `LayoutCalculator` に渡して描画する。

*   **要件3: 「余白を作らない」ストイックな入力ハンドリングとクランプ計算 (Window / Application)**
    *   背景（他のUI判定がない領域）でのマウス操作、およびウィンドウアクティブ時のキーボード操作として以下を追加する。
        *   **SHIFT + 左ドラッグ** または **SHIFT + カーソルキー(上下左右)**: 背景アートの移動（`artOffsetX`, `artOffsetY` の増減）。
        *   **SHIFT + マウスホイール** または **SHIFT + PAGEUP / PAGEDOWN**: 背景アートの拡大縮小（`artScale` の増減）。
        *   **SHIFT + HOME**: 背景アートの位置と拡大率を初期値（オフセット0, スケール1.0）にリセット。
    *   **【重要: 余白防止の数学的クランプ】**
        *   スケールの最小値は常に `1.0f`（ウィンドウに余白なくフィットするギリギリのサイズ）とし、それ以上縮小できないようにクランプする。
        *   移動（パン）の範囲は、現在のスケールによってはみ出している画像領域（Overflow）の半分（上下・左右）までにクランプする。これにより、どれだけ動かしても絶対に黒帯（非表示領域）が見えないスマホ写真アプリのような極上の操作感を実現する。

*   **要件4: リアルタイムな保存（セーブ）**
    *   操作が終了した（または変動した）タイミングで、バックグラウンドで `PlaylistManager::UpdateMetadata` および `PlaylistManager::SaveToFile` を呼び出し、TSVキャッシュへ変更を即座に上書き保存する。

### 作業指示書 REQ: Phase 18-12 Hotfix: ウィンドウリサイズ時の背景アスペクト比崩れ修正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（`D:\ozlab\oztone\_docs\logs\20260707_2345_RES_Phase18-12_ArtFraming.md`）の末尾に「HOTFIX: ウィンドウリサイズ時の背景アスペクト比崩れ修正」の項目を追加し、原因と対応内容を追記すること。
3. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

#### 【実装要件】
*   **不具合の原因**:
    フレーミング設定後にウィンドウをリサイズすると、背景画像を描画する際の「切り抜き領域（`srcRect`）」と「描画先領域（`destRect`）」のアスペクト比がズレてしまい、画像が歪んでしまう問題が発生している。
*   **修正内容 (LayoutCalculator.cpp)**:
    `LayoutCalculator::CalculateBackgroundLayout` の計算ロジックを以下のように変更し、アスペクト比の維持と動的クランプを完全に行うこと。
    1.  **スケール計算**: `baseScale = max(windowWidth / imageWidth, windowHeight / imageHeight)` として基本となるフィットスケールを計算し、これに設定値 `artScale` を掛けたものを `finalScale` とする。
    2.  **描画サイズの算出**: 画像の実際の描画サイズを `drawWidth = imageWidth * finalScale`, `drawHeight = imageHeight * finalScale` とする。
    3.  **動的クランプ（リサイズ追従）**: ウィンドウのリサイズによって「余白を作らない限界」が変動するため、限界値 `limitX = max(0.0f, (drawWidth - windowWidth) / 2.0f)`, `limitY = max(0.0f, (drawHeight - windowHeight) / 2.0f)` を計算し、引数で渡された `artOffsetX`, `artOffsetY` を `-limit` から `+limit` の範囲に `std::clamp` で再クランプする。
    4.  **座標計算**: クランプ後のオフセットを用いて、`destRect`（描画先）の左上座標を `x = (windowWidth - drawWidth) / 2.0f + clampedOffsetX`, `y = (windowHeight - drawHeight) / 2.0f + clampedOffsetY` として計算する。
    5.  `srcRect` は画像全体（`0, 0, imageWidth, imageHeight`）のまま固定とし、`destRect` 側をウィンドウからはみ出させる形で `BackgroundLayout` 構造体を返すように修正すること。これにより、どれだけリサイズされてもアスペクト比は絶対に狂わなくなる。
