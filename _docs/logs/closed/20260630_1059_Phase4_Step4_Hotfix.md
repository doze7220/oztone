# Phase 4-4: 楽曲再生時のクラッシュ修正 (Hotfix)

## 事象
楽曲再生開始後、およそ0.5秒程度（約30フレーム）経過したタイミングでアプリケーションが確定でクラッシュする事象が発生した。

## 原因調査
`Application::Run` 内に詳細なトレースログを仕込んで調査した結果、毎フレーム呼び出されている `m_audioPlayer.GetLengthSeconds()` の内部でクラッシュが発生していることが判明した。

### なぜクラッシュしたか？
`miniaudio` では `ma_sound_init_from_file` をデフォルト（ストリーミングモード）で呼び出した場合、MP3などのストリームの全体長を厳密に計算するために `ma_sound_get_length_in_seconds` が内部でファイルスキャンやデコードを伴う処理を実行する。
この処理は非常に重く、また非同期にデコード・再生を行っているバックグラウンドスレッドとの競合（またはデコーダリソースの枯渇）により、**毎フレーム（秒間60回）連続で呼び出すとクラッシュを引き起こす**仕様上の制約・競合状態が存在していた。

## 対応内容
**曲長のキャッシュ対応 (`AudioPlayer` の改修)**
毎フレーム `ma_sound_get_length_in_seconds` を呼び出すのをやめ、曲のロード時 (`AudioPlayer::Play` 内) に1度だけ取得してキャッシュするように修正した。

1. **`AudioPlayer.h`**: 
   `float m_cachedLengthSeconds;` を追加し、コンストラクタで `0.0f` に初期化。
2. **`AudioPlayer.cpp`**: 
   - `Play(const std::string& filepath)` 内の `ma_sound_start` の直前で `ma_sound_get_length_in_seconds` を呼び出し、結果を `m_cachedLengthSeconds` に保存。
   - `GetLengthSeconds()` の処理を「キャッシュされた変数の値を返すだけ」の処理に差し替え。

## 結果
クラッシュは完全に解消され、毎フレーム安定してシークバーと現在時間が描画され続けることを確認した。
また、毎フレームの無駄なファイルスキャンが排除されたため、CPU負荷の削減にも寄与している。
