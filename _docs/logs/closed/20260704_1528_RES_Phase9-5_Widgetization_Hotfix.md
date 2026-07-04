# Phase 9-5: Widgetization 最終デバッグ (調査と引き継ぎ資料)

本資料は、Phase 9-5 にて導入された Widget 化に伴う3つの不具合の原因調査結果と、後続の AI への引き継ぎ（修正手順）をまとめたものである。

## 1. アーキテクチャの絶対制約 (後続AIへの念押し)
* **Rendererの責務**: Renderer はファサードであり、UI固有の状態を持たないこと。
* **Widget間の独立性**: Widget 同士を直接参照させないこと。情報が足りない場合は必ず `WidgetContext` 構造体 (`Widget.h`) にメンバーを追加し、`Application` → `Renderer` → `WidgetContext` の経路でデータを渡すこと。

## 2. 不具合1の原因と修正手順 (トラックNoが消えた)
**原因**: 
`PlaylistWidget::UpdateLayout` 内でトラック数テキスト(`TRACK XXX/XXX`)の `IDWriteTextLayout` を生成する際、最大幅(`maxWidth`)に固定値 `1000.0f` を渡している。テキストフォーマットは右揃え (`DWRITE_TEXT_ALIGNMENT_TRAILING`) に設定されているため、右端からさらに 1000px オフセットされた画面外に描画されてしまっている。また、現在のトラックインデックスの変更を検知する変数がなく、曲が切り替わってもテキストが更新されない状態になっている。

**修正手順**:
* `src/Widgets.h`: `PlaylistWidget` クラスの private メンバに `size_t m_lastCurrentTrackIndex;` を追加する（コンストラクタで初期値をセットする）。
* `src/Widgets.cpp` (`PlaylistWidget::UpdateLayout`): 
  * テキスト再構築の判定条件を `m_lastTotalTracks != ctx.totalTracks || m_lastCurrentTrackIndex != ctx.currentTrackIndex` に変更し、変数も更新する。
  * `m_dwriteFactory->CreateTextLayout` を呼び出す際、`1000.0f` ではなく `200.0f` ( LayoutCalculator で規定されている `trackCountMaxWidth` と同値) を渡すよう修正する。

## 3. 不具合2の原因と修正手順 (位置ズレ)
**原因**: 
`SeekBarWidget` および `VolumeControlWidget` の `CreateResources` において、`IDWriteTextFormat` に対して `SetTextAlignment` および `SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)` が誤って追加されている。
`LayoutCalculator` が算出する Y座標(`textY` や `textOrigin.y`) は既に INI の手動オフセット等を加味した「左上基準」の座標となっているため、ここで段落ごとの縦方向センタリングを有効にすると、指定矩形の高さの分だけテキストが下方向へズレて描画されてしまう。

**修正手順**:
* `src/Widgets.cpp`: `SeekBarWidget::CreateResources` および `VolumeControlWidget::CreateResources` 内にある以下の段落配置設定のコードを完全に削除し、デフォルトの左上基準に戻す。
  ```cpp
  m_timeTextFormat->SetTextAlignment(...);
  m_timeTextFormat->SetParagraphAlignment(...);
  // および VolumeControl 側の同等コード
  ```

## 4. 不具合3の原因と修正手順 (プレイリストがスライドインしない)
**原因**: 
プレイリストのスライドインを担うアニメーションロジック（`m_playlistSlideX += ...`）が、毎フレーム呼ばれる `PlaylistWidget::UpdateAnimation` ではなく、テキスト変更時等にしか呼ばれない `PlaylistWidget::UpdateLayout` の中に記述されてしまっているため、アニメーションが進行しない。
また、`UpdateAnimation` のシグネチャには `ConfigManager*` が渡されないため、現状ではそのままロジックを移植できない状態になっている。

**修正手順**:
* `src/Widget.h`: `WidgetContext` 構造体に `const ConfigManager* config;` を追加する。
* `src/Renderer.cpp`: `Renderer::UpdateAnimation` メソッド内で、コンテキスト生成時に `ctx.config = m_config;` を代入する。
* `src/Widgets.cpp`: `PlaylistWidget::UpdateLayout` 内にある「`m_playlistSlideX` のアニメーション計算」および「`m_playlistManualScrollY` のスクロール限界クランプ処理」のブロックを切り取り、空になっている `PlaylistWidget::UpdateAnimation` の中へ移動する。移動先では `config` の代わりに `ctx.config` を使用する。

## 5. 対応ステータス
- [x] 不具合1: トラックNo表示の修正
- [x] 不具合2: 音量・シークバーテキストの位置ズレ修正
- [x] 不具合3: プレイリストスライドイン機能の修正

## 6. 詳細作業内容
* タスク1 (不具合1): `PlaylistWidget` に `m_lastCurrentTrackIndex` を追加し、曲が切り替わった際に `UpdateLayout` でテキストを再生成するように修正。また、`CreateTextLayout` の幅を `1000.0f` から `200.0f` (trackCountMaxWidth) に変更して画面外への描画はみ出しを修正した。（追加修正: `IDWriteTextLayout1::SetCharacterSpacing` を呼び出す処理を追加し、設定値 `TrackCountLetterSpacing` が正常に描画へ反映されるよう修正した）
* タスク2 (不具合2): `VolumeControlWidget::CreateResources` における `SetTextAlignment` および `SetParagraphAlignment` の呼び出しを削除し、テキスト描画時の縦方向センタリングによる位置ズレを修正した。（※注釈: 前任者の調査では `SeekBarWidget` の該当コードも削除対象とされていたが、シークバー側は専用のY座標オフセット計算処理を持たず、バーの高さ(`layout.textMaxHeight`)に依存して `DWRITE_PARAGRAPH_ALIGNMENT_CENTER` で縦方向の中央揃えを行う実装になっていたため削除不可だった。また、テキストが左寄せになっていた問題を修正するため、`SetTextAlignment` を `DWRITE_TEXT_ALIGNMENT_TRAILING` (右寄せ) に修正した）
* タスク3 (不具合3): `WidgetContext` に `const ConfigManager* config` を追加し、毎フレーム実行される `Renderer::UpdateAnimation` 経由で設定情報を渡せるようにした上で、`PlaylistWidget` のスライドインおよびスクロール限界の計算処理を `UpdateLayout` から `UpdateAnimation` へ移動した。これにより、プレイリストのアニメーションが正常に毎フレーム進行するように修正した。（追記：`Renderer::UpdateAnimation` 実行時に `WidgetContext` の `dpiScale` が未設定(0)になっていたことで座標計算がNaNとなり、スライドイン時にリスト中身が表示されない問題を修正するため、`ctx.dpiScale = m_dpiScale;` の代入処理を追加した）
