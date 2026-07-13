このファイルを見た限りだと、**いきなりファイル分割はしない方がいい**。

今の `Widget_Playlist.cpp` は、責務が入り混じっているからだ。
この状態で分割すると、

* 同じメンバ変数を複数cppが触る
* ctx/config/layoutを何度も取り回す
* AIが「どのcppを読めば状態が分かるのか」迷う

という状態になり、**コンテキスト節約どころか逆効果**になる可能性がある。

AI-IDE向けなら、

> **責務整理 → 関数化 → ファイル分割**

の3段階がおすすめ。

---

## Phase1 まず責務を分離する

このファイルを見ると、大きく分けてこれだけある。

```
PlaylistWidget
├── Resource
│    CreateResources
│    ReleaseResources
│
├── Animation
│    UpdateAnimation
│
├── Scroll
│    AddScroll
│    GetScrollY
│
├── Draw
│    Draw
│
└── Utility
     ParseHexColor
```

ところがDrawの中身だけでさらに

```
Draw
│
├── グリップ描画
├── 背景描画
├── ツールバー描画
├── ピンボタン描画
├── プレイリスト一覧描画
├── 曲一覧描画
├── スキャンライン描画
└── Hover色計算
```

まで混ざっている。

まずここを整理したい。

---

## Phase2 Drawを100行程度まで小さくする

理想は

```cpp
void PlaylistWidget::Draw(...)
{
    DrawGrip(...);

    DrawBackground(...);

    DrawToolbar(...);

    DrawPlaylistItems(...);

    DrawScanline(...);
}
```

ぐらいになること。

つまりDrawが

**「何を描くか」**

だけ読めるようにする。

---

## Phase3 DrawPlaylistItemsも分割

ここが一番大きい。

今は

```
if (PlaylistListMode)
{
    ...
}
else
{
    ...
}
```

となっている。

これも

```
DrawPlaylistList()

DrawTrackList()
```

に分ける。

するとAIは

「曲一覧だけ直して」

という指示で済む。

---

## Phase4 Toolbarも独立

Toolbarだけでもかなり長い。

```
DrawToolbar()
```

の中も

```
BuildToolbarText()

DrawToolbarButtons()

DrawPinButton()
```

くらいまで分けられる。

---

## Phase5 Utility化

例えば

```
ParseHexColor()
```

これはPlaylist専用ではない。

将来的には

```
ColorUtility.h
```

へ。

Hover色も

```
GetBlendedTextColor()
```

として独立できる。

---

# ここまで整理できたら初めてcpp分割

ここから先はAI-IDE向け。

例えば

```
Widgets/

    Widget_Playlist.cpp
```

を

```
Widgets/

Playlist/

    PlaylistWidget.cpp
    PlaylistWidget_Draw.cpp
    PlaylistWidget_DrawToolbar.cpp
    PlaylistWidget_DrawItems.cpp
    PlaylistWidget_Animation.cpp
    PlaylistWidget_Resources.cpp
```

くらいにする。

---

### AIにはこの粒度がかなり効く

例えば

> プレイリストのスクロールを修正して

なら

```
PlaylistWidget_DrawItems.cpp
PlaylistWidget_Animation.cpp
```

しか読まなくて済む。

Toolbarは読まない。

Resourcesも読まない。

つまりコンテキストが激減する。

---

# さらに一歩進めるなら

実は一番気になったのはここ。

```
DrawTrackList()
```

の中で

```
タイトル取得

↓

アーティスト取得

↓

時間取得

↓

描画

↓

スキャンライン描画
```

まで全部やっている。

これは

```
TrackVisualData
```

のような構造体を作って

```
TrackVisualData data = BuildTrackVisualData(...);

DrawTrackItem(..., data);
```

にするとかなり見通しが良くなる。

AIも

「データ生成」

と

「描画」

を別々に考えられる。

---

## 私なら、この順番で進める

1. **Draw() の巨大化を解消**（最優先）

   * `DrawGrip`
   * `DrawBackground`
   * `DrawToolbar`
   * `DrawPlaylistItems`

2. **DrawPlaylistItems() を二分**

   * `DrawTrackList`
   * `DrawPlaylistList`

3. **Toolbarを整理**

   * `DrawToolbar`
   * `DrawPinButton`
   * `BuildToolbarText`

4. **Utility化**

   * `ParseHexColor`
   * Hover色計算
   * TrackVisualData生成

5. **最後に物理ファイル分割**

   * `PlaylistWidget_Draw.cpp`
   * `PlaylistWidget_DrawToolbar.cpp`
   * `PlaylistWidget_DrawItems.cpp`
   * `PlaylistWidget_Animation.cpp`
   * `PlaylistWidget_Resources.cpp`

この順序なら、各段階で動作確認しやすく、AI-IDEも各タスクで読むべきファイルが明確になります。結果として、ハルシネーションの抑制とコンテキスト消費の削減の両方に効果が期待できます。
