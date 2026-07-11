##### 作業指示書 REQ: Phase 19-4: ヘイローダスト(Halo Dust)の究極ブラッシュアップ (実装実行)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
1. 本プロンプトは「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-4_HaloDustEvolution.md）を新規作成し、本タスクの詳細作業内容とタスクリストを記載すること。
3. チャットにて「Phase 19-4の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
Phase 19-4として、円形パーティクル・ビジュアライザ（`Visualizer_HaloDust.cpp`）をスケールフリー化＆物理シミュレーション化し、究極のブラッシュアップを行う。

*   **1. `ConfigManager` の拡張 (INIによるスケール・グロー制御)**
    *   `src/ConfigManager.h` および `src/ConfigManager.cpp` を修正し、`[Visualizer_HaloDust]` セクションに以下の設定項目を追加し、ゲッター/セッターとINI読み書き処理を実装する。
        *   `HaloLaserLengthRatio` (float, デフォルト: 0.2f): 画面短辺に対するレーザー長の割合
        *   `HaloParticleSizeRatio` (float, デフォルト: 0.05f): 画面短辺に対するパーティクルサイズの割合
        *   `HaloGlowOpacity` (float, デフォルト: 0.6f): ネオングローの不透明度
        *   `HaloGlowThickness` (float, デフォルト: 12.0f): ネオングローの太さ

*   **2. `Visualizer_HaloDust.cpp`: パーティクル構造体の物理エンジン化**
    *   `Visualizer_HaloDust.cpp` 内に定義されている `Particle` および `LaserRay` 構造体を拡張する。
    *   `lifeTime`（現在の生存時間）、`maxLifeTime`（最大寿命）、`acceleration`（加速度）などの物理パラメータを追加する。
    *   構造体の初期化時に、ランダムな寿命と加速度をセットするように変更する。

*   **3. `Visualizer_HaloDust.cpp`: 命の軌跡（VFX）ロジックの組み込み**
    *   描画・更新ループ内で以下の物理シミュレーションを実装する。
        *   **成長と減速**: 長さ（または位置）は最初 0 からスタートし、`acceleration` によって時間差で伸び始める。`lifeTime` が `maxLifeTime` に近づくにつれてマイナス加速度（減速）を適用し、最終的に点に戻るように計算する。
        *   **フェードイン/アウト**: `lifeTime` に基づいて、生成直後は透明から急速に実体化し、消滅時も透明になって消えるようにアルファ値（不透明度）を動的に計算して描画カラーに乗算する。

*   **4. `Visualizer_HaloDust.cpp`: テーマカラーの「青・緑」シフト（RGB桁ずらし）**
    *   曲名とアーティスト名のハッシュ値からテーマカラー（RGB）を生成している箇所を修正する。
    *   R, G, B に割り当てるビットシフトの桁を入れ替える（例えば、Rに割り当てていたビットをBに、BをGになど）ことで、ハッシュのランダム性を保ったまま、黄色・ピンクに寄りやすい偏りを青・緑系（サイバーカラー）へシフトさせるビット演算ロジックへと改修する。

*   **5. `Visualizer_HaloDust.cpp`: スケールフリー化の適用**
    *   描画ロジックにおいて、円やレーザー、パーティクルのサイズを固定値ではなく、画面サイズ（`drawRect` の短辺）と `ConfigManager` から取得した各種Ratio設定（`HaloLaserLengthRatio`, `HaloParticleSizeRatio` 等）を乗算して決定するように修正する。
    *   ネオングローの描画時、`HaloGlowOpacity` と `HaloGlowThickness` を適用し、2パス描画の質感をINIから調整可能にする。

-----------

##### 作業指示書 REQ: Phase 19-4: ヘイローダスト(Halo Dust)の究極ブラッシュアップ (実装実行)


###### 【作業手順（厳守事項）】
1. 本プロンプトは「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-4_HaloDustEvolution.md）に、Hotfixとして本タスクの詳細作業内容とタスクリストを追記すること。
3. チャットにて「Phase 19-4Hotfixの実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
Phase 19-4として、円形パーティクル・ビジュアライザ（`Visualizer_HaloDust.cpp`）をスケールフリー化＆物理シミュレーション化し、パラメータの完全データ駆動（INI化）による究極のブラッシュアップを行う。

*   **1. `ConfigManager` の拡張 (VFXパラメータの完全INI化)**
    *   `src/ConfigManager.h` / `.cpp` を修正し、`[Visualizer_HaloDust]` セクションに以下の設定を追加・パース処理を実装する。
        *   `HaloLaserLengthRatio` (float, デフォルト: 0.2f): 画面短辺に対するレーザー長の割合
        *   `HaloLaserThickness` (float, デフォルト: 2.0f): レーザーのコアの太さ
        *   `HaloLaserSpeed` (float, デフォルト: 1.0f): レーザーの移動速度（ベース値への乗算用）
        *   `HaloLaserSpawnRate` (float, デフォルト: 0.1f): レーザーの発生率・閾値
        *   `HaloLaserLifeTime` (float, デフォルト: 30.0f): レーザーの基本寿命（フレーム数等）
        *   `HaloParticleSizeRatio` (float, デフォルト: 0.05f): 画面短辺に対するパーティクルサイズの割合
        *   `HaloParticleSpeed` (float, デフォルト: 1.0f): パーティクルの移動速度（ベース値への乗算用）
        *   `HaloParticleSpawnRate` (float, デフォルト: 0.2f): パーティクルの発生率・閾値
        *   `HaloParticleLifeTime` (float, デフォルト: 60.0f): パーティクルの基本寿命（フレーム数等）
        *   `HaloGlowOpacity` (float, デフォルト: 0.6f): ネオングローの不透明度
        *   `HaloGlowThickness` (float, デフォルト: 12.0f): ネオングローの太さ

