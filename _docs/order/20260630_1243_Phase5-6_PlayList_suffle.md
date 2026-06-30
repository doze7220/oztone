【AIへの指示（Phase 5 - Step 6 : シャッフル・リピート機能の統合）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 5 - Step 6】**
PlaylistManager を改修し、追加された楽曲群に対して「常時シャッフル・リピート再生」を行う機能を実装します。

**【実装要件】**
1. **PlaylistManager の内部構造変更**:
   - 楽曲のファイルパスを保持する実体リスト（`m_playlist` 等）とは別に、再生順序（インデックス）を保持する配列 `std::vector<size_t> m_shuffleIndices` と、現在のシャッフル再生位置を示す `m_shuffleIndex` を追加してください。

2. **シャッフルリストの生成と再生成**:
   - 内部に `GenerateShuffleList()` などのメソッドを作成し、`<random>` の `std::mt19937` と `std::shuffle` を用いて、現在のプレイリストの全インデックスをランダムに並び替えて `m_shuffleIndices` に格納する処理を実装してください。
   - アプリ起動時の `LoadFromFile` 完了時、および D&D 等で新しい曲が追加された際に、このシャッフルリストを生成・更新するようにしてください。

3. **再生進行ロジックの書き換え**:
   - `GetCurrentTrack()` と `GetNextTrack()` は、実体リストの順序ではなく、`m_shuffleIndices` が示すインデックスのファイルパスを返すように修正してください。
   - `Advance()` が呼ばれた際、`m_shuffleIndex` を進めてください。もし `m_shuffleIndices` の末尾まで到達した（1周した）場合は、**再度 `GenerateShuffleList()` を呼び出してシャッフル順を新しく再生成し、インデックスを0に戻してループ**させてください。

**【事後処理】**
実装完了後、ビルドを行ってください。複数のファイルが存在するプレイリストにおいて、元のリスト順ではなくランダムな順序で再生されること、および1周したあとに再び別のランダムな順序で再生が続くことを確認できたら、`PROJECT_ARCHITECTURE.md` の PlaylistManager の仕様記述を更新し、`_docs/logs/YYYYMMDD_HHMM_Phase5_Step6.md` へ実装レポートを出力してください。
