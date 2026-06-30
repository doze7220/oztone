【AIへの指示（Phase 6 - Step 1 : デフォルトINIテンプレートの導入）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 6 - Step 1】**
ここからプロジェクトは機能追加を終え、コードとUXの「ブラッシュアップフェイズ（Phase 6）」に入ります。
最初のステップとして、ConfigManager 内で個別にハードコードされている各設定項目の初期値を、C++の生文字列リテラル（Raw String Literal）を用いた「デフォルトINIテンプレート」として一箇所に集約し、INIファイル新規作成時の処理をスマートに改修します。
（※各設定の数値は現在のまま変更せず、現状のレイアウトを維持してください）

**【実装要件】**
1. **デフォルトINIテンプレートの定義**:
   - `ConfigManager.cpp` 内（またはヘッダ）に、`constexpr const char* DEFAULT_INI_CONTENT = R"( ... )";` のような形で、現在の設定項目（`[Window]`, `[Layout_AppLogo]`, `[Layout_NowPlaying]`, `[Layout_SeekBar]`, `[Playlist]` など）とその初期値を網羅したINIフォーマットのテキストを定義してください。
   - 各項目の数値や文字列は、現在ソースコード内にハードコーディングされている初期値をそのまま正確に記述してください（コメント等で見やすく整形するのは構いません）。

2. **INIファイル一括生成処理の刷新**:
   - `ConfigManager::Initialize()` 内の「INIファイルが存在しない場合」の処理を修正してください。
   - `WritePrivateProfileStringW` 等で1項目ずつ書き込む従来の冗長な方式を廃止し、`std::ofstream` 等を用いて `DEFAULT_INI_CONTENT` のテキストデータを `OZtone.ini` に一括で書き出して新規作成するようにしてください。
   - ファイルの生成（書き出し）が完了した後は、既存の読み込み処理をそのまま実行して値をロードしてください。

**【追加指示１】**
再生中の曲情報および「次の曲（Next Track）」のテキストについて、ConfigManager を通じて INI ファイルから任意のフォント（FontFamily）を指定できるようにし、UIのカスタマイズ性を向上させます。

**【実装要件１】**
1. **ConfigManager の拡張**:
   - `DEFAULT_INI_CONTENT` の `[Layout_NowPlaying]` セクションに `TitleFontFamily=Meiryo` と `ArtistFontFamily=Meiryo` を追加してください。
   - `[Layout_NextTrack]` セクションに `LabelFontFamily=Meiryo`、`TitleFontFamily=Meiryo`、`ArtistFontFamily=Meiryo` を追加してください。
   - 追加した各項目の文字列を INI から読み込む処理、および外部へ提供するゲッター（`GetTitleFontFamily()` 等、戻り値は `std::wstring` を推奨）を実装してください。

2. **Renderer の拡張**:
   - `Renderer::Initialize` における DirectWrite の `IDWriteFactory::CreateTextFormat` 呼び出し部分を修正してください。
   - メインの曲名・アーティスト名、および Next Track のラベル・曲名・アーティスト名の各フォーマットを生成する際、ハードコードされたフォント名ではなく、ConfigManager から取得した対応するフォントファミリー名を指定するように変更してください。

**【追加指示２】**
左下に表示されている「再生中の曲情報（アルバムアート・曲名・アーティスト名）」の描画基準点を、現在の左上基準（BaseY）から、ウィンドウ下端を基準とした「左下基準（BaseBottomOffset）」へリファクタリングし、ウィンドウサイズ変更時にもレイアウトが崩れないようにします。

**【実装要件２】**
1. **ConfigManager と INIテンプレートの変更**:
   - `ConfigManager.cpp` の `DEFAULT_INI_CONTENT` 内、`[Layout_NowPlaying]` セクションにある `BaseY=350` を廃止し、代わりに `BaseBottomOffset=162` （※ 512 - 350 の値）を追加してください。
   - `ConfigManager` のプロパティ読み込み処理およびゲッターを `GetBaseY()` から `GetBaseBottomOffset()` へ変更してください。

2. **Renderer のレイアウト計算の修正**:
   - `Renderer::Render` メソッド内における「現在再生中の曲情報」の描画基点を計算する際、Y座標を `m_d2dContext->GetSize().height - config.GetBaseBottomOffset()` を基点とするようにロジックを修正してください。
   - アルバムアート画像やテキストの描画は、この新しい「左下基点（X = BaseX, Y = height - BaseBottomOffset）」に対して、既存の各種オフセット（`ArtOffsetX/Y`, `TitleOffsetX/Y`, `ArtistOffsetX/Y`）を加算する形で描画されるように計算式を調整してください。


**【事後処理】**
実装完了後、ビルドを行ってください。既存の `OZtone.ini` を一旦削除してからアプリを起動し、新しい `OZtone.ini` がテンプレート通りに美しく生成され、従来通りのレイアウトで正常に起動できることを確認できたら、`_docs/logs/YYYYMMDD_HHMM_Phase6_Step1.md` へ実装レポートを出力してください。
