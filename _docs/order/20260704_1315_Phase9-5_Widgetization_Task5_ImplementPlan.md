# Goal

Widget化によって不要となった Renderer 内のデッドコードを削除し、
Renderer が Widget のライフサイクル管理・描画順序管理・共通状態管理のみを担当する構造になっていることを最終確認する。

本タスクでは、新しいアーキテクチャ（Compositeパターン、WidgetManager、Facadeの再設計等）の導入や責務の再分配は行わない。
目的は「不要コードの削除」と「最終監査」のみである。

---

## User Review Required

Widget化完了後の最終クリーンアップを実施する。

以下は既にWidgetへ移行済みであり、Renderer側に残っている場合はデッドコードである。

### 削除対象

Renderer.h
- m_titleTextFormat
- m_artistTextFormat
- m_nextLabelTextFormat
- m_nextTitleTextFormat
- m_nextArtistTextFormat
- m_textBrush
- m_shadowBrush
- m_appLogoBitmap
- m_appLogoHoverBitmap
- DrawSeekBar
- DrawPlaybackControls
- DrawVolumeControl

Renderer.cpp
- 上記リソースの生成処理
- 上記ビットマップの読み込み処理
- 上記メソッドの実装

---

### 削除対象外（重要）

以下はRenderer全体で共有する状態であり、Widgetへ複製してはならない。

- m_controlAlpha
- WidgetContext::controlAlpha
- m_controlAlpha の更新処理

PlaybackControls・VolumeControl・SeekBar が常に同じフェード値を共有する設計であるため、
Rendererで一元管理することを許可する。

同じフェード処理を各Widgetへ複製して実装してはならない（DRY原則）。

---

## Proposed Changes

### Renderer のクリーンアップ

Renderer.h

以下を削除する。

- 未使用TextFormat
- 未使用Brush
- 未使用Bitmap
- 未使用Drawメソッド宣言

Renderer.cpp

以下を削除する。

- 未使用リソース生成
- 未使用Bitmapロード
- 未使用Drawメソッド実装

m_controlAlpha 関連は変更しない。

---

### Widget

変更しない。

WidgetContext も変更しない。

新しいWidget階層は追加しない。

Compositeパターンは導入しない。

---

## Verification Plan

### Automated Tests

- build.bat が成功すること
- コンパイルエラーがないこと
- リンクエラーがないこと

---

### Manual Verification

- アプリが正常起動すること
- UIが従来通り表示されること
- コントロールバーのフェードが従来通り動作すること

---

### Architecture Verification

以下を確認すること。

- Renderer が Widget の具象クラスへ直接依存しているのは生成時のみであること。
- 更新処理は IWidget インターフェース経由のみで行われていること。
- 描画処理は IWidget インターフェース経由のみで行われていること。
- Renderer に UI固有の描画ロジックが残っていないこと。
- Renderer に残っている状態は共通状態（共有入力・共有アニメーション・共通リソース）のみであること。

---

### Dead Code Verification

削除対象として列挙したメンバ・メソッドについて、

- 宣言
- 定義
- 呼び出し
- 参照

が一切残っていないことを grep 等で確認すること。

削除漏れがある場合は一覧で報告すること。

---

## Scope

本タスクは最終クリーンアップである。

以下は本タスクでは実施してはならない。

- 新しいアーキテクチャの導入
- Compositeパターンの導入
- WidgetManagerの導入
- Rendererの責務変更
- Widgetの責務変更
- Widget階層の変更
- 新しい設計提案
- リファクタリング範囲の拡大

スコープ外の改善案がある場合は実装せず、TODOとして報告すること。
