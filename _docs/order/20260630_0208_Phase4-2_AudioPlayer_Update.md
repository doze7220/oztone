【AIへの指示（Phase 4 - Step 2 : 時間取得のためのAudioPlayer強化）】
現在のプロジェクトをベースに、次の実装を開始します。

**【今回の目的：Phase 4 - Step 2】**
AudioPlayerクラスの再生方式を fire-and-forget から `ma_sound` オブジェクトによる管理へ移行し、再生時間の取得基盤を構築します。

**【実装要件】**
1. **AudioPlayer クラスの拡張**:
   - `ma_sound m_sound;` と `bool m_isSoundLoaded;` をメンバに追加してください。
   - `Play` メソッドを改修し、`ma_engine_play_sound` を廃止してください。代わりに `ma_sound_init_from_file` を用いてサウンドをロードし、`ma_sound_start` で再生を開始するようにしてください。既にロードされている場合は、一度 `ma_sound_uninit` で破棄してからロードし直すこと。
   - 以下の新しいメソッドを追加してください。
     - `float GetPositionSeconds()`: 現在の再生位置（秒）を返す（`ma_sound_get_cursor_in_seconds` 等を使用）。
     - `float GetLengthSeconds()`: 曲の総時間（秒）を返す（`ma_sound_get_length_in_seconds` 等を使用）。
     - `bool IsPlaying()`: 現在再生中かどうかを返す。
   - `Uninitialize` またはデストラクタにて、エンジン解放前に `ma_sound_uninit` を適切に呼び出し、リソースリークを防いでください。

**【事後処理】**
実装完了後、`.\build.bat` を実行して自動ビルドテストを行い、エラーがあれば自己修正してください。成功したら `PROJECT_ARCHITECTURE.md` の更新と、`_docs/logs/YYYYMMDD_HHMM_Phase4_Step2.md` へ実装レポートを出力してください。
