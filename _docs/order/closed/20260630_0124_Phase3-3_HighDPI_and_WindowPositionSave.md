**【今回の目的：Phase 3 - Step 3】**
現代のPC環境（4K/マルチモニタ/表示倍率変更）で美しく描画するための High DPI（Per-Monitor V2）対応と、ConfigManagerを通じた「ウィンドウの表示位置・サイズの保存と復元」を実装します。

**【実装要件】**
1. **High DPI (Per-Monitor V2) の有効化**:
   - `main.cpp` の初期化処理（WIC等の初期化の前後）で、Windows 10APIである `SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)` を呼び出し、プロセス全体をHigh DPI対応にしてください。
2. **ConfigManagerの拡張（ウィンドウ状態の管理）**:
   - `OZtone.ini` の `[Window]` セクションを追加し、`WindowX`, `WindowY`, `WindowWidth`, `WindowHeight` を読み書きできるようにしてください。
   - デフォルト値は現在ハードコードされているサイズ（1024x512）とし、X, Yは画面中央となるような特殊値（例：`CW_USEDEFAULT` に相当する値など）を設定してください。
3. **Windowクラスの拡張（DPIスケーリングと座標復元・保存）**:
   - `Initialize` にて、ConfigManagerから取得した座標とサイズでウィンドウを作成してください。その際、`GetDpiForSystem` または `GetDpiForWindow` の値を用いて、指定されたサイズ（論理ピクセル）にDPIスケールを乗算（物理ピクセル化）して `SetWindowPos` または作成時のサイズに適用してください。
   - `WM_DESTROY` をフックしてアプリ終了時に `GetWindowRect` で現在のウィンドウの物理座標を取得し、DPIスケールで割り戻して論理ピクセルに変換した上で、ConfigManagerの保存メソッドを呼び出して `OZtone.ini` へ書き込んでください。
4. **RendererクラスのDPI追従対応**:
   - Direct2D（WIC描画やDirectWrite含む）がDPIスケーリングに自動追従するよう、必要に応じてD2DのレンダーターゲットのDPI設定を適切に行うか、描画時にスケールマトリックスを適用する処理を確認・追加してください。

**【事後処理】**
実装完了後、`.\build.bat` を実行して自動ビルドテストを行い、エラーがあれば自己修正してください。成功したら `PROJECT_ARCHITECTURE.md` の更新と、`_docs/logs/YYYYMMDD_HHMM_Phase3_Step3.md` へ実装レポートを出力してください。
