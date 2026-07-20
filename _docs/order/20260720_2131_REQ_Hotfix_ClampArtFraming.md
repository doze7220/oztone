### 作業指示書 REQ: Phase 23-7 Hotfix 2 : ClampArtFraming の完全再実装 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
本プロンプトは、前回実装を省略された `Renderer::ClampArtFraming` メソッドを正しく再実装するための Hotfix である。
1. ルールおよび開発資料を熟読・把握すること。
2. 以下の【実装要件】に記載された計算ロジックを **1行たりとも省略せずに** `src/Renderer.cpp` へ実装すること。
3. 作業完了後、作業レポート（D:\ozlab\oztone\_docs\logs\20260720_RES_Hotfix_ClampArtFraming.md）を新規作成し、対応内容を記録すること。
4. チャットにて「ClampArtFramingの完全実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
*   `src/Renderer.cpp` 内の空になっている `Renderer::ClampArtFraming(float& scale, float& offsetX, float& offsetY)` を、以下のロジックに書き換えて完全実装すること。

**【クランプ計算ロジック】**
1. `!m_d2dContext` または `!m_currentBgBitmap` の場合は何もせず `return;` する。
2. `m_d2dContext->GetSize()` でウィンドウの論理サイズ (`size`) を取得する。
3. `m_currentBgBitmap->GetSize()` で画像の論理サイズ (`bmpSize`) を取得する。
4. ウィンドウサイズと画像サイズの比率から、アスペクト比を維持して画面を覆うための `baseScale` を計算する。
   `float scaleX = size.width / bmpSize.width;`
   `float scaleY = size.height / bmpSize.height;`
   `float baseScale = std::max(scaleX, scaleY);`
5. 引数の `scale` は 1.0f を最小値としてクランプする。（`if (scale < 1.0f) scale = 1.0f;`）
6. 拡大後の実際の画像サイズを計算する。
   `float scaledWidth = bmpSize.width * baseScale * scale;`
   `float scaledHeight = bmpSize.height * baseScale * scale;`
7. 画像がウィンドウ内に余白を作らないための、オフセットの最小値（限界座標）を計算する。
   `float minX = size.width - scaledWidth;`
   `float minY = size.height - scaledHeight;`
8. 引数の `offsetX` を `minX` 〜 `0.0f` の範囲にクランプする。
9. 引数の `offsetY` を `minY` 〜 `0.0f` の範囲にクランプする。

#### 【絶対遵守ルール (Constraints)】
*   **ロジックの省略禁止** : 上記の計算は背景アートがウィンドウの余白に露出しないための必須ロジックである。絶対に空実装で済ませたり、簡略化したりしないこと。
