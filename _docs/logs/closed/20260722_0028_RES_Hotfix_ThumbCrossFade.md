# RES:HOTFIX作業レポート: サムネイル遅延フェードの完全クロスフェード化

## 1. 実装目的
サムネイル画像の遅延フェードイン時において、背景のガラス板（プレースホルダ）の不透明度を徐々に下げることで、完全なクロスフェード効果を実現する。

## 2. 調査内容
現在、ガラス板は `ConfigManager` から取得した `FallbackArtOpacity` の値で固定の不透明度を持って描画されている。サムネイル画像のフェードイン（`m_thumbFadeAlpha[slotIndex]` による制御）に合わせてこのガラス板の不透明度を減少させることで、画像が完全に読み込まれた際に背面のガラス板を完全に非表示にする。

## 3. 対象ファイル
* `src/Widget_TrackInfo.cpp`

## 4. 実装タスクリスト
[x] タスク1: ガラス板（プレースホルダ）のフェードアウト描画の実装

## 5. 詳細作業内容
* タスク1: ガラス板（プレースホルダ）のフェードアウト描画の実装
    - `Widget_TrackInfo::Draw` 内でガラス板を描画する際、`m_fallbackBlackBrush->SetOpacity()` に渡す値を `config->GetFallbackArtOpacity() * (1.0f - m_thumbFadeAlpha[slotIndex])` に変更した。
    - 既存のサムネイル画像自体のフェードインロジック（`m_thumbFadeAlpha[slotIndex]` の適用）はそのまま維持した。
