# RES:実装計画・作業レポート Phase 20-8: WidgetCommon_Survey

## 1. 実装目的
将来的なタスクである `src/Widgets.cpp` の「1 Widget = 1 .cpp」の完全物理分割に向けた準備として、特定のWidgetクラスに依存しない共通処理（ノイズ）を調査・抽出し、あらかじめ新規ファイル `src/WidgetCommon.h / .cpp` へ隔離・パージするための計画を立案する。

## 2. アーキテクチャ設計
### 要件1: 共通処理の洗い出し
`src/Widgets.cpp` を精査し、以下の共通処理（ヘルパー関数やラムダ式、描画の定型処理）を抽出対象としてリストアップした。

1. **`LoadBitmapResourceHelper` (WIC画像読み込みユーティリティ)**
   - **理由**: WICを用いたファイルからのデコードおよび `RCDATA` リソースからのフォールバック読み込み処理は、特定のWidgetに依存しない汎用的な画像ロード処理であるため。現在は `Widgets.cpp` の無名名前空間に置かれている。
2. **`ApplyTrimming` (テキストトリミング適用ユーティリティ)**
   - **理由**: `TrackInfoWidget` および `NextTrackWidget` の `CreateResources` 内で同一のラムダ式として重複定義されている。DWriteフォーマットに対する汎用的な設定（EllipsisとNo-wrapの適用）であるため。
3. **`HexToColorF` (Hex色文字列変換ユーティリティ)**
   - **理由**: 現在は `GlobalHotkeysWidget::CreateResources` 内にローカルラムダ式として定義されているが、Hex文字列(`#RRGGBB`)から `D2D1::ColorF` への変換はUI全般で再利用可能な純粋なユーティリティ関数であるため。
4. **影付きテキスト描画パターンのユーティリティ化 (例: `DrawShadowedTextLayout` 等)**
   - **理由**: `TrackInfoWidget`、`NextTrackWidget`、および `GlobalHotkeysWidget` の `Draw` 関数において、「影用ブラシの不透明度設定 → シャドウ位置へオフセット描画 → テキスト本体描画」という同じボイラープレート（定型文）コードが多数散在している。これを共通の描画ユーティリティ関数として切り出すことで、各Widgetの描画ロジックを大幅に軽量化できるため。

*※補足: `Widgets.cpp` 先頭の無名名前空間にある `enum ActionID` は、実質的に `GlobalHotkeysWidget` のみに密結合している。そのため共通処理には含めず、将来の単一ファイル分割の際に `GlobalHotkeysWidget.cpp` へ移行するものとする。*

### 要件2: `WidgetCommon` への分離計画の立案
新規に `src/WidgetCommon.h` および `src/WidgetCommon.cpp` を作成し、以下のように `WidgetCommon` 名前空間内にユーティリティ関数を集約・定義する。

#### `WidgetCommon.h` 設計
```cpp
#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <string>

namespace WidgetCommon {
    // WICを利用したビットマップリソースの読み込み
    bool LoadBitmapResource(IWICImagingFactory* wicFactory, ID2D1DeviceContext* d2dContext, const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap);

    // テキストフォーマットに対するトリミング（...）と折り返し無効化の適用
    void ApplyTextTrimming(IDWriteFactory* dwriteFactory, IDWriteTextFormat* format);

    // Hexカラーコード文字列 (#RRGGBB) から D2D1::ColorF への変換
    D2D1::ColorF HexToColorF(const std::wstring& hex, float alpha = 1.0f);

    // 影付きテキストレイアウトの描画ユーティリティ
    void DrawShadowedTextLayout(
        ID2D1DeviceContext* context,
        IDWriteTextLayout* textLayout,
        ID2D1SolidColorBrush* textBrush,
        ID2D1SolidColorBrush* shadowBrush,
        D2D1_POINT_2F textOrigin,
        D2D1_POINT_2F shadowOrigin,
        float shadowOpacity);
        
    // 影付きテキスト（直接文字列指定）の描画ユーティリティ
    void DrawShadowedText(
        ID2D1DeviceContext* context,
        const std::wstring& text,
        IDWriteTextFormat* textFormat,
        ID2D1SolidColorBrush* textBrush,
        ID2D1SolidColorBrush* shadowBrush,
        const D2D1_RECT_F& textRect,
        const D2D1_RECT_F& shadowRect,
        float shadowOpacity);
}
```

#### 依存関係と移行時の影響
- `src/Widgets.cpp` 先頭に `#include "WidgetCommon.h"` を追加するだけで依存関係は解決される。
- 各Widgetクラスの既存実装にて、重複しているラムダ式や冗長な描画定型文を `WidgetCommon::` の呼び出しに一括置換する。
- 全ての関数が独立したステートレスなユーティリティであるため、既存の各Widgetからの呼び出しにおいて依存関係が壊れることはなく、描画結果や動作の完全な互換性（機能変更なし）が保証される。

## 3. 実装タスクリスト
[x] タスク1: `WidgetCommon` ファイル群の作成とCMakeへの追加
    - `src/WidgetCommon.h` および `src/WidgetCommon.cpp` を新規作成する。
    - `CMakeLists.txt` に `src/WidgetCommon.cpp` と `src/WidgetCommon.h` を追加する。
    - **完了**: ユーティリティ宣言と定義ファイルを新規作成し、ビルドシステムに組み込んだ。
[x] タスク2: 共通処理の実装と抽出
    - リストアップした汎用関数群 (`LoadBitmapResource`, `ApplyTextTrimming`, `HexToColorF`, `DrawShadowedTextLayout`, `DrawShadowedText`) を `WidgetCommon` 内へ実装する。
    - **完了**: 実装をステートレスなユーティリティとして `WidgetCommon.cpp` に集約した。`shadowOpacity` が `0.0f` の場合は影描画をスキップする等、効率的な描画をサポートするよう設計した。
[x] タスク3: `Widgets.cpp` へのリファクタリング適用
    - `Widgets.cpp` で定義されている `LoadBitmapResourceHelper` や各Widget内のローカルなラムダ式（`ApplyTrimming`, `HexToColorF`）を削除し、`WidgetCommon::` への呼び出しに置き換える。
    - 各Widgetの `Draw` メソッド内にある影付きテキスト描画のボイラープレート部分を `WidgetCommon::DrawShadowedTextLayout` 等へ置き換える。
    - **完了**: 全ての定型ボイラープレートを `WidgetCommon` の呼び出しへ置換完了。`Widgets.cpp` のコード行数を大幅に削減し、可読性を向上させた。描画順序や見た目の変更は一切行っていない。

## 4. Hotfix (2026-07-12 23:22)
- **問題**: `src/WidgetCommon.h` にて `ID2D1DeviceContext` が未定義となるコンパイルエラーが発生。
- **原因**: `ID2D1DeviceContext` は Direct2D 1.1 のインターフェースであり、`<d2d1.h>` ではなく `<d2d1_1.h>` をインクルードする必要があった。
- **対応**: `src/WidgetCommon.h` の `#include <d2d1.h>` を `#include <d2d1_1.h>` に修正し、ビルドエラーを解消した。
