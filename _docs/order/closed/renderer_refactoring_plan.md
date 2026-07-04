ざっと全体を見たけど、このRendererは**「1200行だから分割する」ではなく、「責務が4～5個混ざっているから分割する」段階**だと思う。

逆に言うと、描画順序そのものは結構綺麗なんだ。

```
BeginDraw()

背景
↓
Visualizer
↓
Logo
↓
NowPlaying
↓
SeekBar
↓
NextTrack
↓
PlaybackControl
↓
Playlist
↓
ResizeGrip

EndDraw()
```

この順番はUIツリーになっているので、これを崩さない方がいい。

---

## 一番やってはいけないリファクタリング

よくある

```
Renderer.cpp
↓
Renderer_DrawLogo.cpp
Renderer_DrawSeekBar.cpp
Renderer_DrawPlaylist.cpp
```

みたいな「1200行だからファイルだけ分ける」はおすすめしない。

結局

```
Renderer
```

が全部知っているので何も改善されない。

---

# まず Render() を10個くらいに分ける

最初はこれだけでいい。

```
Renderer::Render()
{
    Begin();

    DrawBackground();

    DrawVisualizer();

    DrawLogo();

    DrawNowPlaying();

    DrawSeekBar();

    DrawNextTrack();

    DrawPlaybackControls();

    DrawPlaylist();

    DrawResizeGrip();

    End();
}
```

これだけでかなり読むのが楽になる。

ここまでは同じcppでもいい。

---

# 次に責務を見ていく

今Rendererを見ると、大きく5種類の仕事をしている。

## ① DirectX管理

```
Initialize()

Resize()

EndDraw()

LoadBitmap()
```

これは

```
GraphicsDevice
```

の仕事。

---

## ② UI描画

```
DrawLogo()

DrawSeekBar()

DrawPlaylist()

DrawNextTrack()
```

これは

```
RendererUI
```

みたいなもの。

---

## ③ レイアウト計算

例えば

```
float x
float y

float width

float rightMargin

float titleRight
```

こういう計算。

これが実は大量にある。

これは描画ではない。

例えば

```
Rect titleRect

Rect artistRect

Rect seekRect
```

を返すだけの

```
LayoutCalculator
```

へ持っていける。

---

## ④ アニメーション

例えば

```
m_controlAlpha

m_playlistSlideX
```

これは描画じゃない。

更新処理。

つまり

```
UpdateAnimations()
```

になる。

今はRender()に混ざっている。

これはかなり分離したい。

---

## ⑤ UI部品

例えば

```
SeekBar
```

は

```
バー

時間

ブラシ

文字

レイアウト
```

全部知っている。

つまりSeekBar自体が一つのWidget。

---

# 俺なら最終的にはこうする

```
Renderer
│
├── Device
│
├── DrawBackground()
├── DrawVisualizer()
├── DrawLogo()
├── DrawNowPlaying()
├── DrawSeekBar()
├── DrawNextTrack()
├── DrawControls()
├── DrawPlaylist()
└── DrawResizeGrip()
```

さらに

```
Renderer
    ↓
NowPlayingRenderer

SeekBarRenderer

PlaylistRenderer

PlaybackRenderer
```

まで行く。

---

# もっと気になった点

実は1200行より気になったものがある。

これ。

```
CreateSolidColorBrush(...)
```

Render中で何十回もやってる。

例えば

```
SeekBar
```

だけでも

```
bgBrush

fgBrush
```

毎フレーム生成。

Playlistでも生成。

Volumeでも生成。

Backgroundでも生成。

これ、Direct2Dでは結構重い。

ブラシは

```
Initialize()
```

で作るもの。

透明度だけ変える。

```
brush->SetOpacity(...)
```

で十分。

ここはかなり改善余地がある。

---

# Geometryも同じ

例えば

```
Play

Pause

Speaker

Triangle
```

毎フレーム

```
CreatePathGeometry()
```

してる。

これも初期化時に作ってしまっていい。

---

# 一番おすすめの進め方

一気にWidget化すると事故るので、

**第1段階**

```
Render()

↓

DrawBackground()

DrawLogo()

・・・

```

だけにする。

---

**第2段階**

レイアウト計算を外へ。

```
CalcSeekBarRect()

CalcTitleRect()

CalcPlaylistRect()
```

---

**第3段階**

ブラシ・Geometryのキャッシュ。

---

**第4段階**

Widget化。

```
PlaylistRenderer

SeekBarRenderer

PlaybackRenderer

NowPlayingRenderer
```

---

## このRendererは設計自体は悪くない

コードを見た印象では、「設計が破綻して1200行になった」のではなく、**機能追加を素直に積み重ねた結果として1200行になった**という状態だ。

だから、大規模に作り直すよりも、

1. `Render()` を描画関数へ分割する。
2. リソース生成（ブラシ・Geometryなど）を初期化へ移す。
3. UI部品ごとにクラス化する。

という順番で進めるのが、安全性と効果のバランスが最も良いと思う。
