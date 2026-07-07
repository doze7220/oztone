とはいえ、まだ1000行もあるんだ。
この中から切り分けたほうがいいのってあるかな。

見た感じだと、思ったより健全だった。

「1000行あるからヤバい」のではなく、**小さいWidgetがたくさん入っているから1000行**という印象だね。

逆に、「これは独立させた方が気持ちいい」と思ったのはある。

### 優先度★★★★★

**VolumeControlWidget**

これだけ他と少し毛色が違う。

* Geometry生成
* TextLayout管理
* Shadow
* アイコン描画
* 音量バーではなくスピーカー描画
* テキスト生成

まで持っている。

一つのWidgetとして完成度が高いので、

```text
VolumeControlWidget.cpp
```

に出してしまっても違和感がない。

---

### 優先度★★★★☆

**PlaybackControlsWidget**

これも同じ。

```cpp
Play
Pause
Prev
Next
Geometry生成
```

まで自己完結している。

今後

* ホバー
* アニメーション
* 押し込み
* Ripple

なんかを追加したら一気に膨らむ。

独立候補。

---

### 優先度★★★☆☆

**LogoMenuWidget**

これは今でも結構長い。

でもこれは

```
LogoMenuWidget
```

として一つの世界があるので、

単独cppにしてもいい。

逆に内部をさらに

```
DrawMenu()
DrawDescription()
DrawIndicator()
```

なんて分ける必要はまだ感じない。

---

### 優先度★★☆☆☆

**TrackInfoWidget**

今くらいならこのままでいい。

レイアウト計算を全部

```
LayoutCalculator
```

へ逃がしているので、
見た目ほど重くない。

---

### 優先度★☆☆☆☆

* SeekBar
* AppLogo
* NextTrack
* ResizeGrip

この辺は小さい。

今は一緒で十分。

---

## 実は一番気になったのは別のところ

Widgetより、

```cpp
LoadBitmapResourceHelper(...)
```

このユーティリティ。

これだけ唯一、

**Widgetじゃない。**

なので

```text
WidgetUtils.cpp
```

とか

```text
BitmapLoader.cpp
```

へ出してしまうと綺麗になる。

---

## 私ならこうする

今なら

```text
Widgets.cpp
    AppLogoWidget
    TrackInfoWidget
    NextTrackWidget
    SeekBarWidget
    ResizeGripWidget

LogoMenuWidget.cpp

PlaybackControlsWidget.cpp

VolumeControlWidget.cpp

PlaylistWidget.cpp

WidgetUtils.cpp
```

くらいが一番バランスがいい気がする。

---

### でも、今すぐやる必要はない

実際にコードを見た感想としては、**1000行という数字ほど読みづらくはなかった**。

理由は、ちゃんと

```cpp
CreateResources()

ReleaseResources()

UpdateAnimation()

UpdateLayout()

Draw()
```

という並びが全Widgetで統一されているから。

スクロールしていても「あ、ここから次のWidgetだな」がすぐ分かる。

だから今の優先順位なら、私は**PlaylistWidgetの設計を先に固める**ことを勧めるかな。

その後で「よし、UI周りは完成」となったタイミングで、`VolumeControlWidget` と `PlaybackControlsWidget` あたりから独立させると、きれいに整理できると思うよ。
