【AIへの指示（Phase 8 - Step 2 : 忍者UI・音量コントロールとホバー領域拡張）】
Phase 8のブラッシュアップとして、OZtoneのコンセプトである「ストイックな忍者UI」に基づいた音量コントロール機能を実装します。
さらに、Step 1で実装したホバー判定領域を拡張し、画面下部全体でUIが反応するようにアーキテクチャを調整してください。

以下の要件に従って関連ファイルを修正してください。

**【実装要件】**
1. **ConfigManager の拡張 (INIレイアウトとホバー領域)**:
   - 新たに `[Layout_VolumeControl]` セクションを追加し、以下の描画位置・フォント設定を読み書きできるようにしてください。
     - `BaseLeftOffset` (ウィンドウ左端からのX座標基点)
     - `BaseBottomOffset` (ウィンドウ下端からのY座標基点)
     - `IconSize` (アイコンのサイズ)
     - `FontSize` (フォントサイズ)
     - `FontFamily` (使用するフォントファミリー)
   - `[Audio]` セクションに `DefaultVolume` を追加し、起動時の音量を保存・復元できるようにしてください。
   - `[Layout_Window]` セクションに `ControlHoverHeight` (コントロールUIが反応する画面下部からの高さ、例: 50.0f) を追加してください。

2. **Window クラスの拡張 (ホバー領域の変更とホイール検知)**:
   - Step 1で実装したコントロール領域のホバー判定 (`IsInPlaybackControlRegion` 等) を修正し、「指定したX座標の範囲」ではなく、「画面下端から `ControlHoverHeight` ピクセルの範囲内（画面の横幅全体）」にマウスが入った場合にホバー状態となるように判定を拡張してください。
   - `WindowProc` で `WM_MOUSEWHEEL` メッセージを捕捉し、マウスカーソルが「左下の音量UI周辺（アイコンと数値を含む矩形領域）」にある状態でホイールが回された場合、音量を増減（例: ±0.05 等）させる処理を実装してください。変更された音量は `AudioPlayer::SetVolume` に渡し、同時にINIへ `DefaultVolume` として保存してください。

3. **AudioPlayer クラスの拡張 (音量制御)**:
   - miniaudioのAPIを利用して、音量を設定・取得するメソッドを追加してください。
     - `void SetVolume(float volume)` (0.0f 〜 1.0f または 0 〜 100 を想定)
     - `float GetVolume() const`
   - 初期音量は `ConfigManager` から `DefaultVolume` を読み込んで適用してください。

4. **Renderer クラスの拡張 (音量UIの描画)**:
   - `Render` メソッド内で、コントロール領域のホバーアニメーション用不透明度 (`m_controlAlpha`) を利用して音量UIを描画します。
   - `ConfigManager` で指定された `BaseLeftOffset`, `BaseBottomOffset` を基点とし、外部画像は使わずに `ID2D1PathGeometry` 等で「スピーカーのベクターアイコン」を描画してください。
   - その右隣に現在の音量数値を、指定された `FontSize` と `FontFamily` を用いて DirectWrite で描画してください。
