かなり良くできている。
AI-IDE向けの指示としても十分明確だ。

ただ、**2点だけ修正した方が安全**だと思う。

---

## ① 要件5（Context/Setter）は少し危険

これだけ少し引っかかった。

> **要件5: コンテキストとセッターの分離 (`Renderer_Context.cpp` / `Renderer_Setter.cpp`)**

以前も言ったけど、

```
Renderer_Setter.cpp
```

はAIのコンテキスト節約という目的から見ると、あまり効果がない。

例えば

```
SetTrackInfo()
SetAlbumArt()
SetBackgroundFraming()
SetFocusedPlaylistIndex()
TriggerVolumeOsd()
TriggerFlyText()
```

これらは全部合わせても100行程度しかない。

これを

```
Renderer_Setter.cpp
```

へ逃がしても、

* ファイル数は増える
* CMakeも増える
* AIが読むファイルも増える

だけになりがち。

---

### 代わりに

```
Renderer_Context.cpp
```

だけを独立させる方が価値が高い。

例えば

```
BuildAnimationContext()

BuildLayoutContext()

BuildRenderContext()
```

をここへ置く。

その結果

```
Renderer_Update.cpp
Renderer_Draw.cpp
```

は

```
WidgetContext ctx = BuildRenderContext(...);
```

だけになる。

これはかなり効く。

---

## ② 「大元Renderer.cppを最小限にする」は少し強すぎる

ここ。

> Renderer.cppにはコンストラクタ・デストラクタ程度のみ残す

これはAIが暴走しやすい。

例えば

```
ClampArtFraming()
```

は

* Drawでもない
* Updateでもない
* Imageでもない

ので、

AIが

```
Renderer_Utility.cpp
```

を勝手に作る可能性がある。

---

なので俺なら

こう書く。

> Renderer.cppには、責務分類が難しい小規模ユーティリティおよびコンストラクタ等のみを残し、巨大な責務単位のみを分割する。

これなら暴走しない。

---

# あと一つ追加したい要件

これが実は一番重要。

AIはファイル分割すると、

```
#include
```

をぐちゃぐちゃにすることが多い。

だから要件に

```
要件7:
各.cppには必要最小限のincludeのみを持たせること。

不要なinclude、
循環依存、
重複includeを作らないこと。

ビルドエラーが出ない構成を維持すること。
```

これを入れておく。

---

# あと、AI-IDE向けならこの一文を入れる

これはかなり効く。

```
今回の目的は
「AI-IDEが必要最小限の.cppのみを
読み込める構成にすること」
であり、

責務分離そのものではなく、
コンテキスト削減を最優先とする。
```

この一文があるだけで、

AIは

```
Renderer_Helper.cpp
Renderer_Common.cpp
Renderer_Misc.cpp
```

みたいな中途半端な分割をしにくくなる。

---

## 全体評価

この作業指示書は**90～95点**くらいの完成度だと思う。

私なら以下の3点だけ修正して実行する。

* **要件5**は `Renderer_Context.cpp` を中心とし、`Renderer_Setter.cpp` は必須にしない。
* **要件6**は「Renderer.cppを空に近づける」ではなく、「責務分類できない小規模ユーティリティは残してよい」とする。
* **要件7**として、**`#include` の最適化と依存関係の維持**を追加する。

この3点を加えると、AI-IDEが「目的のための分割」を行いやすくなり、不必要なファイル増殖や依存関係の乱れも起きにくくなる。
