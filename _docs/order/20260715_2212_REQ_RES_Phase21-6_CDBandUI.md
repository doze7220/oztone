### 作業指示書 REQ: Phase 21-6 : CD帯風トラックナンバーUIの実装 (実装実行)
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

#### 【作業手順（厳守事項）】
1. 本プロンプトは「CD帯風トラックナンバーUI」の実装実行である。直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. 作業完了後、作業レポート（`D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase21-6_CDBandUI.md`）として新規作成し、詳細作業内容を記録すること。
3. チャットにて「CD帯UI(Phase 21-6)の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
現在の曲情報（TrackInfoWidget）の一部として水平に描画されているトラックナンバー（XXX/XXX）を、アルバムアートの左側に添えられた「CDケースの帯（タグ）」を模した縦向きのデザインへと改変する。

*   **要件1: ConfigManager への CD帯UI パラメータ追加**
    *   `ConfigManager_Window.cpp`, `ConfigManager_DefaultIni.h`, `ConfigManager.h` を改修し、`[Layout_NowPlaying]` セクションに以下の新規パラメータを追加する。
        *   `TrackCountBoxFontColor`, `TrackCountBoxBaseColor`, `TrackCountBoxBaseOpacity`
        *   `TrackCountBoxWidth`, `TrackCountHeight`
        *   `TrackCountBoxUnderLineWidth`, `TrackCountUnderLineHeight`, `TrackCountBoxUnderLineX`, `TrackCountUnderLineY`
    *   既存の `TrackCountOffsetX`/`Y` 等のパラメータは維持し、デフォルト設定を `DEFAULT_INI_CONTENT` のSSOTとして確実に同期させる。

*   **要件2: LayoutCalculator での起点座標算出**
    *   `LayoutCalculator.cpp` の `CalculateTrackInfoLayout` において、トラックナンバー用の起点座標（回転のアンカーポイント）を計算する。
    *   起点はアルバムアートのベース座標（`ArtOffsetX`, `ArtOffsetY` を加味した位置）を親とし、そこに `TrackCountOffsetX`, `TrackCountOffsetY` を加算した「テキストボックスの右上」の座標とする。

*   **要件3: Widget_TrackInfo でのトランスフォーム描画 (-90度回転)**
    *   `Widget_TrackInfo.cpp` の `CreateResources` にて、新規追加された色情報をもとに、テキストカラーブラシ、ボックス背景ブラシ、アンダーラインブラシを生成する。
    *   `Draw` メソッド内にて、テキストレイアウトの描画サイズをボックスのサイズに合わせる。
    *   描画直前に、現在のトランスフォーム行列を保存し、`D2D1::Matrix3x2F::Rotation(-90.0f, origin)` を用いてキャンバス全体を-90度回転させる。
    *   回転した状態で、半透明の背景ボックス（`FillRectangle`）、アンダーライン、およびトラックナンバーテキストを描画し、直後に保存しておいたトランスフォーム行列を元に戻す。

#### 【絶対遵守ルール (Constraints)】
*   **トランスフォームの復元**: 回転描画の後は、必ず `SetTransform` で以前のマトリクスを復元し、以降の描画（アルバムアートや曲名など）に一切影響を与えないこと。
*   **プレイドラムの除外**: 今回の実装には、スクロールやバケツリレーなどの「プレイドラム」に関連する機能・アニメーションは絶対に含めず、純粋なCD帯UIの描画のみに留めること。

----------------------------------------------------
### 作業指示書 REQ: Hotfix Phase 21-6 CD帯UIのパラメータ整理とシャドウ補正 (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  D:\ozlab\oztone\_docs\logs\20260715_2217_RES_Phase21-6_CDBandUI.md

#### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、Hotfix作業レポートテンプレートを元に、作業レポート（`20260715_2217_RES_Phase21-6_CDBandUI.md`）の末尾にHotfixとして項目を新規作成し、対応内容を記録すること。
3. チャットにて「CD帯UIのパラメータ整理と修正が完了しました。ビルド・動作確認をお願いします」と報告すること。

#### 【実装要件】
先日実装したCD帯風UIにおいて、冗長なパラメータの削除（大掃除）、アルバムアートサイズへの動的連動、および-90度回転キャンバスにおけるシャドウ方向の補正を行う。

*   **要件1: パラメータの語彙統一とパージ (`ConfigManager` 関連)**
    *   `src/ConfigManager.h`, `src/ConfigManager_Window.cpp`, `src/ConfigManager_DefaultIni.h` を修正する。
    *   語彙の統一: `TrackCountBoxUnderLineWidth` を `TrackCountUnderLineWidth` へ、`TrackCountBoxUnderLineX` を `TrackCountUnderLineX` へリネームする。
    *   以下のパラメータは不要となったため、変数・ゲッター・INIパース・デフォルト文字列から**完全に削除**する。
        *   `TrackCountOffsetY`
        *   `TrackCountHeight`
        *   `TrackCountUnderLineY`
        *   `TrackCountUnderLineHeight`
        *   `TrackCountTextAlignment`

*   **要件2: `ArtSize` を基準とした動的レイアウト算出 (`LayoutCalculator`)**
    *   `src/LayoutCalculator.cpp` の `CalculateTrackInfoLayout` におけるトラックナンバーのレイアウト算出を修正する。
    *   Y方向のオフセット（`TrackCountOffsetY`）の加算を廃止し、CD帯の縦の長さ（ボックスの高さやアンダーラインの長さなど、削除されたHeight系パラメータに該当する部分）は、すべて現在のアルバムアートの表示サイズ（`ArtSize`）と完全に一致（連動）するように計算ロジックを改修する。

*   **要件3: アンダーラインのカラー参照変更 (`Widget_TrackInfo`)**
    *   `src/Widget_TrackInfo.cpp` の `CreateResources` および `Draw` において、アンダーライン（直線）を描画するブラシの色を、フォントカラーから `TrackCountBoxBaseColor` と `TrackCountBoxBaseOpacity` を使用するように変更する。

*   **要件4: アライメントのハードコーディング (`Widget_TrackInfo`)**
    *   `src/Widget_TrackInfo.cpp` のテキストレイアウト生成部において、設定値によるアライメント分岐を削除し、常に `DWRITE_TEXT_ALIGNMENT_CENTER`（センタリング）を適用するようにハードコーディングする。

*   **要件5: 回転を考慮したシャドウオフセット補正 (`Widget_TrackInfo`)**
    *   キャンバスが -90度（反時計回り）に回転しているため、画面の「右下（+X, +Y）」に影を落とすには、ローカル座標系でのオフセットを補正する必要がある。
    *   影を描画する際のオフセット適用において、本来の `ShadowOffsetX` と `ShadowOffsetY` をそのまま適用するのではなく、回転行列を前提として「`Xオフセット = ShadowOffsetY`」「`Yオフセット = -ShadowOffsetX`」となるように値を入れ替えて影を描画し、画面上で正確に右下へ影が落ちるように修正する。

#### 【絶対遵守ルール (Constraints)】
*   **描画の破壊防止**: トランスフォームの補正処理等によって、既存の曲名やアーティスト名など他の描画要素に影響を与えないよう、`GetTransform` / `SetTransform` のスコープ管理を徹底すること。