*   **2. `Visualizer_HaloDust.cpp`: パーティクル構造体の物理エンジン化**
    *   `Particle` および `LaserRay` 構造体を拡張し、`lifeTime`（現在の生存時間）、`maxLifeTime`（最大寿命）、`acceleration`（加速度）などのパラメータを追加する。
    *   初期化時に、INIから取得した LifeTime と Speed を基準に、ランダムな揺らぎを加えた寿命と加速度をセットする。発生判定（if文）にも SpawnRate を適用する。

*   **3. `Visualizer_HaloDust.cpp`: 命の軌跡（VFX）ロジックの組み込み**
    *   **成長と減速**: レーザーやパーティクルは発生時（長さ/サイズ 0）からスタートし、`acceleration` と Speed 設定によって時間差で伸び（移動し）始める。`lifeTime` が `maxLifeTime` に近づくにつれてマイナス加速度（減速）を適用し、最終的に点に戻るように計算する。
    *   **フェードイン/アウト**: 生成直後は透明から急速に実体化し、消滅時も透明になって消えるようアルファ値を動的に計算し、カラーに乗算する。

*   **4. `Visualizer_HaloDust.cpp`: テーマカラーの「青・緑」シフト（RGB桁ずらし）**
    *   曲名とアーティスト名のハッシュ値からカラーを生成する箇所を修正し、R, G, B に割り当てるビットシフトの桁を入れ替える（例: R用のビットをBへ）ことで、ランダム性を保ちつつテーマカラーを青・緑系（サイバーカラー）へシフトさせる。

*   **5. `Visualizer_HaloDust.cpp`: スケールフリー化とレーザーへのグロー適用**
    *   レーザー・パーティクルの基準サイズを「**画面の短辺** × INIの各種Ratio」の計算式に置き換える。INIの Thickness 設定等も描画時のパラメータとして渡す。
    *   **レーザーの描画処理を改修し、円本体と同様に「グロー（太く半透明な線）」と「コア（細く不透明な線）」の2パス描画を行うようにし、レーザー全体に発光エフェクト（ネオングロー）を適用する。**

------
##### 作業指示書 REQ: Phase 19-4 Hotfix 2: ヘイローダストのVFX微調整とパラメータINI化 (実装実行)

###### 【作業手順（厳守事項）】
1. 本プロンプトはHotfixの「実装実行」である。事前のレポート作成や計画立案は不要なので、直ちに以下の【実装要件】に従ってコードの修正を実行すること。
2. コード修正が完全に終わった後、既存の作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase19-4_HaloDustEvolution.md 等の最新ファイル）の末尾に「HOTFIX 2: VFXパラメータのINI化とアニメーション微調整」の項目を追加し、対応内容を追記すること。
3. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
4. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

--------------------------------------------------------------------------------

###### 【実装要件】
すでに実装済みのヘイローダスト（`Visualizer_HaloDust.cpp`）に対して、VFXの深度コントロールとアニメーションの微調整、およびパラメータのINI化を行う。

*   **1. `ConfigManager` の拡張 (VFXパラメータのINI化)**
    *   `src/ConfigManager.h` / `.cpp` を修正し、`[Visualizer_HaloDust]` セクションに以下の設定項目を追加し、ゲッター/セッターとINI読み書き処理を実装する。
        *   `HaloLaserBaseOpacity` (float, デフォルト: 0.3f): レーザーの基本透明度
        *   `HaloParticleBaseOpacity` (float, デフォルト: 0.5f): パーティクルの基本透明度

*   **2. `Visualizer_HaloDust.cpp`: 深度コントロールとレーザーのグロー化**
    *   レーザーおよびパーティクルの最終的な描画アルファ値に、INIから取得した `HaloLaserBaseOpacity` または `HaloParticleBaseOpacity` を乗算し、主役のヘイロー（円）を邪魔しない深度（レーザー＜パーティクル＜ヘイロー）を適用する。

*   **3. `Visualizer_HaloDust.cpp`: アニメーションの物理演算微調整**
    *   **パーティクルのサイズ固定化**: 破片（パーティクル）のサイズの拡大縮小（成長・減速）アニメーションを廃止し、サイズを常に固定する。寿命（`lifeTime`）に基づく「フェードイン・フェードアウト（透明度の増減）」のみを適用する。
    *   **レーザーのマイナス反転防止**: レーザーが寿命に近づきマイナス加速度で減速・縮小する際、長さがマイナス（逆方向に突き抜ける）にならないよう、下限を最低 `0.0f` にクランプする安全装置を追加する。
